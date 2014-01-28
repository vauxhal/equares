#ifndef EQUARESPRINTUTIL_H
#define EQUARESPRINTUTIL_H

#include "PrintUtil.h"
#include "equares_core/equares_core.h"
#include <QScriptValueIterator>

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

inline QString formatScriptValue(const QScriptValue& v)
{
    if (v.isArray()) {
        int n = v.property("length").toInt32();
        QString result = "[";
        for (int i=0; i<n; ++i) {
            if (i > 0)
                result += ", ";
            result += formatScriptValue(v.property(i));
        }
        result += "]";
        return result;
    }
    else if (v.isObject()) {
        QScriptValueIterator it(v);
        bool first = true;
        QString result = "{";
        while(it.hasNext()) {
            it.next();
            if (first)
                first = false;
            else
                result += ", ";
            result += it.name() + ": " + formatScriptValue(v.property(it.name()));
        }
        result += "}";
        return result;
    }
    else if (v.isString())
        return escapeString(v.toString());
    else
        return v.toString();
}

struct BoxPropPrinter {
    BoxPropPrinter(const Box *box) : box(box) {}
    void operator()(QTextStream& os, const BoxProperty& boxProp) const {
        os << "{name: '" << boxProp.name
           << "', help: '" << escapeString(boxProp.helpString) << "'"
           << ",\n     userType: '" << escapeString(boxProp.userType) << "'";
        if (!boxProp.userType.isEmpty()) {
            QScriptValue v = box->engine()->newVariant(box->property(boxProp.name.toLatin1()));
            os << ",\n     defaultValue: " << formatScriptValue(v);
        }
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
private:
    const Box *box;
};

#endif // EQUARESPRINTUTIL_H
