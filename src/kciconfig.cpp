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
#include "kciplugin.h"

#include "kciconfig.h"
#include "kciconfig.moc"

#include <QMessageBox>
#include <QCheckBox>

#include <kdebug.h>
//END Includes

namespace KateCodeinfo
{

Config::Config(QWidget* parent, const char* name):Kate::PluginConfigPage(parent, name)
{
  setupUi(this);
  btnUp->setIcon(KIcon("arrow-up"));
  btnDown->setIcon(KIcon("arrow-down"));
  btnAdd->setIcon(KIcon("list-add"));
  btnRemove->setIcon(KIcon("list-remove"));

  reset();

  connect(btnAdd, SIGNAL(clicked()), this, SLOT(add()));
  connect(btnRemove, SIGNAL(clicked()), this, SLOT(remove()));
  connect(btnDown, SIGNAL(clicked()), this, SLOT(down()));
  connect(btnUp, SIGNAL(clicked()), this, SLOT(up()));
  connect(btnLoadDefault, SIGNAL(clicked()), this, SLOT(loadDefault()));
  connect(tblActions, SIGNAL(currentCellChanged(int, int, int, int)),
          this, SLOT(currentCellChanged(int, int, int, int)));
  connect(tblActions, SIGNAL(itemChanged(QTableWidgetItem *)),
          this, SLOT(itemChanged(QTableWidgetItem *)));
  connect(this, SIGNAL(changed()), this, SLOT(hasChanged()));
  connect(chkSaveBeforeRun, SIGNAL(stateChanged(int)),
          this, SLOT(emitChanged()));
  connect(cmbNonParsed, SIGNAL(currentIndexChanged(int)),
          this, SLOT(emitChanged()));

  btnRemove->setDisabled((tblActions->rowCount()) == 0);
  currentCellChanged(tblActions->currentRow(), 0, 0, 0);
  m_changed = false;
}

void Config::addItem(QString& name, QString& command, QString& regex, bool enabled)
{
  Store::Action ac;
  ac.name = name;
  ac.command = command;
  ac.regex = regex;
  ac.enabled = enabled;
  addItem(ac);
}

void Config::addItem(Store::Action action)
{
  int row = tblActions->currentRow() + 1;
  tblActions->insertRow(row);
  QCheckBox *chk= new QCheckBox();
  chk->setChecked(action.enabled);
  connect(chk, SIGNAL(stateChanged(int)), this, SLOT(hasChanged()));
  tblActions->setCellWidget(row, 0, chk);
  tblActions->setItem(row, 1, new QTableWidgetItem(action.name));
  tblActions->setItem(row, 2, new QTableWidgetItem(action.command));
  tblActions->setItem(row, 3, new QTableWidgetItem(action.regex));
  tblActions->resizeColumnsToContents();
  tblActions->setCurrentCell(row, 0);
  btnRemove->setDisabled(false);
  emit changed();
}

void Config::add()
{
  QString empty = "Test";
  addItem(empty, empty, empty);
  emit changed();
}

void Config::remove()
{
  tblActions->removeRow(tblActions->currentRow());
  btnRemove->setDisabled((tblActions->rowCount()) == 0);
  emit changed();
}

void Config::swapRows(int from, int to)
{
  bool se = tblActions->isSortingEnabled();
  QTableWidgetItem* tmp;
  for(int i = 0; i < 3; i++) {
    tmp = tblActions->takeItem(from, i);
    tblActions->setItem(from, i, tblActions->takeItem(to, i));
    tblActions->setItem(to, i, tmp);
  }
  tblActions->selectRow(to);
  tblActions->setSortingEnabled(se);
  emit changed();
}

void Config::down()
{
  swapRows(tblActions->currentRow(), tblActions->currentRow() + 1);
}

void Config::up()
{
  swapRows(tblActions->currentRow(), tblActions->currentRow() - 1);
}

void Config::loadDefault()
{
  QStringList content;
  switch(QMessageBox::warning(
           this, "Codeinfo plugin",
           i18n("This will overwrite your current action list with the default one.\n"
           "Do you want to continue?"),
           i18n("&Yes"), i18n("&No"), QString::null, 1, 1)) {
  case 0:
    for (int i=tblActions->rowCount(); --i >= 0; ) {
      tblActions->removeRow(i);
    }
    // (P<filename>.*):(P<line>\d+):(P<col>\d+):\s*(P<code>\w+)\s*(P<message>.*)
    content << "get all" << "cat %filename" << "(P<message>.*)";
    content << "pep8" << "pep8 %filename" << "(P<filename>.*):(P<line>\\d+):(P<col>\\d+):\\s*(P<code>\\w+)\\s*(P<message>.*)";
    content << "pylint" << "pylint -f parseable -r n %filename" << "(P<filename>.*):(P<line>\\d+):\\s*\\[(P<code>\\w+)(?:,(?:.*))*\\]\\s+(P<message>.*)";
    content << "gcc syntax" << "gcc -fsyntax-only -Wall %filename" << "(P<filename>.*):(P<line>\\d+):(P<col>\\d+):\\s*(P<code>\\w+):\\s*(P<message>.*)";
    content << "clang syntax" << "clang -fsyntax-only -Wall -fno-caret-diagnostics %filename" << "(P<filename>.*):(P<line>\\d+):(P<col>\\d+):\\s*(P<code>\\w+):\\s*(P<message>.*)";
    content << "gjslint" << "gjslint %filename" << "Line\\s(P<line>\\d+),\\s(P<code>\\w:\\w+):\\s(P<message>.*)";
    for(int i = 0; i < content.length(); i += 3) {
      addItem(content[i], content[i+1], content[i+2]);
    }
    emit changed();
    break;
  default:
    break;
  }
}

void Config::currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn);
  Q_UNUSED(previousRow);
  Q_UNUSED(previousColumn);
  btnUp->setDisabled((currentRow == 0));
  btnDown->setDisabled(currentRow == (tblActions->rowCount() - 1));
  emit changed();
}

void Config::emitChanged()
{
  emit changed();
}

void Config::hasChanged()
{
  m_changed = true;
}

void Config::itemChanged(QTableWidgetItem * item)
{
  Q_UNUSED(item);
  emit changed();
}

Config::~Config()
{
}

void Config::apply()
{
  if(m_changed) {
    Store::deleteActions();
    for(int i = 0; i < (tblActions->rowCount()); i++) {
      Store::writeAction(tblActions->item(i, 1)->text(),
                         tblActions->item(i, 2)->text(),
                         tblActions->item(i, 3)->text(),
                         qobject_cast<QCheckBox*>(tblActions->cellWidget(i, 0))->isChecked());
    }
    Store::Global global;
    global.saveBeforeRun = chkSaveBeforeRun->isChecked();
    global.showNonParsed = cmbNonParsed->currentIndex();
    Store::writeGlobal(global);
    Plugin::self().refreshActions();
    m_changed = false;
  }
}

void Config::reset()
{
  Store::Action ac;
  foreach(QString name, Store::actionNames()) {
    ac = Store::readAction(name);
    addItem(ac);
  }
  Store::Global global = Store::readGlobal();
  chkSaveBeforeRun->setChecked(global.saveBeforeRun);
  cmbNonParsed->setCurrentIndex(global.showNonParsed);
}

void Config::defaults()
{

}


namespace Store
{

Global readGlobal()
{
  KConfig _config( "codeinfo", KConfig::NoGlobals, "appdata" );
  KConfigGroup config = _config.group("global");
  Global global;
  global.saveBeforeRun = (config.readEntry("saveBeforeRun","true") == "true");
  global.showNonParsed = config.readEntry("showNonParsed", 1);
  return global;
}

void writeGlobal(const Global& global)
{
  KConfig _config( "codeinfo", KConfig::NoGlobals, "appdata" );
  KConfigGroup config = _config.group("global");
  config.writeEntry("saveBeforeRun", global.saveBeforeRun);
  config.writeEntry("showNonParsed", global.showNonParsed);
}

void writeAction(const QString& name, const QString& command, const QString& regex, bool enabled)
{
  KConfig _config( "codeinfo", KConfig::NoGlobals, "appdata" );
  KConfigGroup config;
  if (name == "global") {
    config= _config.group("global_");
  } else {
    config = _config.group(name);
  }
  config.writeEntry("command", command);
  config.writeEntry("regex", regex);
  config.writeEntry("enabled", enabled);
  QList<QString> names = actionNames();
  if (!names.contains(name)) {
    names.append(name);
    _config.group("global").writeEntry("actions", names);
  }
}

void deleteActions()
{
  KConfig _config( "codeinfo", KConfig::NoGlobals, "appdata" );
  foreach(QString name, actionNames()) {
    deleteAction(name, false);
  }
  _config.group("global").writeEntry("actions", QList<QString>());
}

void deleteAction(const QString& name, bool removeFromList)
{
  KConfig _config( "codeinfo", KConfig::NoGlobals, "appdata" );
  _config.deleteGroup(name);
  QList<QString> names = actionNames();
  names.removeOne(name);
  if (removeFromList) {
    _config.group("global").writeEntry("actions", names);
  }
}

QList<QString> actionNames()
{
  KConfig _config( "codeinfo", KConfig::NoGlobals, "appdata" );
  return _config.group("global").readEntry("actions", QStringList());
}

Action readAction(const QString& name)
{
  KConfig _config( "codeinfo", KConfig::NoGlobals, "appdata" );
  Action ac;
  if (_config.hasGroup(name)) {
    KConfigGroup config(&_config, name);        
    ac.name = name;
    ac.enabled = (config.readEntry("enabled", "none") == "true");
    ac.command = config.readEntry("command", "");
    ac.regex = config.readEntry("regex", "");
  } else {
    ac.name = name;
    ac.command = i18n("Not found");
    ac.regex = i18n("Not found");
  }
  return ac;
}

};

};

// kate: space-indent on; indent-width 2; replace-tabs on;
