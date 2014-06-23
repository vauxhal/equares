#ifndef FDECXXBOX_H
#define FDECXXBOX_H

#include "equares_core.h"
#include "CxxBuildHelper.h"
#include "box_util.h"
#include <QLibrary>

class EQUARES_CORESHARED_EXPORT FdeCxxBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QString src READ src WRITE setSrc)
    Q_PROPERTY(QString srcExample READ srcExample)
    Q_PROPERTY(bool useQmake READ useQmake WRITE setUseQmake)
public:
    explicit FdeCxxBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

    QString src() const;
    FdeCxxBox& setSrc(const QString& src);

    QString srcExample() const;

    bool useQmake() const;
    FdeCxxBox& setUseQmake(bool useQmake);

    int paramCount() const;
    QStringList paramNames() const;
    int varCount() const;
    QStringList varNames() const;

    using Box::engine;

    class FdeLibProxy {
    public:
        FdeLibProxy() {}
        explicit FdeLibProxy(const QLibraryPtr &lib, const Box *box);
        ~FdeLibProxy();

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
            m_nextState(m_inst, out, param, state);
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
        typedef void (*nextStateFunc)(void*, double*, const double*, const double*);
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
        nextStateFunc m_nextState;
        hashFunc m_hash;

        void *m_inst;
    };
    const FdeLibProxy *fdeLibProxy() const;

public slots:
    QString buildDir(const QScriptValue& boxProps) const;

private:
    mutable InputPort m_param;
    mutable InputPort m_state;
    mutable OutputPort m_nextState;
    bool m_useQmake;
    QString m_src;

    FdeLibProxy m_libProxy;
};

class FdeCxxRuntimeBox : public RuntimeBox
{
public:
    explicit FdeCxxRuntimeBox(const FdeCxxBox *box);

private:
    RuntimeInputPort m_param;
    RuntimeInputPort m_state;
    RuntimeOutputPort m_nextState;

    const FdeCxxBox::FdeLibProxy *m_fdeLibProxy;

    bool m_hasParamData;
    QVector<double> m_paramData;
    QVector<double> m_nextStateData;

    bool setParameters(int);
    bool setState(int);
};

#endif // FDECXXBOX_H
