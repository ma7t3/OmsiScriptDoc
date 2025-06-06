#include "Worker.h"

void WorkingFiles::printAll() {
    qInfo().noquote() << QString("Script files (%1):\033[1;34m").arg(scriptFiles.count());
    qInfo().noquote() << scriptFiles.join("\r\n");

    qInfo().noquote() << QString("\r\n\033[0mVarlist files (%1):\033[1;34m").arg(varlists.count());
    qInfo().noquote() << varlists.join("\r\n");

    qInfo().noquote() << QString("\r\n\033[0mStringvarlist files (%1):\033[1;34m")
                             .arg(stringVarlists.count());
    qInfo().noquote() << stringVarlists.join("\r\n");

    qInfo().noquote() << QString("\r\n\033[0mConstfiles (%1):\033[1;34m").arg(constFiles.count());
    qInfo().noquote() << constFiles.join("\r\n");

    qInfo().noquote() << QString("\r\n\033[0mVarlist documentation files (%1):\033[1;34m")
                             .arg(varlistDocs.count());
    qInfo().noquote() << varlistDocs.join("\r\n") + "\033[0m";
}


Worker::Worker(const QString &workingDir) :
    _config(workingDir), _workingDir(workingDir), _data(workingDir, _config) {
}

void Worker::start() {
    qInfo() << "Working Directory:" << _workingDir;
    bool recursive = _config.recursive;
    qInfo() << "recursive:" << recursive;

    qInfo() << "\r\n\033[1;34mIndexing files...\033[0m";
    _workingFiles = getFiles(_workingDir, recursive);
    _workingFiles.printAll();

    qInfo() << "\r\n\033[1;34mReading varlists...\033[0m";
    readVarlists();

    qInfo() << "\r\n\033[1;34mReading var documentation files...\033[0m";
    readVarDocs();

    qInfo() << "\r\n\033[1;34mReading constfiles...\033[0m";
    readConstFiles();

    qInfo() << "\r\n\033[1;34mCreating documentation files...\033[0m";

    _data.createDocumentation();

    qInfo() << "\033[1;32mFinished!\033[0m";
}

FileType Worker::fileType(const QString &filePath) {
    const QString fileName = filePath.split("/").last();

    if (fileName.endsWith(".osc"))
        return ScriptFile;

    if (fileName.endsWith(".vardoc"))
        return VarlistDocFile;

    if (!fileName.endsWith(".txt"))
        return InvalidFile;

    static const QStringList stringVarlistParts = {"stringvarlist",
                                                   "stringvarnamelist",
                                                   "stringlist"};
    static const QStringList varlistParts   = {"varlist", "varnamelist", "varlistfile", "varnamelistfile"};
    static const QStringList constFileParts = {"constfile", "constlist"};

    bool stringVarlist = false, varlist = false, constFile = false;
    for (const QString &current : stringVarlistParts)
        stringVarlist |= fileName.contains(current);

    for (const QString &current : varlistParts)
        varlist |= fileName.contains(current);

    for (const QString &current : constFileParts)
        constFile |= fileName.contains(current);

    if(constFile)
        return ConstFile;
    if(stringVarlist)
        return StringVarlistFile;
    if(varlist)
        return VarlistFile;

    return InvalidFile;
}

void Worker::readVarlists() {
    for(const QString &currentFile : std::as_const(_workingFiles.varlists))
        processVarlist(currentFile, false);

    for(const QString &currentFile : std::as_const(_workingFiles.stringVarlists))
        processVarlist(currentFile, true);

    qInfo() << "\033[1;32mFound" << _data.vars.size() << "vars.\033[0m";
    qInfo() << "\033[1;32mFound" << _data.stringVars.size() << "stringvars.\033[0m";
}

void Worker::processVarlist(const QString &fileName, const bool &isStringVarlist) {
    QFile f(_workingDir + "/" + fileName);
    if(!f.open(QFile::ReadOnly)) {
        qWarning() << "\033[31mCannot read varlist:" << fileName << "Reason:" << f.errorString() << "\033[0m";
        return;
    }

    const QStringList list = QString(f.readAll()).split("\r\n", Qt::SkipEmptyParts);

    for(const QString &currentVar : list) {
        Var var(currentVar, fileName);
        if(isStringVarlist)
            _data.stringVars.insert(currentVar, var);
        else
            _data.vars.insert(currentVar, var);
    }

    f.close();
}

void Worker::readConstFiles() {
    for(const QString &currentFile : std::as_const(_workingFiles.constFiles)) {
        QFile f(_workingDir + "/" + currentFile);
        if(!f.open(QFile::ReadOnly)) {
            qWarning() << "\033[31mCannot read constfile:" << currentFile << "Reason:" << f.errorString() << "\033[0m";
            continue;
        }

        const QStringList list = QString(f.readAll()).split("\r\n");

        for(int i = 0; i < list.size(); i++) {
            const QString currentLine = list[i];

            if(currentLine != "[const]" && currentLine != "[newcurve]")
                continue;

            if(i + 2 >= list.size()) {
                qWarning() << "Invalid const/curve definition. Early end of file:" << currentFile;
                continue;
            }

            const QString name     = list[i + 1];
            const QString valueStr = list[i + 2];
            bool ok;
            float value = valueStr.toFloat(&ok);
            if(!ok && currentLine == "[const]")
                qWarning().noquote() << QString("Invalid const value \"%1\" in file %2 (line %3).").arg(valueStr, currentFile, QString::number(i + 2));

            QString documentation;
            for(int k = i - 1; k >= 0; k--) {
                if(k - 2 >= 0) {
                    const QString lastDef = list[k - 2];
                    if(lastDef == "[const]" || lastDef == "[newcurve]" || lastDef == "[pnt]")
                        break;
                }

                const QString currentLine = list[k];
                QString cleanLine = currentLine;
                cleanLine.remove(QRegularExpression("\\s"));
                if(cleanLine.isEmpty())
                    break;

                documentation = currentLine + "\r\n" + documentation;
            }

            if(currentLine == "[const]") {
                Const c(name, value, currentFile);
                c.documentation = documentation;
                _data.consts.insert(name, c);
            } else {
                Curve c(name, currentFile);
                c.documentation = documentation;
                _data.curves.insert(name, c);
            }
        }

        f.close();
    }

    qInfo() << "\033[1;32mFound" << _data.consts.size() << "consts and" << _data.curves.size() << "curves.\033[0m";
}


void Worker::readVarDocs() {
    for(const QString &currentFile : std::as_const(_workingFiles.varlistDocs)) {
        QFile f(_workingDir + "/" + currentFile);
        if(!f.open(QFile::ReadOnly)) {
            qWarning() << "\033[31mCannot read var documentation file:" << currentFile << "Reason:" << f.errorString() << "\033[0m";
            continue;
        }
        f.close();

        QSettings s(f.fileName(), QSettings::IniFormat);
        s.beginGroup("vars");
        const QStringList keys = s.allKeys();

        for(const QString &key : keys) {
            const QString value = s.value(key).toString();

            if(_data.vars.contains(key))
                _data.vars[key].documentation = value;
            else {
                Var var(key, "");
                var.documentation = value;
                var.deleted = true;
                _data.vars.insert(key, var);
            }
        }
        s.endGroup();

        // TODO: Improve
        s.beginGroup("stringvars");
        const QStringList stringKeys = s.allKeys();

        for(const QString &key : stringKeys) {
            const QString value = s.value(key).toString();

            if(_data.stringVars.contains(key))
                _data.stringVars[key].documentation = value;
            else {
                Var var(key, "");
                var.documentation = value;
                var.deleted = true;
                _data.stringVars.insert(key, var);
            }
        }
        s.endGroup();
    }
}

WorkingFiles Worker::getFiles(const QString &path, const bool &recursive, WorkingFiles workingFiles) {
    QDir dir(path);
    const QStringList allFiles = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString &currentFile : allFiles) {
        FileType type = fileType(currentFile);
        switch (type) {
        case ScriptFile:
            workingFiles.scriptFiles.append(currentFile);
            break;
        case VarlistFile:
            workingFiles.varlists.append(currentFile);
            break;
        case StringVarlistFile:
            workingFiles.stringVarlists.append(currentFile);
            break;
        case ConstFile:
            workingFiles.constFiles.append(currentFile);
            break;
        case VarlistDocFile:
            workingFiles.varlistDocs.append(currentFile);
            break;
        case InvalidFile:
            break;
        }
    }

    if (!recursive)
        return workingFiles;

    const QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &currentDir : subDirs)
        workingFiles = getFiles(path + "/" + currentDir, recursive, workingFiles);

    return workingFiles;
}
