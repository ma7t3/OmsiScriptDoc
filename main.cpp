#include <QCoreApplication>

#include <QDebug>
#include <QString>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QSettings>

#include "Worker.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QString testPath("H:/SteamLibrary/steamapps/common/OMSI 2/Scripts/nuntius");
    Worker worker(testPath);
    worker.start();
    //a.quit();

    //return a.exec();
}
