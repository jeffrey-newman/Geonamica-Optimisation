/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <boost/scoped_ptr.hpp>
#include "OptimiserController.h"
#include "GeonamicaPolicyParametersQtMetaTyping.hpp"

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
QT_END_NAMESPACE

class ProblemSpecPage;
class GeonSettingsPage;
class EAPage;
class MainWindow;

extern ZonalPolicyParameters dummy_params;

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    ConfigDialog(MainWindow * _main_window, ZonalPolicyParameters& _params = dummy_params);
    void initialise(int argc, char *argv[]);
    void newFile();
    bool loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }

public slots:
    bool save();
    bool saveAs();
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void changeTimeoutCmd(QString new_val);
    void changeWineCmd(QString new_val);
    void changeGeonCmd(QString new_val);
    void changeResetAndSave(int do_save_and_reset);
    void changeGeoprojDirectory(QString new_val);
    void changeWorkingDirectory(QString new_val);
    void changePrefixPath(QString new_val);
    void changeWhetherPrefixEnvVarSet(int new_val);
    void changeGeoprojectFile(QString new_val);
    void changeLogFileForObjective(QString new_val);
    void changeLogFileForPostOptimisationSave(QString new_val);
    void changeNumberReplicates(int new_val);
    void changeObjectiveMaps(QVector<QString> new_val);
//    void changeObjectiveMaps(QListWidgetItem *current, QListWidgetItem *previous);
    void changeZonalLayer(QString new_val);
    void changeZoneDelineation(QString new_val);
    void changeXPathDVs(QVector<QString> new_val);
//    void changeXPathDVs(QListWidgetItem *current, QListWidgetItem *previous);
    void changeDoLog(int do_log);
    void changeSaveDir(QString new_val);
    void changeSaveMaps(QVector<QString> new_val);
//    void changeSaveMaps(QListWidgetItem *current, QListWidgetItem *previous);
    void changePopSize(int new_val);
    void changeMaxGenHypImprove(int new_val);
    void changeMaxGen(int new_val);
    void changeSaveFreq(int new_val);
    void changeRessed(QString new_val);
    void changeYearStart(QString new_val);
    void changeYearEnd(QString new_val);
    void changeDiscountRate(QString new_val);
    void changeObjModules(QVector<QString> new_vals);



private:
    void createIcons();

    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
    ProblemSpecPage * problem_spec_page;
    GeonSettingsPage * geon_setting_page;
    EAPage * ea_Page;
    MainWindow * main_window;
    QTextEdit * help_widget;
    OptimiserController * optsn_cntrl;


protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void documentWasModified();

private:
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);


    QString curFile;
    bool isUntitled;

private slots:
    void run();
    void step();
    void test();

signals:
    void runOptimisation(double val);
    void contentsChanged();

private:
//    Ui::OptimisationWizardPage *ui;
//    QList<OptimiserController *> controllers;
    boost::scoped_ptr<ZonalPolicyParameters> params;
    bool is_modified;
    bool opt_needs_initialisation;
    LoadParameters parameter_loader;
};

#endif
