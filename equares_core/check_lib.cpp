// check_lib.cpp

#include "check_lib.h"
#include "EquaresException.h"
#include "equares_exec.h"   // deBUG
#include <QProcess>
#include <QStringList>
#include <QFileInfo>
#include <QRegExp>
#include <QDir>

void checkLib(const QString& libName)
{
    //return;// deBUG
    QProcess proc;
    QFileInfo fi(libName);
    QString libFileName = fi.dir().absoluteFilePath("lib" + fi.baseName() + ".so");
    QString cmd = (QStringList() << "nm" << "-gC" << libFileName).join(" ");
    proc.start(cmd);
    if (!proc.waitForStarted())
        throw EquaresException(QObject::tr("Failed to start command\n%1").arg(cmd));
    if (!proc.waitForFinished())
        throw EquaresException(QObject::tr("Command\n%1\ntakes too long to execute").arg(cmd));
    QString stderr = QString::fromUtf8(proc.readAllStandardError());
    if (proc.exitStatus() != QProcess::NormalExit   ||   proc.exitCode() != 0   ||   !stderr.isEmpty())
        throw EquaresException(QObject::tr("Command\n%1\nfailed")//: code: %2, message: %3")
            .arg(cmd, QString::number(proc.exitCode()), stderr));
    QString stdout = QString::fromUtf8(proc.readAllStandardOutput());
    if (stdout.isEmpty())
        throw EquaresException(QObject::tr("Command\n%1\nreturned nothing").arg(cmd));
    QRegExp rx("^([0-9a-z]+)?\\s+(\\w)\\s+([^@]+)(@@([.\\w]+))?$");
    foreach (const QString& line, stdout.split('\n')) {
        if (line.isEmpty())
            continue;
        if (rx.indexIn(line) != 0)
            throw EquaresException(QObject::tr("Command\n%1\nfailed: unrecognized output").arg(cmd));
        QStringList matches = rx.capturedTexts();
        Q_ASSERT(matches.length() == 6);
        QString type = matches[2], name = matches[3];
        // EQUARES_COUT << "========= [" << type << "][" << name << "]" << endl;
        // TODO
    }
}
