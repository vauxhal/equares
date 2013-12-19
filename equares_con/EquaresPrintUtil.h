#ifndef EQUARESPRINTUTIL_H
#define EQUARESPRINTUTIL_H

#include "PrintUtil.h"
#include "equares_core/equares_core.h"

struct NamePrinter {
    void operator()(std::ostream& os, const Named *x) const {
        os << x->name();
    }
    void operator()(std::ostream& os, const Named& x) const {
        os << x.name();
    }
};

struct PortPrinter {
    void operator()(std::ostream& os, const Port *port) const {
        os << "{name: '" << port->name() + "'";
        if (port->format().isFixed()) {
            os << ", format: [";
            printContainer(os, port->format().size(), SimplePrinter<int>(), ", ");
            os << "]";
        }
        if (!port->helpString().isEmpty())
            os << ", help: '" << escapeString(port->helpString()) << "'";
        if (port->entryHints().hasHints()) {
            os << ", hints: [";
            printContainer(os, port->entryHints().hints(), SimplePrinter<QString>(), ", ");
            os << "]";
        }
        os << "}";
    }
};

struct BoxPropPrinter {
    void operator()(std::ostream& os, const BoxProperty& boxProp) const {
        os << "{name: '" << boxProp.name
           << "', help: '" << escapeString(boxProp.helpString) << "'}";
    }
};

#endif // EQUARESPRINTUTIL_H
