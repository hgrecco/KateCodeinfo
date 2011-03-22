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

#ifndef KCICONFIG_H
#define KCICONFIG_H

#include <kate/pluginconfigpageinterface.h>
#include "ui_kciconfig.h"

#include <QTableWidgetItem>

namespace KateCodeinfo
{

namespace Store
{

struct Action
{
  bool enabled;
  QString name;
  QString command;
  QString regex;
};

  Action readAction(const QString& name);
  void writeAction(const QString& name, const QString& command, const QString& regex, bool enabled=true);
  QList<QString> actionNames();
  void deleteActions();
  void deleteAction(const QString& name, bool removeFromList=true);
};


class Config : public Kate::PluginConfigPage, private Ui::CodeinfoConfigWidget
{
  Q_OBJECT
public:
  explicit Config(QWidget* parent = 0, const char* name = 0);
  virtual ~Config();

public slots:
  virtual void apply();
  virtual void reset();
  virtual void defaults();

private slots:
  void hasChanged();
  void itemChanged(QTableWidgetItem *item);
  void add();
  void remove();
  void down();
  void up();
  void loadDefault();
  void currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
  bool m_changed;
  void addItem(QString& name, QString& command, QString& regex, bool enabled=true);
  void addItem(Store::Action action);
  void swapRows(int from, int to);
};

};

#endif //KCICONFIG_H

// kate: space-indent on; indent-width 2; replace-tabs on;
