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

static CodeinfoInfo parseCodeinfoLine(const QString& line, const QString& regex)
{
  // the syntax types we support are
  // filename \t line number \t column number \t code \t message
  kDebug() << regex ;
  QStringList parsed = line.split("\t");
  if (parsed.count() > 4) {
    CodeinfoInfo info;
    info.filename = parsed[0];
    info.line = parsed[1].toInt();
    info.col = parsed[2].toInt();
    info.code = parsed[3];
    for (int i=4; i<parsed.count(); i++) {
        info.message.append(parsed[i]);
    }
    return info;
  }

  kDebug() << "Unknown codeinfo line:" << line;

  CodeinfoInfo info;
  return info;
}

QList<CodeinfoInfo>  KateCodeinfoParser::parseCodeinfo(const QString& ci, const QString& regex)
{
  QStringList l = ci.split(eolDelimiter(ci), QString::SkipEmptyParts);

  //l = normalizeBt(l);

  QList<CodeinfoInfo> results;
  for (int i = 0; i < l.size(); ++i) {
    CodeinfoInfo info = parseCodeinfoLine(l[i], regex);
    if (info.line >= 0) {
      results.append(info);
    }
  }

  return results;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
