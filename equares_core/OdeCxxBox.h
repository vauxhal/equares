#ifndef ODECXXBOX_H
#define ODECXXBOX_H

#include "equares_core.h"
#include <QLibrary>
#include <QSharedPointer>

class EQUARES_CORESHARED_EXPORT OdeCxxBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QString src READ src WRITE setSrc)
    Q_PROPERTY(QString srcExample READ srcExample)
    Q_PROPERTY(bool useQmake READ useQmake WRITE setUseQmake)
public:
    explicit OdeCxxBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    BoxPropertyList boxProperties() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

    QString src() const;
    OdeCxxBox& setSrc(const QString& src);

    QString srcExample() const;

    bool useQmake() const;
    OdeCxxBox& setUseQmake(bool useQmake);

    int paramCount() const;
    int varCount() const;

    using Box::engine;

    class OdeLibProxy {
    public:
        typedef QSharedPointer<OdeLibProxy> Ptr;

        explicit OdeLibProxy(const QString& libName);
        ~OdeLibProxy();

        int paramCount() const {
            return m_paramCount(m_inst);
        }
        int varCount() const {
            return m_varCount(m_inst);
        }
        void prepare(const double *param) const {
            m_prepare(m_inst, param);
        }
        void rhs(double *out, const double *param, const double *state) const {
            m_rhs(m_inst, out, param, state);
        }
        QString hash() const {
            return m_hash();
        }
    private:
        OdeLibProxy(const OdeLibProxy&);
        OdeLibProxy& operator=(const OdeLibProxy&);

        typedef void* (*newInstanceFunc)();
        typedef void (*deleteInstanceFunc)(void*);
        typedef int (*paramCountFunc)(void*);
        typedef int (*varCountFunc)(void*);
        typedef void (*prepareFunc)(void*, const double*);
        typedef void (*rhsFunc)(void*, double*, const double*, const double*);
        typedef const char* (*hashFunc)();

        QLibrary m_lib;
        newInstanceFunc m_newInstance;
        deleteInstanceFunc m_deleteInstance;
        paramCountFunc m_paramCount;
        varCountFunc m_varCount;
        prepareFunc m_prepare;
        rhsFunc m_rhs;
        hashFunc m_hash;

        void *m_inst;
    };
    const OdeLibProxy *odeLibProxy() const;

private:
    mutable InputPort m_param;
    mutable InputPort m_state;
    mutable OutputPort m_rhs;
    bool m_useQmake;
    QString m_src;

    OdeLibProxy::Ptr m_libProxy;

    static void checkSrc(const QString& src);
    static bool libUpToDate(const QString& libName, const QString& hashString);
};

class OdeCxxRuntimeBox : public RuntimeBox
{
public:
    explicit OdeCxxRuntimeBox(const OdeCxxBox *box);

private:
    RuntimeInputPort m_param;
    RuntimeInputPort m_state;
    RuntimeOutputPort m_rhs;

    const OdeCxxBox::OdeLibProxy *m_odeLibProxy;

    bool m_hasParamData;
    QVector<double> m_paramData;
    QVector<double> m_rhsData;

    bool setParameters();
    bool setState();
};

#endif // ODECXXBOX_H
