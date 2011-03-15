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

#ifndef KATE_CODEINFO_H
#define KATE_CODEINFO_H

#include <ktexteditor/document.h>
#include <ktexteditor/configpage.h>
#include <kate/plugin.h>
#include <kate/pluginconfigpageinterface.h>
#include <kate/mainwindow.h>

#include "ui_codeinfowidget.h"
#include "ui_codeinfoconfigwidget.h"

#include <QString>
#include <QTimer>

#include <KProcess>

class KateCodeinfoPlugin: public Kate::Plugin, public Kate::PluginConfigPageInterface
{
  Q_OBJECT
  Q_INTERFACES(Kate::PluginConfigPageInterface)
public:
  explicit KateCodeinfoPlugin( QObject* parent = 0, const QList<QVariant>& = QList<QVariant>() );
  virtual ~KateCodeinfoPlugin();

  static KateCodeinfoPlugin& self();

  Kate::PluginView *createView (Kate::MainWindow *mainWindow);

signals:
  void newStatus(const QString&);

  //
  // PluginConfigPageInterface
  //
public:
  virtual uint configPages() const;
  virtual Kate::PluginConfigPage* configPage (uint number = 0, QWidget *parent = 0, const char *name = 0);
  virtual QString configPageName(uint number = 0) const;
  virtual QString configPageFullName(uint number = 0) const;
  virtual KIcon configPageIcon(uint number = 0) const;

  //
  // private data
  //
private:
  static KateCodeinfoPlugin* s_self;
};

class KateCodeinfoPluginView : public Kate::PluginView, public Ui::CodeinfoWidget
{
  Q_OBJECT

public:
  KateCodeinfoPluginView(Kate::MainWindow* mainWindow);

  ~KateCodeinfoPluginView();

  virtual void readSessionConfig (KConfigBase* config, const QString& groupPrefix);
  virtual void writeSessionConfig (KConfigBase* config, const QString& groupPrefix);

  void loadCodeinfo(const QString& ci);

public slots:
  void loadFile();
  void loadClipboard();
  void run();
  void setStatus(const QString& status);

private slots:
  void itemActivated(QTreeWidgetItem* item, int column);
  void processOutput();
  void processExited(int exitCode, QProcess::ExitStatus exitStatus);
  void cmbChanged(const QString & text);

private:
  QWidget* toolView;
  Kate::MainWindow* mw;
  QTimer timer;
  QString m_output;
  QString m_regex;

  KProcess* m_proc;

  void execute(const QString &command);
};

class KateCodeinfoConfigWidget : public Kate::PluginConfigPage, private Ui::CodeinfoConfigWidget
{
  Q_OBJECT
public:
  explicit KateCodeinfoConfigWidget(QWidget* parent = 0, const char* name = 0);
  virtual ~KateCodeinfoConfigWidget();

public slots:
  virtual void apply();
  virtual void reset();
  virtual void defaults();

private slots:
  virtual void hasChanged();
  virtual void itemChanged(QTableWidgetItem *item);
  virtual void add();
  virtual void remove();
  virtual void down();
  virtual void up();
  virtual void currentCellChanged( int currentRow, int currentColumn, int previousRow, int previousColumn );

private:
  bool m_changed;

  void addItem(QString& name, QString& command, QString& regex);

  void swapRows(int from, int to);
};

class KateCodeinfoConfigDialog : public KDialog
{
  Q_OBJECT
public:
  KateCodeinfoConfigDialog(QWidget* parent = 0);
  ~KateCodeinfoConfigDialog();

public slots:
  void changed();

private:
  KateCodeinfoConfigWidget* m_configWidget;
};

#endif //KATE_CODEINFO_H

// kate: space-indent on; indent-width 2; replace-tabs on;
