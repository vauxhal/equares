// check_lib.cpp

#include "check_lib.h"
#include "EquaresException.h"
#include "equares_exec.h"   // deBUG
#include <QProcess>
#include <QStringList>
#include <QFileInfo>
#include <QRegExp>
#include <QDir>
#include <QSet>

static QString readFile(const QString& fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        throw EquaresException(QObject::tr("readFile(): failed to open file %1").arg(fileName));
    return QString::fromUtf8(file.readAll());
}

void checkLib(const QString& libName)
{
    static QString script;
    static QSet<QString> whiteList;
    static QRegExp rx("^\\w+[^#]*");
    if (script.isEmpty()) {
        // Read soimports command and whitelist
        script = readFile(":/checklib/soimports").split('\n')[0];
        foreach (const QString& s, readFile(":/checklib/soimports-whitelist.txt").split('\n'))
            if (rx.indexIn(s) == 0)
                whiteList << rx.capturedTexts()[0];
    }

    QProcess proc;
    QFileInfo fi(libName);
    QString libFileName = fi.dir().absoluteFilePath("lib" + fi.baseName() + ".so");
    QString cmd = script;
    cmd.replace("$1", libFileName);
    proc.start("sh");
    if (!proc.waitForStarted())
        throw EquaresException(QObject::tr("Failed to start command\n%1").arg(cmd));
    proc.write(cmd.toUtf8());
    proc.closeWriteChannel();
    if (!proc.waitForFinished())
        throw EquaresException(QObject::tr("Command\n%1\ntakes too long to execute").arg(cmd));
    QString stderr = QString::fromUtf8(proc.readAllStandardError());
    if (proc.exitStatus() != QProcess::NormalExit   ||   proc.exitCode() != 0   ||   !stderr.isEmpty())
        throw EquaresException(QObject::tr("Command\n%1\nfailed: code: %2, message: %3")
            .arg(cmd, QString::number(proc.exitCode()), stderr));
    QString stdout = QString::fromUtf8(proc.readAllStandardOutput());
    if (stdout.isEmpty())
        throw EquaresException(QObject::tr("Command\n%1\nreturned nothing").arg(cmd));
    QStringList prohibitedSymbols;
    foreach (const QString& line, stdout.split('\n')) {
        if (line.isEmpty())
            continue;
        if (rx.indexIn(line) != 0)
            throw EquaresException(QObject::tr("Command\n%1\nfailed: unrecognized output").arg(cmd));
        QString s = rx.capturedTexts()[0];
        if (!whiteList.contains(s))
            prohibitedSymbols << s;
    }
    if (!prohibitedSymbols.isEmpty())
        throw EquaresException(QObject::tr("The following imported symbols are not allowed\n%1").arg(prohibitedSymbols.join(", ")));
}
