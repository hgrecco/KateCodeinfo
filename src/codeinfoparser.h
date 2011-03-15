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

#ifndef KATE_CODEINFOPARSER_H
#define KATE_CODEINFOPARSER_H

#include <QList>
#include <QString>

class CodeinfoInfo
{

  public:
    CodeinfoInfo():line(-1),col(-1){};
    QString filename;
    QString code;
    QString message;
    int line;
    int col;
};

namespace KateCodeinfoParser {

QList<CodeinfoInfo> parseCodeinfo(const QString& ci, const QString& regex);

}

#endif //KATE_CODEINFOPARSER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
