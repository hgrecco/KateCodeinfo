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
#include "kciplugin.h"
#include "kciplugin.moc"

#include "kciview.h"
#include "kciconfig.h"

#include "kciparser.h"

#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kdebug.h>

//END Includes

namespace KateCodeinfo
{

K_PLUGIN_FACTORY(KatecodeinfoFactory, registerPlugin<Plugin>();)
K_EXPORT_PLUGIN(KatecodeinfoFactory(KAboutData("katecodeinfoplugin", "katecodeinfoplugin", ki18n("Codeinfo"), "0.1", ki18n("Codeinfo"), KAboutData::License_GPL_V2)))

Plugin* Plugin::s_self = 0L;

Plugin::Plugin(QObject* parent, const QList<QVariant>&)
  : Kate::Plugin((Kate::Application*)parent)
  , Kate::PluginConfigPageInterface()
{
  s_self = this;
}

Plugin::~Plugin()
{
  s_self = 0L;
}

Plugin& Plugin::self()
{
  return *s_self;
}

Kate::PluginView *Plugin::createView(Kate::MainWindow *mainWindow)
{
  View* pv = new View(mainWindow);
  connect(this, SIGNAL(actionsUpdated()),
          pv, SLOT(updateCmbActions()));
  return pv;
}

uint Plugin::configPages() const
{
  return 1;
}

Kate::PluginConfigPage* Plugin::configPage(uint number, QWidget *parent, const char *name)
{
  if(number == 0) {
    return new Config(parent, name);
  }

  return 0L;
}

QString Plugin::configPageName(uint number) const
{
  if(number == 0) {
    return i18n("Codeinfo");
  }
  return QString();
}

QString Plugin::configPageFullName(uint number) const
{
  if(number == 0) {
    return i18n("Codeinfo Settings");
  }
  return QString();
}

KIcon Plugin::configPageIcon(uint number) const
{
  Q_UNUSED(number)
  return KIcon("msg_info");
}

void Plugin::refreshActions()
{
  emit actionsUpdated();
}

};

// kate: space-indent on; indent-width 2; replace-tabs on;
