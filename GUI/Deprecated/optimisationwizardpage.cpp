#include "optimisationwizardpage.h"
#include "ui_optimisationwizardpage.h"
#include <iostream>

OptimisationWizardPage::OptimisationWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::OptimisationWizardPage),
    params(new ZonalPolicyParameters)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    isUntitled = true;
}

void OptimisationWizardPage::initialise(int argc, char **argv)
{

    int err = parameter_loader.processOptions(argc, argv, *params);
    if (err != 0)
    {
        throw std::runtime_error("Could not parse options. Check file and path");
    }
    params->evaluator_id = 0;

}

OptimisationWizardPage::~OptimisationWizardPage()
{
    delete ui;
}

void OptimisationWizardPage::on_pushButton_clicked()
{
    std::cout << "Button clicked" << std::endl;
    OptimiserController * c(new OptimiserController(*params));
//    controllers.push_back(c);
    c->setParent(this);
}

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

//﻿OptimisationWizardPage::OptimisationWizardPage()
//{
//
//}

void OptimisationWizardPage::newFile()
{
    static int sequenceNumber = 1;

    isUntitled = true;
    curFile = tr("document%1.txt").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]");
    is_modified = false;

    connect(this, &OptimisationWizardPage::contentsChanged,
            this, &OptimisationWizardPage::documentWasModified);
}

bool OptimisationWizardPage::loadFile(const QString &fileName)
{



    //While reading in file, if long, change cursor.
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // Open file and read in values into params struct.
    parameter_loader.processOptions(fileName.toStdString(), *params);
    //Return cursor to normal.
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);

    is_modified = false;

    //
    connect(this, &OptimisationWizardPage::contentsChanged,
             this, &OptimisationWizardPage::documentWasModified);

    return true;
}

bool OptimisationWizardPage::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool OptimisationWizardPage::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    curFile);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool OptimisationWizardPage::saveFile(const QString &fileName)
{

    QApplication::setOverrideCursor(Qt::WaitCursor);

    //Save params to file here.

    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    is_modified = false;
    return true;
}

QString OptimisationWizardPage::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

void OptimisationWizardPage::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void OptimisationWizardPage::documentWasModified()
{
    setWindowModified(true);
}

bool OptimisationWizardPage::maybeSave()
{
    if (!this->is_modified)
        return true;
    const QMessageBox::StandardButton ret
            = QMessageBox::warning(this, tr("MDI"),
                                   tr("'%1' has been modified.\n"
                                              "Do you want to save your changes?")
                                           .arg(userFriendlyCurrentFile()),
                                   QMessageBox::Save | QMessageBox::Discard
                                   | QMessageBox::Cancel);
    switch (ret) {
        case QMessageBox::Save:
            return save();
        case QMessageBox::Cancel:
            return false;
        default:
            break;
    }
    return true;
}

void OptimisationWizardPage::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    this->is_modified = false;
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

QString OptimisationWizardPage::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}