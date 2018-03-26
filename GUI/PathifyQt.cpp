//
// Created by a1091793 on 28/11/17.
//

#include "PathifyQt.hpp"

#include <QMessageBox>
#include <iostream>

bool
pathifyQMsg(CmdLinePaths & path)
{
    if (path.first.empty())
    {
        QMessageBox msgBox;
        msgBox.setText("Warning: empty path given to pathifyQMsg");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStandardButtons(QMessageBox::Ok);
        int ret = msgBox.exec();
        return false;
    }
    path.second = boost::filesystem::path(path.first);
    if (!(boost::filesystem::exists(path.second)))
    {
        QMessageBox msgBox;
        msgBox.setText("Warning: path \" << path.first << \" does not exist");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStandardButtons(QMessageBox::Ok);
        int ret = msgBox.exec();
        return false;
    }
    return true;
}

bool
pathifyMkQMsg(CmdLinePaths & path)
{
    if (path.first.empty())
    {
        QMessageBox msgBox;
        msgBox.setText("Warning: empty path given to pathifyQMsg");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setStandardButtons(QMessageBox::Ok);
        int ret = msgBox.exec();
        return false;
    }
    path.second = boost::filesystem::path(path.first);
    if (!(boost::filesystem::exists(path.second)))
    {
        try
        {
            boost::filesystem::create_directories(path.second);
            std::cout << "path " << path.first << " did not exist, so created\n";
        }
        catch(boost::filesystem::filesystem_error& e)
        {
            QMessageBox msgBox;
            msgBox.setText("Attempted to create \" << path.first << \" but was unable");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setStandardButtons(QMessageBox::Ok);
            int ret = msgBox.exec();
            return false;
        }

        return true;
    }
    return true;

}
