#ifndef SCRIPT_ARRAYS_H
#define SCRIPT_ARRAYS_H

// See "Custom Script Class Example" in Qt Assistant

#include <QObject>
#include <QScriptClass>
#include <QScriptString>
#include <QScriptable>
#include <QScriptValue>
#include <QScriptClassPropertyIterator>
#include <QScriptEngine>
#include <QVariant>
#include <QVector>

#include "equares_core_global.h"

struct CustomArrayDoubleTypeTraits
{
    typedef double value_type;
    static const char *baseName() { return "Double"; }
};

struct CustomArrayIntTypeTraits
{
    typedef int value_type;
    static const char *baseName() { return "Int"; }
};



template< class TypeTraits >
class CustomArrayClassPropertyIterator : public QScriptClassPropertyIterator
{
public:
    typedef typename TypeTraits::value_type value_type;
    typedef QVector<value_type> vector_type;

    CustomArrayClassPropertyIterator(const QScriptValue &object) :
        QScriptClassPropertyIterator(object)
    {
        toFront();
    }

    bool hasNext() const {
        vector_type *ba = qscriptvalue_cast<vector_type*>(object().data());
        return m_index < ba->size();
    }

    void next() {
        m_last = m_index;
        ++m_index;
    }

    bool hasPrevious() const {
        return (m_index > 0);
    }

    void previous() {
        --m_index;
        m_last = m_index;
    }

    void toFront() {
        m_index = 0;
        m_last = -1;
    }

    void toBack() {
        vector_type *ba = qscriptvalue_cast<vector_type*>(object().data());
        m_index = ba->size();
        m_last = -1;
    }

    QScriptString name() const {
        return object().engine()->toStringHandle(QString::number(m_last));
    }

    uint id() const {
        return m_last;
    }

private:
    int m_index;
    int m_last;
};

template< class T >
struct CustomArrayPrototypeTraits {};

template< class TypeTraits >
class CustomArrayClass : public QObject, public QScriptClass
{
public:
    typedef typename TypeTraits::value_type value_type;
    typedef QVector<value_type> vector_type;
    typedef CustomArrayPrototypeTraits<value_type> proto_traits;
    typedef CustomArrayClassPropertyIterator<TypeTraits> PropertyIterator;
    typedef CustomArrayClass<TypeTraits> ThisClass;

    explicit CustomArrayClass(QScriptEngine *engine)
        : QObject(engine), QScriptClass(engine)
    {
        qScriptRegisterMetaType<vector_type>(engine, toScriptValue, fromScriptValue);

        length = engine->toStringHandle(QLatin1String("length"));

        proto = engine->newQObject(proto_traits::newArrayPrototype(this),
                                   QScriptEngine::QtOwnership,
                                   QScriptEngine::SkipMethodsInEnumeration
                                   | QScriptEngine::ExcludeSuperClassMethods
                                   | QScriptEngine::ExcludeSuperClassProperties);
        QScriptValue global = engine->globalObject();
        proto.setPrototype(global.property("Object").property("prototype"));

        ctor = engine->newFunction(construct, proto);
        QObject *q = this;
        ctor.setData(engine->toScriptValue(q));
        engine->globalObject().setProperty(name(), ctor);
    }

    QScriptValue constructor() {
        return ctor;
    }

    QScriptValue newInstance(int size = 0) {
        engine()->reportAdditionalMemoryCost(size);
        return newInstance(vector_type(size, /*ch=*/0));
    }

    QScriptValue newInstance(const vector_type &ba) {
        QScriptValue data = engine()->newVariant(QVariant::fromValue(ba));
        return engine()->newObject(this, data);
    }

    QueryFlags queryProperty(const QScriptValue &object,
                             const QScriptString &name,
                             QueryFlags flags, uint *id)
    {
        vector_type *ba = qscriptvalue_cast<vector_type*>(object.data());
        if (!ba)
            return 0;
        if (name == length) {
            return flags;
        } else {
            bool isArrayIndex;
            qint32 pos = name.toArrayIndex(&isArrayIndex);
            if (!isArrayIndex)
                return 0;
            *id = pos;
            if ((flags & HandlesReadAccess) && (pos >= ba->size()))
                flags &= ~HandlesReadAccess;
            return flags;
        }
    }

    QScriptValue property(const QScriptValue &object,
                          const QScriptString &name, uint id)
    {
        vector_type *ba = qscriptvalue_cast<vector_type*>(object.data());
        if (!ba)
            return QScriptValue();
        if (name == length) {
            return ba->size();
        } else {
            qint32 pos = id;
            if ((pos < 0) || (pos >= ba->size()))
                return QScriptValue();
            return ba->at(pos);
        }
        return QScriptValue();
    }

    void setProperty(QScriptValue &object, const QScriptString &name,
                     uint id, const QScriptValue &value)
    {
        vector_type *ba = qscriptvalue_cast<vector_type*>(object.data());
        if (!ba)
            return;
        if (name == length) {
            resize(*ba, value.toInt32());
        } else {
            qint32 pos = id;
            if (pos < 0)
                return;
            if (ba->size() <= pos)
                resize(*ba, pos + 1);
            (*ba)[pos] = qscriptvalue_cast<value_type>(value);
        }
    }

    QScriptValue::PropertyFlags propertyFlags(
        const QScriptValue &object, const QScriptString &name, uint id)
    {
        Q_UNUSED(object);
        Q_UNUSED(id);
        if (name == length) {
            return QScriptValue::Undeletable
                | QScriptValue::SkipInEnumeration;
        }
        return QScriptValue::Undeletable;
    }

    QScriptClassPropertyIterator *newIterator(const QScriptValue &object) {
        return new PropertyIterator(object);
    }

    QString name() const {
        return staticName();
    }

    QScriptValue prototype() const {
        return proto;
    }
private:
    static QString staticName() {
        return QString(TypeTraits::baseName()) + "Array";
    }

    static ThisClass *my_cast(const QScriptValue& s) {
        QObject *o = qscriptvalue_cast<QObject*>(s);
        Q_ASSERT(o);
        ThisClass *result = static_cast<ThisClass*>(o);
        return result;
    }

    static QScriptValue construct(QScriptContext *ctx, QScriptEngine *eng)
    {
        Q_UNUSED(eng);
        ThisClass *cls = my_cast(ctx->callee().data());
        if (!cls)
            return QScriptValue();
        QScriptValue arg = ctx->argument(0);
        if (arg.instanceOf(ctx->callee()))
            return cls->newInstance(qscriptvalue_cast<vector_type>(arg));
        int size = arg.toInt32();
        return cls->newInstance(size);
    }

public:
    static QScriptValue toScriptValue(QScriptEngine *eng, const vector_type &ba)
    {
        QScriptValue ctor = eng->globalObject().property(staticName());
        ThisClass *cls = my_cast(ctor.data());
        if (!cls)
            return eng->newVariant(QVariant::fromValue(ba));
        return cls->newInstance(ba);
    }

private:
    static void fromScriptValue(const QScriptValue &obj, vector_type &ba)
    {
        if (obj.isArray()) {
            int length = obj.property("length").toInt32();
            ba.resize(length);
            for (int i=0; i<length; ++i)
                ba[i] = qscriptvalue_cast<value_type>(obj.property(i));
        }
        else if (obj.isNumber()) {
            ba.resize(1);
            ba[0] = qscriptvalue_cast<value_type>(obj);
        }
        else
            ba = qvariant_cast<vector_type>(obj.data().toVariant());
    }

    void resize(vector_type &ba, int newSize)
    {
        int oldSize = ba.size();
        ba.resize(newSize);
        if (newSize > oldSize)
            engine()->reportAdditionalMemoryCost(newSize - oldSize);
    }

    QScriptString length;
    QScriptValue proto;
    QScriptValue ctor;
};

typedef CustomArrayClass<CustomArrayDoubleTypeTraits> CustomDoubleArrayClass;
typedef CustomArrayClass<CustomArrayIntTypeTraits> CustomIntArrayClass;

template< class TypeTraits >
class CustomArrayPrototype : public QObject, public QScriptable
{
public:
    typedef typename TypeTraits::value_type value_type;
    typedef QVector<value_type> vector_type;

    explicit CustomArrayPrototype(QObject *parent = 0) : QObject(parent) {}

    // The following methods will be slots in derived classes
    bool equals(const vector_type &other) {
        return *thisCustomArray() == other;
    }

    QScriptValue remove(int pos, int len) {
        thisCustomArray()->remove(pos, len);
        return thisObject();
    }

    QScriptValue valueOf() const {
        return thisObject().data();
    }

private:
    vector_type *thisCustomArray() const {
        return qscriptvalue_cast<vector_type*>(thisObject().data());
    }
};

class EQUARES_CORESHARED_EXPORT CustomDoubleArrayPrototype : public CustomArrayPrototype<CustomArrayDoubleTypeTraits> {
    Q_OBJECT
public:
    typedef CustomArrayPrototype<CustomArrayDoubleTypeTraits> BaseClass;
    explicit CustomDoubleArrayPrototype(QObject *parent = 0) : BaseClass(parent) {}

public slots:
    bool equals(const vector_type &other) { return BaseClass::equals(other); }
    QScriptValue remove(int pos, int len) { return BaseClass::remove(pos, len); }
    QScriptValue valueOf() const { return BaseClass::valueOf(); }
};

class EQUARES_CORESHARED_EXPORT CustomIntArrayPrototype : public CustomArrayPrototype<CustomArrayIntTypeTraits> {
    Q_OBJECT
public:
    typedef CustomArrayPrototype<CustomArrayIntTypeTraits> BaseClass;
    explicit CustomIntArrayPrototype(QObject *parent = 0) : BaseClass(parent) {}

public slots:
    bool equals(const vector_type &other) { return BaseClass::equals(other); }
    QScriptValue remove(int pos, int len) { return BaseClass::remove(pos, len); }
    QScriptValue valueOf() const { return BaseClass::valueOf(); }
};

template<>
struct CustomArrayPrototypeTraits<double> {
    static CustomDoubleArrayPrototype *newArrayPrototype(CustomDoubleArrayClass *c) {
        return new CustomDoubleArrayPrototype(c);
    }
};

template<>
struct CustomArrayPrototypeTraits<int> {
    static CustomIntArrayPrototype *newArrayPrototype(CustomIntArrayClass *c) {
        return new CustomIntArrayPrototype(c);
    }
};

Q_DECLARE_METATYPE(QVector<double>*)
Q_DECLARE_METATYPE(QVector<int>*)
Q_DECLARE_METATYPE(QVector<double>)
Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(CustomDoubleArrayClass*)
Q_DECLARE_METATYPE(CustomIntArrayClass*)

#endif // SCRIPT_ARRAYS_H
