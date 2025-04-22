#ifndef WORKER_H
#define WORKER_H

#include <QString>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QRegularExpression>

#include "Data.h"
#include "Config.h"
enum FileType {
    ScriptFile,
    VarlistFile,
    StringVarlistFile,
    ConstFile,
    VarlistDocFile,
    InvalidFile
};

class WorkingFiles {
public:
    void printAll();

    QStringList scriptFiles;
    QStringList varlists;
    QStringList stringVarlists;
    QStringList constFiles;
    QStringList varlistDocs;
};

/// The worker does all the heavy work :)))
class Worker {
public:
    Worker(const QString &workingDir);

    void start();

protected:

    /**
     * @brief Creates and returns a WorkingFiles object
     * @param path The path to work in
     * @param recursive Work recursively? (includes sub-directories
     * @return The WorkingFiles object
     */
    WorkingFiles getFiles(const QString &path,
                          const bool &recursive = false,
                          WorkingFiles workingFiles = WorkingFiles());

    /**
     * @brief Determines the type of a file
     * @param filePath The file's path
     * @return The files type
     */
    FileType fileType(const QString &filePath);

    /// reads all vars
    void readVarlists();

    void processVarlist(const QString &fileName, const bool &isStringVarlist);

    void readConstFiles();

    /// reads all var documentations
    void readVarDocs();

private:
    Config _config;

    QString _workingDir;
    WorkingFiles _workingFiles;
    Data _data;
};

#endif // WORKER_H
