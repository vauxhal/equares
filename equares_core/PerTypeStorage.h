#ifndef PER_TYPE_STORAGE_H
#define PER_TYPE_STORAGE_H

#include "equares_core_global.h"
#include <QMap>
#include <QLatin1String>
#include <typeinfo>

class EQUARES_CORESHARED_EXPORT PerTypeStorage
{
public:
    template<class X> static X& instance() {
        QLatin1String type = QLatin1String(typeid(X).name());
        InstanceMap& im =  instances();
        InstanceMap::const_iterator it = m_instances->constFind(type);
        if (it == im.constEnd()) {
            X *result = new X();
            im[type] = result;
            return *result;
        }
        else
            return *reinterpret_cast<X*>(it.value());
    }

private:
    typedef QMap<QLatin1String, void*> InstanceMap;
    static InstanceMap *m_instances;

    static InstanceMap& instances() {
        if (!m_instances)
            m_instances = new InstanceMap();
        return *m_instances;
    }
};

#endif // PER_TYPE_STORAGE_H
