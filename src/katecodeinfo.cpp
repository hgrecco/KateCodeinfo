/* This file is part of the KDE project
   Copyright 2011 Hernan E. Grecco <hernan.grecco gmail com>

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
#include "katecodeinfo.h"
#include "katecodeinfo.moc"

#include "codeinfoparser.h"

#include <klocale.h>          // i18n
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <KStandardDirs>
#include <ktexteditor/view.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kfiledialog.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QTimer>
#include <QClipboard>
#include <QMessageBox>

//END Includes

K_PLUGIN_FACTORY(KatecodeinfoFactory, registerPlugin<KateCodeinfoPlugin>();)
K_EXPORT_PLUGIN(KatecodeinfoFactory(KAboutData("katecodeinfoplugin","katecodeinfoplugin",ki18n("Codeinfo"), "0.1", ki18n("Codeinfo"), KAboutData::License_LGPL_V2)) )

KateCodeinfoPlugin* KateCodeinfoPlugin::s_self = 0L;

KateCodeinfoPlugin::KateCodeinfoPlugin( QObject* parent, const QList<QVariant>&)
  : Kate::Plugin ( (Kate::Application*)parent )
  , Kate::PluginConfigPageInterface()
{
  s_self = this;
}

KateCodeinfoPlugin::~KateCodeinfoPlugin()
{
  s_self = 0L;
}

KateCodeinfoPlugin& KateCodeinfoPlugin::self()
{
  return *s_self;
}

Kate::PluginView *KateCodeinfoPlugin::createView (Kate::MainWindow *mainWindow)
{
  KateCodeinfoPluginView* pv = new KateCodeinfoPluginView (mainWindow);
  connect(this, SIGNAL(actionsUpdated()),
          pv, SLOT(refreshActions()));
  return pv;
}

uint KateCodeinfoPlugin::configPages () const
{
  return 1;
}

Kate::PluginConfigPage* KateCodeinfoPlugin::configPage(uint number, QWidget *parent, const char *name)
{
  if (number == 0) {
    return new KateCodeinfoConfigWidget(parent, name);
  }

  return 0L;
}

QString KateCodeinfoPlugin::configPageName (uint number) const
{
  if (number == 0) {
    return i18n("Codeinfo");
  }
  return QString();
}

QString KateCodeinfoPlugin::configPageFullName (uint number) const
{
  if (number == 0) {
    return i18n("Codeinfo Settings");
  }
  return QString();
}

KIcon KateCodeinfoPlugin::configPageIcon (uint number) const
{
  Q_UNUSED(number)
  return KIcon("msg_info");
}

void KateCodeinfoPlugin::refreshActions() {
  emit actionsUpdated();
}

KateCodeinfoPluginView::KateCodeinfoPluginView(Kate::MainWindow *mainWindow)
  : Kate::PluginView(mainWindow)
  , mw(mainWindow)
{
  toolView = mainWindow->createToolView("KatecodeinfoPlugin", Kate::MainWindow::Bottom, SmallIcon("msg_info"), i18n("Codeinfo"));
  QWidget* w = new QWidget(toolView);
  setupUi(w);  
  config();
  w->show();

  btnConfig->setIcon(KIcon("configure"));

  refreshActions();

  timer.setSingleShot(true);
  connect(&timer, SIGNAL(timeout()), this, SLOT(clearStatus()));

  connect(btnFile, SIGNAL(clicked()), this, SLOT(loadFile()));
  connect(btnClipboard, SIGNAL(clicked()), this, SLOT(loadClipboard()));
  connect(btnRun, SIGNAL(clicked()), this, SLOT(run()));
  connect(btnConfig, SIGNAL(clicked()), this, SLOT(config()));
  connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
  connect(btnRevert, SIGNAL(clicked()), this, SLOT(revert()));

  connect(lstCodeinfo, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(itemActivated(QTreeWidgetItem*, int)));
  connect(cmbActions, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(cmbChanged(const QString &)));

  // TODO This is not updating
  cmbActions->setCurrentIndex(0);
}

KateCodeinfoPluginView::~KateCodeinfoPluginView ()
{
  delete toolView;
}

void KateCodeinfoPluginView::refreshActions()
{
  cmbActions->clear();
  cmbActions->addItem("Get all :-)");
  foreach(QString key, KConfigGroup(KGlobal::config(), "codeinfo").keyList()) {
    cmbActions->addItem(key);
  }
}

void KateCodeinfoPluginView::readSessionConfig(KConfigBase* config, const QString& group)
{
  Q_UNUSED(config);
  Q_UNUSED(group);
}

void KateCodeinfoPluginView::writeSessionConfig(KConfigBase* config, const QString& group)
{
  Q_UNUSED(config);
  Q_UNUSED(group);
}

void KateCodeinfoPluginView::cmbChanged(const QString & text)
{
  if (text == "Get all :-)") {
    txtCommand->setText("");
    m_regex = "(P<message>.*)";
  } else {
    QList<QString> list = KConfigGroup(KGlobal::config(), "codeinfo").readEntry(text, QList<QString>());
    txtCommand->setText(list[0]);
    m_regex = list[1];
    kDebug() << text << " " << list[0] << " " << list[1];
  }
}

void KateCodeinfoPluginView::loadFile()
{
  QString url = KFileDialog::getOpenFileName(KUrl(), QString(), mw->window(), i18n("Load file"));
  QFile f(url);
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString str = f.readAll();
    loadCodeinfo(str);
  }}


void KateCodeinfoPluginView::run()
{
  QString filename = "";
  QString directory = "";

  KTextEditor::View *view = mw->activeView();

  if (view) {
    KTextEditor::Document *doc = view->document();
    filename = doc->url().path();
    directory = doc->url().directory();
  }
  QString cmd = txtCommand->text();
  cmd = cmd.replace("%filename", filename).replace("%directory", directory);
  execute(cmd);
}

void KateCodeinfoPluginView::loadClipboard()
{
  QString ci = QApplication::clipboard()->text();
  loadCodeinfo(ci);
}

void KateCodeinfoPluginView::loadCodeinfo(const QString& ci)
{
  QList<CodeinfoInfo> infos = KateCodeinfoParser::parseCodeinfo(ci, m_regex);

  lstCodeinfo->clear();
  foreach (const CodeinfoInfo& info, infos) {
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

  if (!(lstCodeinfo->topLevelItemCount())) {
    setStatus(i18n("Loading codeinfo failed"));
  }
}

void KateCodeinfoPluginView::itemActivated(QTreeWidgetItem* item, int column)
{
  Q_UNUSED(column);

  QString path = item->data(0, Qt::ToolTipRole).toString();
  uint line = item->data(1, Qt::ToolTipRole).toUInt();
  uint col = item->data(2, Qt::ToolTipRole).toUInt();

  if (!path.isEmpty() && QFile::exists(path)) {
    KUrl url(path);
    KTextEditor::View* kv = mw->openUrl(url);
    if (line > 0) {
      kv->setCursorPosition(KTextEditor::Cursor(line - 1, col-1));
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

void KateCodeinfoPluginView::setStatus(const QString& status)
{
  lstCodeinfo->clear();
  QTreeWidgetItem* it = new QTreeWidgetItem(lstCodeinfo);
  it->setData(0, Qt::DisplayRole, "");
  it->setData(1, Qt::DisplayRole, QString::number(-1));
  it->setData(2, Qt::DisplayRole, QString::number(0));
  it->setData(3, Qt::DisplayRole, "");
  it->setData(4, Qt::DisplayRole, status);
  lstCodeinfo->addTopLevelItem(it);
  lstCodeinfo->resizeColumnToContents(4);
}

void KateCodeinfoPluginView::execute(const QString &command)
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


void KateCodeinfoPluginView::processOutput()
{
  m_output += m_proc->readAll();
}


void KateCodeinfoPluginView::processExited(int /* exitCode */, QProcess::ExitStatus exitStatus)
{

  if (exitStatus == QProcess::NormalExit) {
    kDebug() << "   result: " << m_output;

    // analyze the result at m_output
    loadCodeinfo(m_output);

  } else {
    setStatus("Error while running " +  cmbActions->currentText());
  }
  btnRun->setDisabled(false);
}

void KateCodeinfoPluginView::revert()
{
  QList<QString> list = KConfigGroup(KGlobal::config(), "codeinfo").readEntry(cmbActions->currentText(), QList<QString>());
  txtCommand->setText(list[0]);
  txtRegex->setText(list[1]);
}

void KateCodeinfoPluginView::config()
{
  if (btnConfig->isChecked()) {
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

void KateCodeinfoPluginView::save()
{
  KConfigGroup cg(KGlobal::config(), "codeinfo");
  QList<QString> list = QList<QString>();
  list << txtCommand->text() << txtRegex->text();
  cg.writeEntry(cmbActions->currentText(), list);
}

void KateCodeinfoPluginView::onChange()
{
  btnSave->setDisabled(false);
}



KateCodeinfoConfigWidget::KateCodeinfoConfigWidget(QWidget* parent, const char* name)
  : Kate::PluginConfigPage(parent, name)
{
  setupUi(this);
  btnUp->setIcon( KIcon("arrow-up"));
  btnDown->setIcon( KIcon("arrow-down"));
  btnAdd->setIcon( KIcon("list-add"));
  btnRemove->setIcon( KIcon("list-remove"));

  reset();

  connect(btnAdd, SIGNAL(clicked()), this, SLOT(add()));
  connect(btnRemove, SIGNAL(clicked()), this, SLOT(remove()));
  connect(btnDown, SIGNAL(clicked()), this, SLOT(down()));
  connect(btnUp, SIGNAL(clicked()), this, SLOT(up()));
  connect(btnReset, SIGNAL(clicked()), this, SLOT(resetClicked()));
  connect(tblActions, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(currentCellChanged(int, int, int, int)));
  connect(tblActions, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(itemChanged(QTableWidgetItem *)));

  btnRemove->setDisabled((tblActions->rowCount()) == 0);
  currentCellChanged(tblActions->currentRow(), 0, 0, 0);
  m_changed = false;
}

void KateCodeinfoConfigWidget::addItem(QString& name, QString& command, QString& regex) {
  int row = tblActions->currentRow() + 1;
  tblActions->insertRow(row);
  tblActions->setItem(row, 0, new QTableWidgetItem(name));
  tblActions->setItem(row, 1, new QTableWidgetItem(command));
  tblActions->setItem(row, 2, new QTableWidgetItem(regex));
  tblActions->resizeColumnsToContents();
  tblActions->setCurrentCell(row, 0);
  btnRemove->setDisabled(false);
}

void KateCodeinfoConfigWidget::add()
{
  QString empty = "";
  addItem(empty, empty, empty);
}

void KateCodeinfoConfigWidget::remove()
{
  tblActions->removeRow(tblActions->currentRow());
  btnRemove->setDisabled((tblActions->rowCount()) == 0);
}

void KateCodeinfoConfigWidget::swapRows(int from, int to)
{
  bool se = tblActions->isSortingEnabled();
  QTableWidgetItem* tmp;
  for (int i=0; i<3; i++) {
    tmp = tblActions->takeItem(from, i);
    tblActions->setItem(from, i, tblActions->takeItem(to, i));
    tblActions->setItem(to, i, tmp);
  }
  tblActions->selectRow(to);
  tblActions->setSortingEnabled(se);
  m_changed = true;
}

void KateCodeinfoConfigWidget::down()
{
  swapRows(tblActions->currentRow(), tblActions->currentRow()+1);
}

void KateCodeinfoConfigWidget::up()
{
  swapRows(tblActions->currentRow(), tblActions->currentRow()-1);
}

void KateCodeinfoConfigWidget::resetClicked()
{
  QStringList content;
  switch  (	QMessageBox::warning(
             this, "Codeinfo plugin",
             "This will overwrite your current action list with the default one.\n"
             "Continue?",
             "&Yes", "&No", QString::null, 1, 1 )  )
  {
  case 0:
    tblActions->clear();
    content << "pep8" << "pep8 %filename" << "(P<filename>(?:\\w|\\\\|//))+:(P<line>\\d+):(P<col>\\d+)\\w(P<code>\\w)\\w(P<message>)";
    content << "pep8" << "pep8 %filename" << "(P<filename>(?:\\w|\\\\|//))+:(P<line>\\d+):(P<col>\\d+)\\w(P<code>\\w)\\w(P<message>)";
    content << "pep8" << "pep8 %filename" << "(P<filename>(?:\\w|\\\\|//))+:(P<line>\\d+):(P<col>\\d+)\\w(P<code>\\w)\\w(P<message>)";
    content << "pep8" << "pep8 %filename" << "(P<filename>(?:\\w|\\\\|//))+:(P<line>\\d+):(P<col>\\d+)\\w(P<code>\\w)\\w(P<message>)";
    for(int i=0; i<content.count(); i+=3) {
      addItem(content[i], content[i+1], content[i+2]);
    }
    break;
  default:
    break;
  }
}


void KateCodeinfoConfigWidget::currentCellChanged( int currentRow, int currentColumn, int previousRow, int previousColumn )
{
  Q_UNUSED(currentColumn);
  Q_UNUSED(previousRow);
  Q_UNUSED(previousColumn);
  btnUp->setDisabled((currentRow == 0));
  btnDown->setDisabled(currentRow == (tblActions->rowCount() - 1));
}

void KateCodeinfoConfigWidget::hasChanged()
{
  m_changed = true;
}

void KateCodeinfoConfigWidget::itemChanged ( QTableWidgetItem * item )
{
  Q_UNUSED(item);
  m_changed = true;
}

KateCodeinfoConfigWidget::~KateCodeinfoConfigWidget()
{
}

void KateCodeinfoConfigWidget::apply()
{
  //TODO not here
  m_changed = true;
  if (m_changed) {
    KConfigGroup cg(KGlobal::config(), "codeinfo");
    foreach(QString key, cg.keyList()) {
      cg.deleteEntry(key);
    }
    QList<QString> list = QList<QString>();
    for (int i=0; i < (tblActions->rowCount()); i++)
    {            
      list.clear();
      list.append(tblActions->item(i, 1)->text());
      list.append(tblActions->item(i, 2)->text());
      cg.writeEntry(tblActions->item(i, 0)->text(), list);      
    }
    KateCodeinfoPlugin::self().refreshActions();
    m_changed = false;
  }
}

void KateCodeinfoConfigWidget::reset()
{
  QList<QString> list;
  foreach(QString key, KConfigGroup(KGlobal::config(), "codeinfo").keyList()) {
    list = KConfigGroup(KGlobal::config(), "codeinfo").readEntry(key, QList<QString>());
    addItem(key, list[0], list[1]);
  }
}

void KateCodeinfoConfigWidget::defaults()
{
  // TODO

  m_changed = true;
}



KateCodeinfoConfigDialog::KateCodeinfoConfigDialog(QWidget* parent)
  : KDialog(parent)
{
  setCaption(i18n("Codeinfo Settings"));
  setButtons(KDialog::Ok | KDialog::Cancel);

  m_configWidget = new KateCodeinfoConfigWidget(this, "kate_bt_config_widget");
  setMainWidget(m_configWidget);

  connect(this, SIGNAL(applyClicked()), m_configWidget, SLOT(apply()));
  connect(this, SIGNAL(okClicked()), m_configWidget, SLOT(apply()));
  connect(m_configWidget, SIGNAL(changed()), this, SLOT(changed()));
}

KateCodeinfoConfigDialog::~KateCodeinfoConfigDialog()
{
}

void KateCodeinfoConfigDialog::changed()
{
  enableButtonApply(true);
}



// kate: space-indent on; indent-width 2; replace-tabs on;
