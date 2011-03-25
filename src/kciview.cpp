/* This file is part of the KDE project
   Copyright 2011 Hernan E. Grecco <hernan.grecco@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

//BEGIN Includes
#include "kciview.h"
#include "kciview.moc"

#include "ui_kciview.h"

#include "kciparser.h"

#include <ktexteditor/view.h>
#include <kfiledialog.h>

#include <QClipboard>
#include <QMessageBox>

//END Includes

namespace KateCodeinfo
{

View::View(Kate::MainWindow *mainWindow)
  : Kate::PluginView(mainWindow)
  , mw(mainWindow)
{
  toolView = mainWindow->createToolView("KatecodeinfoPlugin", Kate::MainWindow::Bottom, SmallIcon("msg_info"), i18n("Codeinfo"));
  QWidget* w = new QWidget(toolView);
  setupUi(w);
  config();
  w->show();

  btnConfig->setIcon(KIcon("configure"));

  m_nregex = NamedRegExp();
  updateCmbActions();
  updateGlobal();

  connect(btnFile, SIGNAL(clicked()), this, SLOT(loadFile()));
  connect(btnClipboard, SIGNAL(clicked()), this, SLOT(loadClipboard()));
  connect(btnRun, SIGNAL(clicked()), this, SLOT(run()));
  connect(btnConfig, SIGNAL(clicked()), this, SLOT(config()));
  connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
  connect(btnRevert, SIGNAL(clicked()), this, SLOT(revert()));

  connect(lstCodeinfo, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(infoSelected(QTreeWidgetItem*, int)));
  connect(cmbActions, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(actionSelected(const QString &)));
  connect(txtRegex, SIGNAL(textChanged(QString)), this, SLOT(regexChanged(QString)));
  actionSelected(cmbActions->currentText());
}

View::~View()
{
  delete toolView;
}

void View::updateCmbActions()
{
  cmbActions->blockSignals(true);
  cmbActions->clear();
  foreach(QString key, Store::actionNames()) {
    if (Store::readAction(key).enabled) {
      cmbActions->addItem(key);
    }
  }
  cmbActions->setCurrentIndex(0);
  actionSelected(cmbActions->currentText());
  cmbActions->blockSignals(false);
}

void View::updateGlobal()
{
  m_global = Store::readGlobal();
}

void View::readSessionConfig(KConfigBase* config, const QString& group)
{
  Q_UNUSED(config);
  Q_UNUSED(group);
}

void View::writeSessionConfig(KConfigBase* config, const QString& group)
{
  Q_UNUSED(config);
  Q_UNUSED(group);
}

void View::actionSelected(const QString & text)
{
  m_currentAction = text;
  Store::Action ac = Store::readAction(text);
  txtCommand->setText(ac.command);
  txtRegex->setText(ac.regex);
}

void View::loadFile()
{
  QString url = KFileDialog::getOpenFileName(KUrl(), QString(), mw->window(), i18n("Load file"));
  QFile f(url);
  if(f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString str = f.readAll();
    show(str);
  }
}


void View::run()
{
  KTextEditor::View *view = mw->activeView();

  if (!view) {
    return;
  }
  if (m_global.saveBeforeRun) {
    view->document()->save();
  }

  KTextEditor::Document *doc = view->document();
  QString filename = doc->url().path();
  QString directory = doc->url().directory();

  kDebug() << filename << "-" << directory;
  QString cmd = txtCommand->text();
  cmd = cmd.replace("%filename", filename).replace("%directory", directory);
  execute(cmd);
}

void View::loadClipboard()
{
  QString ci = QApplication::clipboard()->text();
  show(ci);
}

void View::show(const QString& ci)
{
  QList<Info> infos = parse(ci, txtRegex->text());

  lstCodeinfo->clear();

  bool showAnyway = ((m_global.showNonParsed == 1) && btnConfig->isChecked()) or (m_global.showNonParsed == 2);

  foreach(const Info & info, infos) {
    if (!info.parsed && !showAnyway) {
        continue;
    }
    QTreeWidgetItem* it = new QTreeWidgetItem(lstCodeinfo);

    // File
    QFileInfo fi(info.filename);
    it->setData(0, Qt::DisplayRole, fi.fileName());
    it->setData(0, Qt::ToolTipRole, info.filename);

    // Line
    it->setData(1, Qt::DisplayRole, QString::number(info.line));
    it->setData(1, Qt::ToolTipRole, info.line);

    // Col
    it->setData(2, Qt::DisplayRole, QString::number(info.col));
    it->setData(2, Qt::ToolTipRole, info.col);

    // Code
    it->setData(3, Qt::DisplayRole, info.code);
    it->setData(3, Qt::ToolTipRole, info.code);

    // Message
    it->setData(4, Qt::DisplayRole, info.message);
    it->setData(4, Qt::ToolTipRole, info.message);

    lstCodeinfo->addTopLevelItem(it);
  }
  lstCodeinfo->resizeColumnToContents(0);
  lstCodeinfo->resizeColumnToContents(1);
  lstCodeinfo->resizeColumnToContents(2);
  lstCodeinfo->resizeColumnToContents(3);

  if(!(lstCodeinfo->topLevelItemCount())) {
    setStatus(i18n("Loading codeinfo failed"));
  }
}

void View::infoSelected(QTreeWidgetItem* item, int column)
{
  Q_UNUSED(column);

  QString path = item->data(0, Qt::ToolTipRole).toString();
  uint line = item->data(1, Qt::ToolTipRole).toUInt();
  uint col = item->data(2, Qt::ToolTipRole).toUInt();

  if(!path.isEmpty() && QFile::exists(path)) {
    KUrl url(path);
    KTextEditor::View* kv = mw->openUrl(url);
    if(line > 0) {
      kv->setCursorPosition(KTextEditor::Cursor(line - 1, col - 1));
    } else {
      // Line = 0 is used to show information about a file.
      // Just go to the beginning.
      kv->setCursorPosition(KTextEditor::Cursor(0, 0));
    }
    kv->setFocus();
    kDebug() << "Opened file: " << path;
  } else {
    setStatus(i18n("File not found: %1", path));
  }
}

void View::setStatus(const QString& status)
{
  lstCodeinfo->clear();
  QTreeWidgetItem* it = new QTreeWidgetItem(lstCodeinfo);
  it->setData(0, Qt::DisplayRole, "");
  it->setData(1, Qt::DisplayRole, "");
  it->setData(2, Qt::DisplayRole, "");
  it->setData(3, Qt::DisplayRole, "");
  it->setData(4, Qt::DisplayRole, status);
  lstCodeinfo->addTopLevelItem(it);
  lstCodeinfo->resizeColumnToContents(4);
}

void View::execute(const QString &command)
{
  btnRun->setDisabled(true);

  m_proc = new KProcess(this);
  m_proc->setShellCommand(command);
  m_proc->setOutputChannelMode(KProcess::MergedChannels);
  m_proc->setReadChannel(QProcess::StandardOutput);

  connect(m_proc, SIGNAL(readyReadStandardOutput()),
          this, SLOT(processOutput()));
  connect(m_proc, SIGNAL(readyReadStandardError()),
          this, SLOT(processOutput()));
  connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
          this, SLOT(processExited(int, QProcess::ExitStatus)));

  m_output = "";
  kDebug() << "   execute '" << command << "'";

  m_proc->start();
  setStatus("Running " +  cmbActions->currentText());
}


void View::processOutput()
{
  m_output += m_proc->readAll();
}


void View::processExited(int /* exitCode */, QProcess::ExitStatus exitStatus)
{

  if(exitStatus == QProcess::NormalExit) {
    kDebug() << "   result: " << m_output;

    // analyze the result at m_output
    show(m_output);

  } else {
    setStatus("Error while running " +  cmbActions->currentText());
  }
  btnRun->setDisabled(false);
}

void View::revert()
{  
  Store::Action ac = Store::readAction(cmbActions->currentText());
  txtCommand->setText(ac.command);
  txtRegex->setText(ac.regex);
}

void View::config()
{
  if(btnConfig->isChecked()) {
    widgetConfig->show();
    cmbActions->setEditable(true);
    txtCommand->setReadOnly(false);
    txtRegex->setReadOnly(false);
  } else {
    widgetConfig->hide();
    cmbActions->setEditable(false);
    txtCommand->setReadOnly(true);
    txtRegex->setReadOnly(true);
  }
}

void View::save()
{

  if (m_currentAction != cmbActions->currentText()) {
    switch(QMessageBox::warning(
             this->toolView, tr("Codeinfo plugin"),
             tr("You have changed the name of the action.") +
             tr("Do you want to create a new action with a different name or rename the old one?"),
             tr("&Rename"), tr("&New"), QString::null, 1, 1)) {
    case 0: //Rename
        Store::deleteAction(m_currentAction);
        Store::writeAction(cmbActions->currentText(), txtCommand->text(), txtRegex->text());
        m_currentAction = cmbActions->currentText();
        updateCmbActions();
        cmbActions->setCurrentIndex(cmbActions->findText(m_currentAction));
      break;
    default:
        Store::writeAction(cmbActions->currentText(), txtCommand->text(), txtRegex->text());
        m_currentAction = cmbActions->currentText();
      break;
    }
  }
}

void View::commandChanged(QString newText)
{
onChange();
}

void View::regexChanged(QString newText)
{
  m_nregex.setNamedPattern(newText);
  QPalette palette = txtRegex->palette();
  QSet<QString> valid;
  valid << "filename" << "line" << "col" << "code" << "message";
  QSet<QString> diff = m_nregex.namedGroups() - valid;
  if (!diff.isEmpty()) {
    txtRegex->setToolTip(*diff.begin() + tr(" is not a valid name for a capturing group"));
    palette.setColor(QPalette::Text, Qt::red);
  } else if (!m_nregex.isValid()) {
    palette.setColor(QPalette::Text, Qt::red);
    txtRegex->setToolTip(tr("The regular expression is invalid"));
  } else {
    palette.setColor(QPalette::Text, Qt::black);
    txtRegex->setToolTip(tr(""));
  }
  txtRegex->setPalette(palette);
  onChange();
}

void View::onChange()
{
  btnSave->setDisabled(false);
}

};

// kate: space-indent on; indent-width 2; replace-tabs on;
