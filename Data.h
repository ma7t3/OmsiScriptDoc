#ifndef DATA_H
#define DATA_H

#include <QStringList>
#include <QSet>
#include <QDir>
#include <QFile>
#include <QDebug>

#include "Config.h"

/// represents a reference to any symbol
class SourceReference {
public:
    enum Type {
        ReadType,
        WriteType,
        InvokeType
    };

    bool operator==(const SourceReference &other) {
        return file == other.file &&
               lineNumber == other.lineNumber &&
               type == other.type;
    }

    QString file;
    int lineNumber;
    Type type;
};

inline uint qHash(const SourceReference &sourceRef, uint seed = 0) {return qHashMulti(seed, sourceRef.file, sourceRef.lineNumber, sourceRef.type);}

/// Represents the parameters a macro accepts via the stacks
class MacroParameters {
public:
    MacroParameters(const QStringList &varStack = QStringList(),
                    const QStringList &stringVarStack = QStringList())
        : varStack(varStack),
          stringVarStack(stringVarStack){}
    QStringList varStack;
    QStringList stringVarStack;
};

class MacroDocumentation {
public:
    QString description;
    MacroParameters parameters;
    QString returnDescription;
};

/// represents one value (var, stringvar or const)
class Macro {
public:
    Macro(const QString &name, const QString &sourceFile)
        : name(name), sourceFile(sourceFile) {}

    bool operator==(const Macro &other) const {return name == other.name && sourceFile == other.sourceFile;}

    QString name, sourceFile;
    QSet<SourceReference> references;
    MacroDocumentation documentation;
};

inline uint qHash(const Macro &macro, uint seed = 0) {return qHashMulti(seed, macro.name, macro.sourceFile);}

/// represents one var (var, stringvar)
class Var {
public:
    Var() {}
    Var(const QString &name, const QString &sourceFile)
        : name(name), sourceFile(sourceFile) {}

    bool operator==(const Var &other) const {return name == other.name && sourceFile == other.sourceFile;}

    QString name, sourceFile;
    QSet<SourceReference> references;
    QString documentation;
    bool deleted = false;
};


/// represents a curve
class Curve : public Var {
public:
    Curve(const QString &name, const QString &sourceFile) : Var(name, sourceFile) {}
};

/// represents one const
class Const : public Var {
public:
    Const(const QString &name, const float &value, const QString &sourceFile) : Var(name, sourceFile),
        value(value) {}

    float value;
};


inline uint qHash(const Var &value, uint seed = 0) {return qHashMulti(seed, value.name, value.sourceFile);}

class Data {
public:
    Data(const QString &workingDir, const Config &config);

    void createDocumentation();

protected:
    static QString createAlphabeticVarlist(const QHash<QString, Var> &vars);
    static QString createAlphabeticConstlist(const QHash<QString, Const> &consts);
    static QString createAlphabeticCurvelist(const QHash<QString, Curve> &curves);

public:
    QHash<QString, Var> vars, stringVars;
    QHash<QString, Const> consts;
    QHash<QString, Curve> curves;
    QHash<QString, Macro> macros;

private:
    QString _workingDir;
    Config _config;
};

#endif // DATA_H
