#include "box_util.h"

bool propagateCommonFormat(Port& port1, Port& port2)
{
    if (port1.format().isValid() == port2.format().isValid())
        return false;
    if (port1.format().isValid())
        port2.format() = port1.format();
    else
        port1.format() = port2.format();
    return true;
}

bool propagateCommonFormat(const QList<Port*>& ports)
{
    // Obtain the only valid format; determine if there are ports with an invalid format
    PortFormat f;
    bool allValid = true;
    foreach (Port *port, ports) {
        PortFormat fp = port->format();
        if (fp.isValid()) {
            if (f.isValid()) {
                if (f != fp)
                    // Incompatible formats, give up
                    return false;
            }
            else
                f = fp;
        }
        else
            allValid = false;
    }

    if (!f.isValid() || allValid)
        // Nothing to do
        return false;

    // Propagate the valid format to all ports
    foreach (Port *port, ports)
        if (!port->format().isValid())
            port->format() = f;
    return true;
}

bool specifyFormat(PortFormat& dst, const PortFormat& src) {
    if (!src.isValid())
        return false;
    if (dst.isValid()   &&   dst == src)
        return false;
    dst = src;
    return true;
}

QString readFile(const QString& fileName, const Box *box)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        throw EquaresBoxException(box, QString("readFile(): failed to open file %1").arg(fileName));
    return QString::fromUtf8(file.readAll());
}

QStringList toNameList(const char *s)
{
    QStringList result;
    if (!s)
        return result;
    QString qs = QString::fromUtf8(s);
    if (qs.isEmpty())
        return result;
    result = qs.split(",", QString::KeepEmptyParts);
    for (int i=0; i<result.size(); ++i)
        result[i] = result[i].trimmed();
    return result;
}

