#ifndef EQUARES_SCRIPT_H
#define EQUARES_SCRIPT_H

#include "equares_core.h"
#include "script_arrays.h"

#include <QScriptContext>

#include <iostream>

Q_DECLARE_METATYPE(QList<double>)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(Port*)
Q_DECLARE_METATYPE(Box::Ctor)
Q_DECLARE_METATYPE(LinkList)

template< class T >
inline QScriptValue listToScriptValue(QScriptEngine *e, const QList<T>& v) {
    QScriptValue result = e->newArray(v.size());
    for (int i=0; i<v.size(); ++i)
        result.setProperty(i, v[i]);
    return result;
}

template< class T >
inline void listFromScriptValue(const QScriptValue& v, QList<T>& result) {
    result.clear();
    if (!v.isArray())
        // TODO: throw error
        return;
    int length = v.property("length").toInt32();
    for (int i=0; i<length; ++i)
        result << qscriptvalue_cast<T>(v.property(i));
}

inline QScriptValue portToScriptValue(QScriptEngine *e, Port* const& p) {
    return e->newVariant(QVariant::fromValue(p));
}

inline void portFromScriptValue(const QScriptValue& v, Port*& result) {
    result = qvariant_cast<Port*>(v.toVariant());
}

inline QScriptValue linksToScriptValue(QScriptEngine *e, const LinkList& links) {
    QScriptValue result = e->newArray(links.size());
    for (int i=0; i<links.size(); ++i) {
        const Link& link = links[i];
        QScriptValue s = e->newArray(2);
        s.setProperty(0, portToScriptValue(e, link.inputPort()));
        s.setProperty(1, portToScriptValue(e, link.outputPort()));
        result.setProperty(i, s);
    }
    return result;
}

inline void linksFromScriptValue(const QScriptValue& v, LinkList& result) {
    result.clear();
    if (!v.isArray())
        // TODO: throw error
        return;
    int n = v.property("length").toInt32();
    for (int i=0; i<n; ++i) {
        QScriptValue s = v.property(i);
        if (!s.isArray()   ||   s.property("length").toInt32() != 2) {
            // TODO: throw error
            result.clear();
            return;
        }
        Port *p1 = 0,   *p2 = 0;
        portFromScriptValue(s.property(0), p1);
        portFromScriptValue(s.property(1), p2);
        if (!p1 || !p2) {
            // TODO: throw error
            result.clear();
            return;
        }
        InputPort *in = 0;
        OutputPort *out = 0;
        for (int j=0; j<2; ++j, qSwap(p1,p2)) {
            in = dynamic_cast<InputPort*>(p1);
            if (!in)
                continue;
            out = dynamic_cast<OutputPort*>(p2);
            if (out)
                break;
        }
        if (!(in && out)) {
            // TODO: throw error
            result.clear();
            return;
        }
        result << Link(out, in);
    }
}



template< class PortType >
inline void addPortProperties(QScriptValue& result, const QList<PortType>& ports) {
    foreach(PortType port, ports) {
        Port *p = port;
        result.setProperty(p->name(), portToScriptValue(result.engine(), p),
            QScriptValue::ReadOnly | QScriptValue::Undeletable);
    }
}

inline Box *newBox(const QString& name)
{
    Box::Ctor ctor = BoxFactory::boxCtor(name);
    Q_ASSERT(ctor);
    Box *box = ctor();
    box->setType(name);
    QSettings settings(":/box/settings.ini", QSettings::IniFormat);
    settings.beginGroup(name);
    box->loadSettings(settings);
    return box;
}

inline QScriptValue newBox(QScriptContext *context, QScriptEngine *engine) {
    QString name = context->callee().data().toVariant().toString();
    Box *box = newBox(name);
    QScriptValue result = engine->newQObject(box);
    addPortProperties(result, box->inputPorts());
    addPortProperties(result, box->outputPorts());
    return result;
    }

inline QScriptValue boxTypes(QScriptContext *context, QScriptEngine *engine) {
    Q_UNUSED(context);
    Q_UNUSED(engine);
    foreach (const QString& name, BoxFactory::boxTypes())
        EQUARES_COUT << name << endl;
    return QScriptValue();
}

inline QScriptValue newSimulation(QScriptContext *context, QScriptEngine *engine) {
    Q_UNUSED(context);
    Q_UNUSED(engine);
    return engine->newQObject(new Simulation());
}

class EQUARES_CORESHARED_EXPORT ScriptCustomInit
{
public:
    typedef void (*ScriptInitFunc)(QScriptEngine *engine);
    static void init(QScriptEngine *e);
    static void registerScriptInitFunc(ScriptInitFunc func);
private:
    typedef QList< ScriptInitFunc > ScriptInitFuncList;
    static ScriptInitFuncList *m_initializers;
    static ScriptInitFuncList& initializers();
};

class ScriptCustomInitFuncRegistrator {
public:
    ScriptCustomInitFuncRegistrator(ScriptCustomInit::ScriptInitFunc func) {
        ScriptCustomInit::registerScriptInitFunc(func);
    }
};
#ifdef EQUARES_CORE_STATIC
#define REGISTER_SCRIPT_INIT_FUNC(func)
#define REGISTER_SCRIPT_INIT_FUNC_STATIC(func) static ScriptCustomInitFuncRegistrator scriptCustomInitFuncRegistratorFor_##func(func);
#else // EQUARES_CORE_STATIC
#define REGISTER_SCRIPT_INIT_FUNC(func) static ScriptCustomInitFuncRegistrator scriptCustomInitFuncRegistratorFor_##func(func);
#define REGISTER_SCRIPT_INIT_FUNC_STATIC(func)
#endif // EQUARES_CORE_STATIC


inline void registerEquaresScriptTypes(QScriptEngine *engine)
{
    new CustomDoubleArrayClass(engine);
    new CustomIntArrayClass(engine);
    qScriptRegisterMetaType(engine, listToScriptValue<double>, listFromScriptValue<double>);
    qScriptRegisterMetaType(engine, listToScriptValue<int>, listFromScriptValue<int>);
    qScriptRegisterMetaType(engine, portToScriptValue, portFromScriptValue);
    qScriptRegisterMetaType(engine, linksToScriptValue, linksFromScriptValue);
    foreach (const QString& name, BoxFactory::boxTypes()) {
        QScriptValue fun = engine->newFunction(newBox);
        fun.setData(name);
        engine->globalObject().setProperty(name, fun);
    }
    engine->globalObject().setProperty("boxTypes", engine->newFunction(boxTypes));
    engine->globalObject().setProperty("Simulation", engine->newFunction(newSimulation));
    ScriptCustomInit::init(engine);
}

#endif // EQUARES_SCRIPT_H
