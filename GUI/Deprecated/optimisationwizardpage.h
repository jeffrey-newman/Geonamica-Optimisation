#ifndef OPTIMISATIONWIZARDPAGE_H
#define OPTIMISATIONWIZARDPAGE_H

#include <QWizardPage>
#include "OptimiserController.h"
#include "GeonamicaPolicyParametersQtMetaTyping.hpp"
#include <QList>
#include <boost/scoped_ptr.hpp>

namespace Ui {
class OptimisationWizardPage;
}

class OptimisationWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit OptimisationWizardPage(QWidget *parent = 0);
    ~OptimisationWizardPage();
    void initialise(int argc, char *argv[]);
    void newFile();
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }

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
    void on_pushButton_clicked();

signals:
    void runOptimisation(double val);
    void contentsChanged();

private:
    Ui::OptimisationWizardPage *ui;
//    QList<OptimiserController *> controllers;
    boost::scoped_ptr<ZonalPolicyParameters> params;
    bool is_modified;
    LoadParameters parameter_loader;
};

#endif // OPTIMISATIONWIZARDPAGE_H
