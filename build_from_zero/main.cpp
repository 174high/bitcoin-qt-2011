#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtGui>
#include <QTranslator>
#include <QObject>
#include "bitcoingui.h"


int main(int argc, char** argv)
{
    qDebug()<<"hello qt!";

    QApplication app(argc, argv);

    QTranslator translator;
    translator.load("hellotr_la");
    app.installTranslator(&translator);

    BitcoinGUI window;
    window.show();

    return app.exec();
}
