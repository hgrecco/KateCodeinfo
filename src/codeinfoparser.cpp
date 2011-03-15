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

#include "codeinfoparser.h"

#include <QStringList>
#include <QRegExp>
#include <QHash>

#include <kdebug.h>

static QString eolDelimiter(const QString& str)
{
  // find the split character
  QString separator('\n');
  if (str.indexOf("\r\n") != -1) {
    separator = "\r\n";
  } else if (str.indexOf('\r') != -1 ) {
    separator = '\r';
  }
  return separator;
}

static CodeinfoInfo parseCodeinfoLine(const QString& line, const QRegExp& reg, const QHash<QString, int>& order)
{
  // the syntax types we support are
  // filename \t line number \t column number \t code \t message
  int index = reg.indexIn(line);
  QHash<QString, int>::const_iterator el;
  kDebug() << index;
  if (index > -1) {
    CodeinfoInfo info;
    if ((el = order.find("filename")) != order.end()) {
      info.filename = reg.cap(el.value());
    }
    if ((el = order.find("line")) != order.end()) {
      info.line = reg.cap(el.value()).toInt();
    }
    if ((el = order.find("col")) != order.end()) {
      info.col = reg.cap(el.value()).toInt();
    }
    if ((el = order.find("code")) != order.end()) {
      info.code = reg.cap(el.value());
    }
    if ((el = order.find("message")) != order.end()) {
      info.message = reg.cap(el.value());
    }
    return info;
  }
  kDebug() << "Unknown codeinfo line:" << line;

  CodeinfoInfo info;
  info.line = -1;
  return info;
}

QList<CodeinfoInfo>  KateCodeinfoParser::parseCodeinfo(const QString& ci, QString regex)
{
  QStringList l = ci.split(eolDelimiter(ci), QString::SkipEmptyParts);

  QHash<QString, int> order;
  QRegExp named("\\(P<([^<]*)>");
  kDebug() << "Regex before name transformation: " << regex;
  int pos = 0;
  int count = 0;
  while (pos >= 0) {
    pos = named.indexIn(regex, pos);
    if (pos >= 0) {
      pos += named.matchedLength();
      order[named.cap(1)] = ++count;
    }
  }
  kDebug() << "Name transformation: " << order;
  regex.replace(named, "(");
  kDebug() << "Regex after name transformation: " << regex;
  kDebug() << regex;

  QRegExp reg(regex);
  reg.setPatternSyntax(QRegExp::RegExp2);

  QList<CodeinfoInfo> results;
  for (int i = 0; i < l.size(); ++i) {
    CodeinfoInfo info = parseCodeinfoLine(l[i], reg, order);
    if (info.line >= 0) {
      results.append(info);
    }
  }

  return results;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
