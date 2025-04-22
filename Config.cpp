#include "Config.h"


Config::Config(const QString &workingDir) {
    QSettings s(workingDir + "/omsiScriptDoc.ini", QSettings::IniFormat);
    outputPath = s.value("outputPath", "docs").toString();
    title = s.value("title", "OMSI Script Documentation").toString();
    recursive = s.value("recursive", false).toBool();
}
