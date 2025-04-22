#ifndef DATA_H
#define DATA_H

#include <QStringList>
#include <QSet>

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
    Var(const QString &name, const QString &sourceFile)
        : name(name), sourceFile(sourceFile) {}

    bool operator==(const Var &other) const {return name == other.name && sourceFile == other.sourceFile;}

    QString name, sourceFile;
    QSet<SourceReference> references;
    QString documentation;
    bool deleted = false;
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
    Data();

    void createDocumentation();

    QSet<Var> vars, stringVars, consts;
    QSet<Macro> macros;
};

#endif // DATA_H
