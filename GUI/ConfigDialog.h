

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

extern GeonamicaPolicyParameters dummy_params;

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    ConfigDialog(MainWindow * _main_window, GeonamicaPolicyParameters& _params = dummy_params);
    void initialise(int argc, char *argv[]);
    void newFile();
    void updateParamsValuesInGUI();
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
    void changeWindowsEnvVar(QString new_val);
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
    void changeTestDir(QString new_val);
    void changeSaveMaps(QVector<QString> new_val);
//    void changeSaveMaps(QListWidgetItem *current, QListWidgetItem *previous);
    void changePopSize(int new_val);
    void changeMaxGenHypImprove(int new_val);
    void changeMaxGen(int new_val);
    void changeSaveFreq(int new_val);
    void changeRessed(QString new_val);
//    void changeYearStartSave(QString new_val);
//    void changeYearEndSave(QString new_val);
//    void changeMetricYears(QVector<QString> new_vals);
//    void changeYearStartMetrics(QString new_val);
//    void changeYearEndMetrics(QString new_val);
//    void changeDiscountRate(QString new_val);
//    void changeDiscountYearZero(QString new_val);
    void changeObjModules(QVector<QString> new_vals);
    void changeDriveLetter(QString new_val);
    void changeZonalClasses(QString new_val);
    void duplicateSlot();


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
    void CheckSavingAndWorkingAndTestingDirs();


    QString curFile;
    bool isUntitled;

private slots:
    void run();
    void step();
    void test();

signals:
    void runOptimisation(double val);
    void contentsChanged();
    void duplicateSignal(GeonamicaPolicyParameters params);

private:
//    Ui::OptimisationWizardPage *ui;
//    QList<OptimiserController *> controllers;
    boost::scoped_ptr<GeonamicaPolicyParameters> params;
    bool is_modified;
    bool opt_needs_initialisation;
    LoadParameters parameter_loader;
};

#endif
