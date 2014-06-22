#include "box_util.h"

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
        return true;

    // Propagate the valid format to all ports
    foreach (Port *port, ports)
        if (!port->format().isValid())
            port->format() = f;
    return true;
}

