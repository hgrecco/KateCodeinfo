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

#ifndef KCIVIEW_H
#define KCIVIEW_H

#include <ktexteditor/document.h>
#include <kate/plugin.h>
#include <kate/mainwindow.h>

#include "ui_kciview.h"

#include <KProcess>

namespace KateCodeinfo
{

class View : public Kate::PluginView, public Ui::CodeinfoWidget
{
  Q_OBJECT

public:
  View(Kate::MainWindow* mainWindow);

  ~View();

  virtual void readSessionConfig(KConfigBase* config, const QString& groupPrefix);
  virtual void writeSessionConfig(KConfigBase* config, const QString& groupPrefix);

  void show(const QString& ci);

public slots:
  void updateCmbActions();

private slots:
  void actionSelected(const QString & text);

  void loadFile();
  void loadClipboard();
  void run();

  void config();
  void onChange();
  void revert();
  void save();

  void processOutput();
  void processExited(int exitCode, QProcess::ExitStatus exitStatus);
  void infoSelected(QTreeWidgetItem* item, int column);


private:
  QWidget* toolView;
  Kate::MainWindow* mw;
  QString m_output;

  KProcess* m_proc;

  void setStatus(const QString& status);
  void execute(const QString& command);
};

};
#endif //KCIVIEW_H

// kate: space-indent on; indent-width 2; replace-tabs on;
