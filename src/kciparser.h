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

#ifndef KCIPARSER_H
#define KCIPARSER_H

#include <QString>
#include <QHash>
#include <QRegExp>

namespace KateCodeinfo
{

class Info
{

public:
  Info(): filename(""), code(""), message(""), line(0), col(0) {};
  Info(const QString& message): filename(""), code(""), message(message), line(0), col(0) {};
  QString filename;
  QString code;
  QString message;
  int line;
  int col;
  bool parsed;
};

class NamedRegExp : public QRegExp {

private:
  QHash<QString, int> m_order;

public:
  NamedRegExp();
  NamedRegExp(QString& regex);
  QString namedCap(QString groupName, QString notfound);

  void setNamedPattern(QString &regex);
  QSet<QString> namedGroups();
};

QList<Info> parse(const QString& ci, QString regex);

};

#endif //KCIPARSER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
