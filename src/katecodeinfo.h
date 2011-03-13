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

#ifndef KATE_CODEINFO_H
#define KATE_CODEINFO_H

#include <ktexteditor/document.h>
#include <ktexteditor/configpage.h>
#include <kate/plugin.h>
#include <kate/pluginconfigpageinterface.h>
#include <kate/mainwindow.h>

#include "ui_codeinfowidget.h"
#include "ui_codeinfoconfigwidget.h"

#include <QString>
#include <QTimer>

class KateCodeinfoPlugin: public Kate::Plugin, public Kate::PluginConfigPageInterface
{
    Q_OBJECT
    Q_INTERFACES(Kate::PluginConfigPageInterface)
  public:
    explicit KateCodeinfoPlugin( QObject* parent = 0, const QList<QVariant>& = QList<QVariant>() );
    virtual ~KateCodeinfoPlugin();

    static KateCodeinfoPlugin& self();

    Kate::PluginView *createView (Kate::MainWindow *mainWindow);

  signals:
    void newStatus(const QString&);

  //
  // PluginConfigPageInterface
  //
  public:
    virtual uint configPages() const;
    virtual Kate::PluginConfigPage* configPage (uint number = 0, QWidget *parent = 0, const char *name = 0);
    virtual QString configPageName(uint number = 0) const;
    virtual QString configPageFullName(uint number = 0) const;
    virtual KIcon configPageIcon(uint number = 0) const;

  //
  // private data
  //
  private:
    static KateCodeinfoPlugin* s_self;
};

class KateCodeinfoPluginView : public Kate::PluginView, public Ui::CodeinfoWidget
{
    Q_OBJECT

  public:
    KateCodeinfoPluginView(Kate::MainWindow* mainWindow);

    ~KateCodeinfoPluginView();

    virtual void readSessionConfig (KConfigBase* config, const QString& groupPrefix);
    virtual void writeSessionConfig (KConfigBase* config, const QString& groupPrefix);

    void loadBacktrace(const QString& bt);

  public slots:
    void loadFile();
    void loadClipboard();
    void run();
    void clearStatus();
    void setStatus(const QString& status);

  private slots:
    void itemActivated(QTreeWidgetItem* item, int column);

  private:
    QWidget* toolView;
    Kate::MainWindow* mw;
    QTimer timer;
};

class KateCodeInfoConfigWidget : public Kate::PluginConfigPage, private Ui::CodeinfoConfigWidget
{
    Q_OBJECT
  public:
    explicit KateCodeInfoConfigWidget(QWidget* parent = 0, const char* name = 0);
    virtual ~KateCodeInfoConfigWidget();

  public slots:
    virtual void apply();
    virtual void reset();
    virtual void defaults();

  private slots:
    virtual void hasChanged();

  private:
    bool m_changed;
};

class KateCodeInfoConfigDialog : public KDialog
{
    Q_OBJECT
  public:
    KateCodeInfoConfigDialog(QWidget* parent = 0);
    ~KateCodeInfoConfigDialog();

  public slots:
    void changed();

private:
    KateCodeInfoConfigWidget* m_configWidget;
};

#endif //KATE_CODEINFO_H

// kate: space-indent on; indent-width 2; replace-tabs on;
