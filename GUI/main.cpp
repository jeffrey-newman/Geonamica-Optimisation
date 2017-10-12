#include <iostream>
#include <QApplication>
#include <QCommandLineParser>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Geonamica Optimisation Utility");
    QCoreApplication::setOrganizationName("EnvirocareInformatics / University of Adelaide");
    QCoreApplication::setApplicationVersion("0.1 Alpha");

    qRegisterMetaType<QVector<std::pair<double, double> >>();

    MainWindow main_win;
    main_win.show();

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("cfg-file", QCoreApplication::translate("main", "Configuration file for optimisation"));
    parser.process(a);
    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() == 1) {
        QString cfg_file = positionalArguments.at(0);
        main_win.openFile(cfg_file);
    }
    if (positionalArguments.size() > 1) {
        std::cout << "Several command line arguments specified.";
        return EXIT_FAILURE;
    }

    return a.exec();
}
