#include "Data.h"

Data::Data(const QString &workingDir, const Config &config) : _workingDir(workingDir), _config(config) {}

void Data::createDocumentation() {
    QString outputPath = _config.outputPath;
    QFileInfo fi(outputPath);
    QString dir = fi.isRelative() ? _workingDir + "/" + outputPath : outputPath;

    QString htmlBase = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>${TITLE}</title>
    <style>${STYLE}</style>
</head>
<body>
    <h1>${TITLE}</h1>
    <h2>Numeric variables</h2>
    ${VARS}
    <h2>String variables</h2>
    ${STRINGVARS}
    <h2>Constants</h2>
    ${CONSTS}
    <h2>Curves</h2>
    ${CURVES}
</body>
</html>
    )HTML";

    QFile style(":/Style.css"); style.open(QFile::ReadOnly);
    QString stylesheet = style.readAll();
    style.close();

    stylesheet.replace("\r\n", "");

    QString title = _config.title;
    htmlBase.replace("${TITLE}", title);
    htmlBase.replace("${STYLE}", stylesheet);

    htmlBase.replace("${VARS}", createAlphabeticVarlist(vars));
    htmlBase.replace("${STRINGVARS}", createAlphabeticVarlist(stringVars));
    htmlBase.replace("${CONSTS}", createAlphabeticConstlist(consts));
    htmlBase.replace("${CURVES}", createAlphabeticCurvelist(curves));

    QDir folder(dir);
    if(!folder.exists())
        folder.mkpath(dir);

    QFile f(dir + "/index.html");
    if(!f.open(QFile::WriteOnly)) {
        qWarning() << "\033[31mCannot create index.html:" << f.errorString() << "\033[0m";
        return;
    }

    f.write(htmlBase.toUtf8());
    f.close();
}

QString Data::createAlphabeticVarlist(const QHash<QString, Var> &vars) {
    QList<Var> list = vars.values();
    std::sort(list.begin(), list.end(), [](const Var &a, const Var &b) {return a.name < b.name;});

    QString html = "<table><tr><th>Name</th><th>Description</th><th>File</th></tr>";

    for(const Var &var : std::as_const(list)) {
        QString name = var.name;
        if(var.deleted)
            name = "<del>" + name + "</del>";

        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(name, var.documentation, var.sourceFile);
    }

    html += "</table>";
    return html;
    return "";
}

QString Data::createAlphabeticConstlist(const QHash<QString, Const> &vars) {
    QList<Const> list = vars.values();
    std::sort(list.begin(), list.end(), [](const Var &a, const Var &b) {return a.name < b.name;});

    QString html = "<table><tr><th>Name</th><th>Value</th><th>Description</th><th>File</th></tr>";

    for(const Const &constVal : std::as_const(list)) {
        QString doc = constVal.documentation;
        doc.replace("\r\n", "<br />");
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>").arg(constVal.name).arg(constVal.value).arg(doc).arg(constVal.sourceFile);
    }

    html += "</table>";
    return html;
}

QString Data::createAlphabeticCurvelist(const QHash<QString, Curve> &curves) {
    QList<Curve> list = curves.values();
    std::sort(list.begin(), list.end(), [](const Var &a, const Var &b) {return a.name < b.name;});

    QString html = "<table><tr><th>Name</th><th>Description</th><th>File</th></tr>";

    for(const Curve &curveVal : std::as_const(list)) {
        QString doc = curveVal.documentation;
        doc.replace("\r\n", "<br />");
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(curveVal.name, doc, curveVal.sourceFile);
    }

    html += "</table>";
    return html;
}
