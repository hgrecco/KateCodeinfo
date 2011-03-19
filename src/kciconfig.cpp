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

  btnRemove->setDisabled((tblActions->rowCount()) == 0);
  currentCellChanged(tblActions->currentRow(), 0, 0, 0);
  m_changed = false;
}

void Config::addItem(QString& name, QString& command, QString& regex)
{
  int row = tblActions->currentRow() + 1;
  tblActions->insertRow(row);
  tblActions->setItem(row, 0, new QTableWidgetItem(name));
  tblActions->setItem(row, 1, new QTableWidgetItem(command));
  tblActions->setItem(row, 2, new QTableWidgetItem(regex));
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
           "This will overwrite your current action list with the default one.\n"
           "Continue?",
           "&Yes", "&No", QString::null, 1, 1)) {
  case 0:
    for (int i=tblActions->rowCount(); --i >= 0; ) {
      tblActions->removeRow(i);
    }
    // (P<filename>.*):(P<line>\d+):(P<col>\d+):\s*(P<code>\w+)\s*(P<message>.*)
    content << "pep8" << "pep8 %filename" << "(P<filename>.*):(P<line>\\d+):(P<col>\\d+):\\s*(P<code>\\w+)\\s*(P<message>.*)";
    content << "pylint" << "pylint -f parseable -r n %filename" << "(P<filename>.*):(P<line>\\d+):\\s*\\[(P<code>\\w+)(?:,(?:.*))*\\]\\s+(P<message>.*)";
    content << "gcc syntax" << "gcc -fsyntax-only -Wall %filename" << "(P<filename>.*):(P<line>\\d+):(P<col>\\d+):\\s*(P<code>\\w+):\\s*(P<message>.*)";
    content << "clang syntax" << "clang -fsyntax-only -Wall -fno-caret-diagnostics %filename" << "(P<filename>.*):(P<line>\\d+):(P<col>\\d+):\\s*(P<code>\\w+):\\s*(P<message>.*)";
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
    kDebug() << tblActions->rowCount() << " Check";
    for(int i = 0; i < (tblActions->rowCount()); i++) {
      Store::writeAction(tblActions->item(i, 0)->text(),
                         tblActions->item(i, 1)->text(),
                         tblActions->item(i, 2)->text());
    }
    Plugin::self().refreshActions();
    m_changed = false;
  }
}

void Config::reset()
{
  QList<QString> list;
  foreach(QString name, Store::actionNames()) {
    list = Store::readAction(name);
    addItem(name, list[0], list[1]);
  }
}

void Config::defaults()
{

}


namespace Store
{

QList<QString> readAction(const QString& name)
{
  return KConfigGroup(KGlobal::config(), "codeinfo").readEntry(name, QList<QString>());
}

void writeAction(const QString& name, const QString& command, const QString& regex)
{
  KConfigGroup cg(KGlobal::config(), "codeinfo");
  QList<QString> list = QList<QString>();
  list << command << regex;
  cg.writeEntry(name, list);
  kDebug() << name << " written.";
}

QList<QString> actionNames() {
  return KConfigGroup(KGlobal::config(), "codeinfo").keyList();
} // Store

void deleteActions()
{
  KConfigGroup cg(KGlobal::config(), "codeinfo");
  foreach(QString key, cg.keyList()) {
    cg.deleteEntry(key);
  }
}

void deleteAction(const QString& name)
{
  KConfigGroup cg(KGlobal::config(), "codeinfo");
  cg.deleteEntry(name);
}

};

};

// kate: space-indent on; indent-width 2; replace-tabs on;
