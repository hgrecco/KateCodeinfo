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

#ifndef KCIPLUGIN_H
#define KCIPLUGIN_H

#include <kate/plugin.h>
#include <kate/pluginconfigpageinterface.h>

namespace KateCodeinfo
{
class Plugin: public Kate::Plugin, public Kate::PluginConfigPageInterface
{
  Q_OBJECT
  Q_INTERFACES(Kate::PluginConfigPageInterface)
public:
  explicit Plugin(QObject* parent = 0, const QList<QVariant>& = QList<QVariant>());
  virtual ~Plugin();

  static Plugin& self();

  Kate::PluginView *createView(Kate::MainWindow *mainWindow);

signals:
  void actionsUpdated();

  //
  // PluginConfigPageInterface
  //
public:
  virtual uint configPages() const;
  virtual Kate::PluginConfigPage* configPage(uint number = 0, QWidget *parent = 0, const char *name = 0);
  virtual QString configPageName(uint number = 0) const;
  virtual QString configPageFullName(uint number = 0) const;
  virtual KIcon configPageIcon(uint number = 0) const;

  void refreshActions();
  //
  // private data
  //

private:
  static Plugin* s_self;
};

};

#endif //KCIPLUGIN_H

// kate: space-indent on; indent-width 2; replace-tabs on;
