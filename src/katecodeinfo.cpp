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
  connect(this, SIGNAL(newStatus(const QString&)),
          pv, SLOT(setStatus(const QString&)));
  pv->setStatus(i18n("Ready"));
  return pv;
}

uint KateCodeinfoPlugin::configPages () const
{
  return 1;
}

Kate::PluginConfigPage* KateCodeinfoPlugin::configPage(uint number, QWidget *parent, const char *name)
{
  if (number == 0) {
    return new KateCodeInfoConfigWidget(parent, name);
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





KateCodeinfoPluginView::KateCodeinfoPluginView(Kate::MainWindow *mainWindow)
  : Kate::PluginView(mainWindow)
  , mw(mainWindow)
{
  toolView = mainWindow->createToolView("KatecodeinfoPlugin", Kate::MainWindow::Bottom, SmallIcon("msg_info"), i18n("Codeinfo"));
  QWidget* w = new QWidget(toolView);
  setupUi(w);
  w->show();

  KConfigGroup cg(KGlobal::config(), "codeinfo");
  QString content;
  QMap<QString, QString> entries = cg.entryMap();
  cmbActions->clear();
  foreach(QString key, entries.keys()) {
      cmbActions->addItem(key, entries[key]);
  }

  timer.setSingleShot(true);
  connect(&timer, SIGNAL(timeout()), this, SLOT(clearStatus()));

  connect(btnBacktrace, SIGNAL(clicked()), this, SLOT(loadFile()));
  connect(btnClipboard, SIGNAL(clicked()), this, SLOT(loadClipboard()));
  connect(btnRun, SIGNAL(clicked()), this, SLOT(run()));
  connect(lstCodeinfo, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(itemActivated(QTreeWidgetItem*, int)));
}

KateCodeinfoPluginView::~KateCodeinfoPluginView ()
{
  delete toolView;
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

void KateCodeinfoPluginView::loadFile()
{
  QString url = KFileDialog::getOpenFileName(KUrl(), QString(), mw->window(), i18n("Load file"));
  QFile f(url);
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString str = f.readAll();
    loadCodeinfo(str);
  }
}

void KateCodeinfoPluginView::loadClipboard()
{
  QString ci = QApplication::clipboard()->text();
  loadCodeinfo(ci);
}

void KateCodeinfoPluginView::loadCodeinfo(const QString& ci)
{
  QList<CodeinfoInfo> infos = KateCodeinfoParser::parseCodeinfo(ci);

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

  if (lstCodeinfo->topLevelItemCount()) {
    setStatus(i18n("Loading codeinfo succeeded"));
  } else {
    setStatus(i18n("Loading codeinfo failed"));
  }
}


void KateCodeinfoPluginView::run()
{
  // TODO: Put the run code here
  QString action = cmbActions->itemData(cmbActions->currentIndex()).toString();
  execute(action);
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
    setStatus(i18n("Opened file: %1", path));
  } else {
    setStatus(i18n("File not found: %1", path));
  }
}

void KateCodeinfoPluginView::setStatus(const QString& status)
{
  lblStatus->setText(status);
  timer.start(10*1000);
}

void KateCodeinfoPluginView::clearStatus()
{
  lblStatus->setText(QString());
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

// get all output of identify

void KateCodeinfoPluginView::processOutput()
{
    m_output += m_proc->readAll();
}

// identify was called

void KateCodeinfoPluginView::processExited(int /* exitCode */, QProcess::ExitStatus exitStatus)
{

    if (exitStatus == QProcess::NormalExit) {
        kDebug() << "   result: " << m_output;

        // analyze the result at m_output
        loadCodeinfo(m_output);

        setStatus("Finished running " +  cmbActions->currentText());
    } else {
        setStatus("Error while running " +  cmbActions->currentText());
    }
    btnRun->setDisabled(false);
}




KateCodeInfoConfigWidget::KateCodeInfoConfigWidget(QWidget* parent, const char* name)
  : Kate::PluginConfigPage(parent, name)
{
  setupUi(this);

  reset();

  connect(txtActions->document(), SIGNAL(contentsChanged()), this, SLOT(hasChanged()));
  m_changed = false;
}

void KateCodeInfoConfigWidget::hasChanged()
 {
     m_changed = true;
 }

KateCodeInfoConfigWidget::~KateCodeInfoConfigWidget()
{
}

void KateCodeInfoConfigWidget::apply()
{
  if (m_changed) {
    KConfigGroup cg(KGlobal::config(), "codeinfo");
    QMap<QString, QString> entries = cg.entryMap();
    foreach(QString key, entries.keys()) {
        cg.deleteEntry(key);
    }
    QStringList lines = txtActions->toPlainText().split("\n");
    QStringList keyval;

    // TODO: update combo
    // cmbActions->clear();
    foreach (QString line, lines) {
      keyval = line.split(":");
      if (keyval.count() > 1) {
         cg.writeEntry(keyval[0], keyval[1]);
         // TODO: update combo
         // cmbActions->addItem(keyval[0], entries[key]);
      }
    }

    m_changed = false;
  }
}

void KateCodeInfoConfigWidget::reset()
{
  KConfigGroup cg(KGlobal::config(), "codeinfo");
  QString content;
  QMap<QString, QString> entries = cg.entryMap();
  foreach(QString key, entries.keys()) {
      content += key + ":" + entries[key] + "\n";
  }
  txtActions->setPlainText(content);
}

void KateCodeInfoConfigWidget::defaults()
{
  // TODO

  m_changed = true;
}



KateCodeInfoConfigDialog::KateCodeInfoConfigDialog(QWidget* parent)
  : KDialog(parent)
{
  setCaption(i18n("Codeinfo Settings"));
  setButtons(KDialog::Ok | KDialog::Cancel);

  m_configWidget = new KateCodeInfoConfigWidget(this, "kate_bt_config_widget");
  setMainWidget(m_configWidget);

  connect(this, SIGNAL(applyClicked()), m_configWidget, SLOT(apply()));
  connect(this, SIGNAL(okClicked()), m_configWidget, SLOT(apply()));
  connect(m_configWidget, SIGNAL(changed()), this, SLOT(changed()));
}

KateCodeInfoConfigDialog::~KateCodeInfoConfigDialog()
{
}

void KateCodeInfoConfigDialog::changed()
{
  enableButtonApply(true);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
