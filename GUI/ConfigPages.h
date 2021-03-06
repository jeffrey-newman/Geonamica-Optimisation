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

#ifndef PAGES_H
#define PAGES_H

#include <QWidget>
#include <ConfigDialog.h>


class MyLineEdit : public QLineEdit
{
Q_OBJECT

public:
    MyLineEdit(QWidget *parent = 0);
    ~MyLineEdit();

signals:
    void focussed(bool hasFocus);

protected:
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
};


class ProblemSpecPage : public QWidget
{
Q_OBJECT

public:
    ProblemSpecPage(ConfigDialog* config_dialogue, QTextEdit * _help_box, QWidget *parent = 0);

    private slots:
    void addObjectiveMap();
    void addXPathDV();
    void addYear();
    void processObjectiveMapListChange();
//    void processYearListChange();
    void processOXPathDVListChange();
    void addObjModule();
    void addDVModule();
    void processObjModuleChange();
    void displayObjMapHelp();
    void displayObjModuleHelp();
    void processDVModuleChange();
    void displayDVModuleHelp();
    void displayXPathDVsHelp();
    void displayZonalOptimisationHelp();

public:
    void updateObjectiveMaps(std::vector<std::string> obj_maps);
//    void updateDiscountRate(double rate);
//    void updateDiscountYearZero(int year);
    void updateZoneDelineationMap(QString path);
    void updateZonalLayerMap(QString path);
    void updateZonalMapClasses(QString values);
    void updateXpathDVs(std::vector<std::string> xpathDVs);
    void updateObjModules(std::vector<std::string> obj_modules);
    void updateDVModules(std::vector<std::string> obj_modules);
//    void updateYearsCalculated(std::vector<int> years);
//    void updateStartYear(int year);
//    void updateEndYear(int year);

signals:
    void objectiveMapsChanged(QVector<QString> objMapsLists);
//    void yearsCalculatedChanged(QVector<QString> yearsList);
    void xpathDVsChanged(QVector<QString> xpathDVLists);
    void objModulesChanged(QVector<QString> xpathDVLists);
    void dvModulesChanged(QVector<QString> xpathDVLists);

private:
    QListWidget *objectives_List;
    QListWidget *years_List;
    QListWidget *evaluator_modules_list;
    QListWidget *dv_modules_list;
    QListWidget *xpath_List;
    MyLineEdit *rate_edit;
    MyLineEdit *discount_year_zero_edit;
//    QLineEdit *year_begin_metrics_edit;
//    QLineEdit *year_end_metrics_edit;
    MyLineEdit *zone_delineation_edit;
    MyLineEdit *zonal_layer_edit;
    MyLineEdit * zonal_map_classes_edit;
    QTextEdit * help_box;

};

class GeonSettingsPage : public QWidget
{
Q_OBJECT

public:
    GeonSettingsPage(ConfigDialog* config_dialogue,  QTextEdit * _help_box, QWidget *parent = 0);

    private slots:
    void addOutputLogMap();
    void processOutputLogMapListChange();
    void displaySaveMapHelp();
    void displayTimeoutHelp();
    void displayWineCmdHelp();
    void displayGeonamicaCmdHelp();
    void displayWorkDirHelp();
    void displayWinePrefixHelp();
    void displayDriverLetterHelp();


    public:
    void updateTimeoutCmd(QString path);
    void updateWineCmd(QString path);
    void updateGeonCmd(QString path);
    void updateworkingDir(QString path);
    void updateWinePrefix(QString path);
    void updateSaveDir(QString path);
    void updateTestDir(QString path);
    void updateWhetherPrefixEnvVarSet(bool  do_set);
    void updateWindowsEnvVar(QString val);
    void updateWhetherLog(bool do_log);
    void updateGeoprojDir(QString path);
    void updateGeoprojFile(QString path);
    void updateObjLogFile(QString path);
    void updatePostOptPrintLogFile(QString path);
//    void updateStartYear(int year);
//    void updateEndYear(int year);
    void updateNumberReplicates(int number);
    void updateOuputLogMaps(std::vector<std::string> ouput_maps);
    void updateDriveLetter(QString letter);
    void updateWhetherThrowExceptns(bool do_throw);

    signals:
    void outputLogMapsChanged(QVector<QString> logMapsLists);

private:
    QListWidget *save_map_List;
    MyLineEdit *timeout_edit;
    MyLineEdit *wine_edit;
    MyLineEdit *geon_edit;
    MyLineEdit *working_dir_edit;
    MyLineEdit *wine_prefix_edit;
    MyLineEdit *saving_dir_edit;
    MyLineEdit *testing_dir_edit;
    QCheckBox* prefix_env_var_CheckBox;
    MyLineEdit *windows_env_var_edit;
    QCheckBox* log_checkbox;
    MyLineEdit *geoproj_directory_edit;
    MyLineEdit *geoproj_file_edit;
    MyLineEdit *obj_log_file_edit;
    MyLineEdit *plot_log_file_edit;
//    MyLineEdit *year_begin_save_edit;
//    MyLineEdit *year_end_save_edit;
    QSpinBox *replicates_SpinBox;
    QTextEdit * help_box;
    MyLineEdit * driver_letter_edit;
    QCheckBox* do_throw_exceptns_CheckBox;

};

class EAPage : public QWidget
{
Q_OBJECT

private slots:
    void addEmailAddress();
    void processEmailAddressListChange();

public:
    EAPage(ConfigDialog* config_dialogue, QTextEdit * _help_box, QWidget *parent = 0);

    public:
    void updateAlgorithm(QString algorithm);
    void updatePop(int pop);
    void updateHyprvolTerm(int gen);
    void updateMaxGenTerm(int gen);
    void updateReseedFile(QString path);
    void updateLoggingFreq(int freq);
    void updateEmailAddresses(std::vector<std::string> email_addresses);

signals:
    void emailAddressListChanged(QVector<QString> emailAddressList);

private:
    QSpinBox *pop_SpinBox;
    QSpinBox *hyprvol_term_SpinBox;
    QSpinBox *term_SpinBox;
    QLineEdit *reseed_edit;
    QSpinBox *logging_SpinBox;
    QTextEdit * help_box;
    QListWidget *email_address_List;
    QComboBox * algorithm_selector;

};

#endif
