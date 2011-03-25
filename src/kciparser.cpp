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

#include "kciparser.h"

#include <kdebug.h>

#include <QStringList>
#include <QRegExp>

namespace KateCodeinfo
{


NamedRegExp::NamedRegExp():QRegExp() {
}

NamedRegExp::NamedRegExp(QString& regex):QRegExp() {
  setNamedPattern(regex);
}

void NamedRegExp::setNamedPattern(QString& regex) {
  QRegExp named("\\(P<([^<]*)>");
  kDebug() << "Regex before name transformation: " << regex;
  int pos = 0;
  int count = 0;
  m_order.clear();
  while(pos >= 0) {
    pos = named.indexIn(regex, pos);
    if(pos >= 0) {
      pos += named.matchedLength();
      m_order[named.cap(1)] = ++count;
    }
  }
  regex.replace(named, "(");
  setPattern(regex);
  setPatternSyntax(QRegExp::RegExp2);
}

QString NamedRegExp::namedCap(QString groupName, QString notfound) {
  QHash<QString, int>::const_iterator el;
  if ((el = m_order.find(groupName)) != m_order.end()) {
    return cap(el.value());
  } else {
    return notfound;
  }
}

QSet<QString> NamedRegExp::namedGroups() {
  return QSet<QString>::fromList(m_order.keys());
}


static QString eolDelimiter(const QString& str)
{
  // find the split character
  QString separator('\n');
  if(str.indexOf("\r\n") != -1) {
    separator = "\r\n";
  } else if(str.indexOf('\r') != -1) {
    separator = '\r';
  }
  return separator;
}

static Info parseCodeinfoLine(QString& line, NamedRegExp& nreg)
{
  // the syntax types we support are
  // filename \t line number \t column number \t code \t message
  int index = nreg.indexIn(line);
  Info info;    

  if ((info.parsed = (index > -1))) {
    info.filename = nreg.namedCap("filename", "");
    info.line = nreg.namedCap("line", "-1").toInt();
    info.col = nreg.namedCap("col", "-1").toInt();
    info.code = nreg.namedCap("code", "");
    info.message = nreg.namedCap("message", "");
  }  else {
    info.message = line;
    info.line = -1;
  }
  return info;
}

QList<Info> parse(const QString& ci, QString regex)
{
  QStringList l = ci.split(eolDelimiter(ci), QString::SkipEmptyParts);

  NamedRegExp nreg = NamedRegExp(regex);

  QList<Info> results;
  for(int i = 0; i < l.size(); ++i) {
    Info info = parseCodeinfoLine(l[i], nreg);
    results.append(info);
  }

  return results;
};

};

// kate: space-indent on; indent-width 2; replace-tabs on;
