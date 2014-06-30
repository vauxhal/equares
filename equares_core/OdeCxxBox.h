#ifndef ODECXXBOX_H
#define ODECXXBOX_H

#include "equares_core.h"
#include "CxxBuildHelper.h"
#include "box_util.h"
#include <QLibrary>

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
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

    QString src() const;
    OdeCxxBox& setSrc(const QString& src);

    QString srcExample() const;

    bool useQmake() const;
    OdeCxxBox& setUseQmake(bool useQmake);

    int paramCount() const;
    QStringList paramNames() const;
    int varCount() const;
    QStringList varNames() const;

    using Box::engine;

    class OdeLibProxy {
    public:
        OdeLibProxy() {}
        explicit OdeLibProxy(const QLibraryPtr &lib, const Box *box);
        OdeLibProxy(const OdeLibProxy& arg);
        OdeLibProxy& operator=(const OdeLibProxy& arg);
        ~OdeLibProxy();

        int paramCount() const {
            return m_paramCount(m_inst);
        }
        QStringList paramNames() const {
            return toNameList(m_paramNames(m_inst));
        }
        int varCount() const {
            return m_varCount(m_inst);
        }
        QStringList varNames() const {
            return toNameList(m_varNames(m_inst));
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
        bool isNull() const {
            return m_lib.isNull();
        }

    private:
        typedef void* (*newInstanceFunc)();
        typedef void (*deleteInstanceFunc)(void*);
        typedef int (*paramCountFunc)(void*);
        typedef const char* (*paramNamesFunc)(void*);
        typedef int (*varCountFunc)(void*);
        typedef const char* (*varNamesFunc)(void*);
        typedef void (*prepareFunc)(void*, const double*);
        typedef void (*rhsFunc)(void*, double*, const double*, const double*);
        typedef const char* (*hashFunc)();

        QLibraryPtr m_lib;
        const Box *m_box;

        newInstanceFunc m_newInstance;
        deleteInstanceFunc m_deleteInstance;
        paramCountFunc m_paramCount;
        paramNamesFunc m_paramNames;
        varCountFunc m_varCount;
        varNamesFunc m_varNames;
        prepareFunc m_prepare;
        rhsFunc m_rhs;
        hashFunc m_hash;

        void *m_inst;
        void init();
    };
    const OdeLibProxy *odeLibProxy() const;

public slots:
    QString buildDir(const QScriptValue& boxProps) const;

private:
    mutable InputPort m_param;
    mutable InputPort m_state;
    mutable OutputPort m_rhs;
    bool m_useQmake;
    QString m_src;

    OdeLibProxy m_libProxy;
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

    bool setParameters(int);
    bool setState(int);
};

#endif // ODECXXBOX_H
