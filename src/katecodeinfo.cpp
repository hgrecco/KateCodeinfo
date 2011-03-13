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
#include "btfileindexer.h"

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
K_EXPORT_PLUGIN(KatecodeinfoFactory(KAboutData("katecodeinfoplugin","katecodeinfoplugin",ki18n("Backtrace Browser"), "0.1", ki18n("Browsing backtraces"), KAboutData::License_LGPL_V2)) )


KateCodeinfoPlugin* KateCodeinfoPlugin::s_self = 0L;
static QStringList fileExtensions =
    QStringList() << "*.cpp" << "*.cxx" << "*.c" << "*.cc"
                  << "*.h" << "*.hpp" << "*.hxx"
                  << "*.moc";


KateCodeinfoPlugin::KateCodeinfoPlugin( QObject* parent, const QList<QVariant>&)
  : Kate::Plugin ( (Kate::Application*)parent )
  , Kate::PluginConfigPageInterface()
  , indexer(&db)
{
  s_self = this;
  db.loadFromFile(KStandardDirs::locateLocal( "data", "kate/backtracedatabase"));
}

KateCodeinfoPlugin::~KateCodeinfoPlugin()
{
  if (indexer.isRunning()) {
    indexer.cancel();
    indexer.wait();
  }

  db.saveToFile(KStandardDirs::locateLocal( "data", "kate/backtracedatabase"));

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
  pv->setStatus(i18n("Indexed files: %1", db.size()));
  return pv;
}

KateCodeinfoDatabase& KateCodeinfoPlugin::database()
{
  return db;
}

BtFileIndexer& KateCodeinfoPlugin::fileIndexer()
{
  return indexer;
}

void KateCodeinfoPlugin::startIndexer()
{
  if (indexer.isRunning()) {
    indexer.cancel();
    indexer.wait();
  }
  KConfigGroup cg(KGlobal::config(), "codeinfo");
  indexer.setSearchPaths(cg.readEntry("search-folders", QStringList()));
  indexer.setFilter(cg.readEntry("file-extensions", fileExtensions));
  indexer.start();
  emit newStatus(i18n("Indexing files..."));
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
    return i18n("Backtrace Browser");
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
  return KIcon("kbugbuster");
}





KateCodeinfoPluginView::KateCodeinfoPluginView(Kate::MainWindow *mainWindow)
  : Kate::PluginView(mainWindow)
  , mw(mainWindow)
{
  toolView = mainWindow->createToolView("KatecodeinfoPlugin", Kate::MainWindow::Bottom, SmallIcon("kbugbuster"), i18n("Backtrace Browser"));
  QWidget* w = new QWidget(toolView);
  setupUi(w);
  w->show();

  timer.setSingleShot(true);
  connect(&timer, SIGNAL(timeout()), this, SLOT(clearStatus()));

  connect(btnBacktrace, SIGNAL(clicked()), this, SLOT(loadFile()));
  connect(btnClipboard, SIGNAL(clicked()), this, SLOT(loadClipboard()));
  connect(btnConfigure, SIGNAL(clicked()), this, SLOT(configure()));
  connect(lstBacktrace, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(itemActivated(QTreeWidgetItem*, int)));
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
  QString url = KFileDialog::getOpenFileName(KUrl(), QString(), mw->window(), i18n("Load Codeinfo"));
  QFile f(url);
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString str = f.readAll();
    loadBacktrace(str);
  }
}

void KateCodeinfoPluginView::loadClipboard()
{
  QString ci = QApplication::clipboard()->text();
  loadBacktrace(ci);
}

void KateCodeinfoPluginView::loadBacktrace(const QString& ci)
{
  QList<CodeinfoInfo> infos = KateCodeinfoParser::parseCodeinfo(ci);

  lstBacktrace->clear();
  foreach (const CodeinfoInfo& info, infos) {
    QTreeWidgetItem* it = new QTreeWidgetItem(lstBacktrace);

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

    lstBacktrace->addTopLevelItem(it);
  }
  lstBacktrace->resizeColumnToContents(0);
  lstBacktrace->resizeColumnToContents(1);
  lstBacktrace->resizeColumnToContents(2);
  lstBacktrace->resizeColumnToContents(3);

  if (lstBacktrace->topLevelItemCount()) {
    setStatus(i18n("Loading codeinfo succeeded"));
  } else {
    setStatus(i18n("Loading codeinfo failed"));
  }
}


void KateCodeinfoPluginView::configure()
{
  KateCodeInfoConfigDialog dlg(mw->window());
  dlg.exec();
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
    kv->setCursorPosition(KTextEditor::Cursor(line - 1, col));
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




KateCodeInfoConfigWidget::KateCodeInfoConfigWidget(QWidget* parent, const char* name)
  : Kate::PluginConfigPage(parent, name)
{
  setupUi(this);
  edtUrl->setMode(KFile::Directory);
  edtUrl->setUrl(KUrl(QDir().absolutePath()));

  reset();

  connect(btnAdd, SIGNAL(clicked()), this, SLOT(add()));
  connect(btnRemove, SIGNAL(clicked()), this, SLOT(remove()));
  connect(edtExtensions, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged()));

  m_changed = false;
}

KateCodeInfoConfigWidget::~KateCodeInfoConfigWidget()
{
}

void KateCodeInfoConfigWidget::apply()
{
  if (m_changed) {
    QStringList sl;
    for (int i = 0; i < lstFolders->count(); ++i) {
      sl << lstFolders->item(i)->data(Qt::DisplayRole).toString();
    }
    KConfigGroup cg(KGlobal::config(), "codeinfo");
    cg.writeEntry("search-folders", sl);

    QString filter = edtExtensions->text();
    filter.replace(',', ' ').replace(';', ' ');
    cg.writeEntry("file-extensions", filter.split(' ', QString::SkipEmptyParts));

    KateCodeinfoPlugin::self().startIndexer();
    m_changed = false;
  }
}

void KateCodeInfoConfigWidget::reset()
{
  KConfigGroup cg(KGlobal::config(), "codeinfo");
  lstFolders->clear();
  lstFolders->addItems(cg.readEntry("search-folders", QStringList()));
  edtExtensions->setText(cg.readEntry("file-extensions", fileExtensions).join(" "));
}

void KateCodeInfoConfigWidget::defaults()
{
  lstFolders->clear();
  edtExtensions->setText(fileExtensions.join(" "));

  m_changed = true;
}

void KateCodeInfoConfigWidget::add()
{
  QDir url(edtUrl->lineEdit()->text());
  if (url.exists())
  if (lstFolders->findItems(url.absolutePath(), Qt::MatchExactly).size() == 0) {
    lstFolders->addItem(url.absolutePath());
    emit changed();
    m_changed = true;
  }
}

void KateCodeInfoConfigWidget::remove()
{
  QListWidgetItem* item = lstFolders->currentItem();
  if (item) {
    delete item;
    emit changed();
    m_changed = true;
  }
}

void KateCodeInfoConfigWidget::textChanged()
{
  emit changed();
  m_changed = true;
}





KateCodeInfoConfigDialog::KateCodeInfoConfigDialog(QWidget* parent)
  : KDialog(parent)
{
  setCaption(i18n("Backtrace Browser Settings"));
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
