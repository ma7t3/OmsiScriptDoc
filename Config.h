#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>

class Config {
public:
    Config(const QString &workingDir);
    QString outputPath;
    QString title;
    bool recursive;
};

#endif // CONFIG_H
