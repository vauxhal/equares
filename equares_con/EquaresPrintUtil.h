#ifndef EQUARESPRINTUTIL_H
#define EQUARESPRINTUTIL_H

#include "PrintUtil.h"
#include "equares_core/equares_core.h"

struct NamePrinter {
    void operator()(QTextStream& os, const Named *x) const {
        os << x->name();
    }
    void operator()(QTextStream& os, const Named& x) const {
        os << x.name();
    }
};

struct PortPrinter {
    void operator()(QTextStream& os, const Port *port) const {
        os << "{name: '" << port->name() + "'";
        if (port->format().isFixed()) {
            os << ", format: [";
            printContainer(os, port->format().size(), SimplePrinter<int>(), ", ");
            os << "]";
        }
        if (!port->helpString().isEmpty())
            os << ", help: '" << escapeString(port->helpString()) << "'";
        if (port->hints().hasEntryHints()) {
            os << ",\n     hints: [";
            printContainer(os, port->hints().entryHints(), SimplePrinter<QString>(), ", ");
            os << "]";
        }
        if (port->hints().hasPosition())
            os << ", pos: " << port->hints().position();
        os << "}";
    }
};

struct BoxPropPrinter {
    void operator()(QTextStream& os, const BoxProperty& boxProp) const {
        os << "{name: '" << boxProp.name
           << "', help: '" << escapeString(boxProp.helpString) << "'"
           << ",\n     userType: '" << escapeString(boxProp.userType) << "'";
        if (!boxProp.toUserType.isEmpty())
            os << ",\n     toUserType: '" << escapeString(boxProp.toUserType) << "'";
        if (!boxProp.toBoxType.isEmpty())
            os << ",\n     toBoxType: '" << escapeString(boxProp.toBoxType) << "'";
        if (!boxProp.deps.isEmpty()) {
            os << ",\n     deps: [";
            printContainer(os, boxProp.deps, SimplePrinter<QString>(), ", ");
            os << "]";
        if (!boxProp.resolveUserType.isEmpty())
            os << ",\n     resolveUserType: '" << escapeString(boxProp.resolveUserType) << "'";
        }
        os << "}";
    }
};

#endif // EQUARESPRINTUTIL_H
