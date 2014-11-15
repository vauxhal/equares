/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef CXXTRANSFORMBOX_H
#define CXXTRANSFORMBOX_H

#include "equares_core.h"
#include "CxxBuildHelper.h"
#include "box_util.h"
#include <QLibrary>

class EQUARES_CORESHARED_EXPORT CxxTransformBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QString src READ src WRITE setSrc)
    Q_PROPERTY(QString srcExample READ srcExample)
    Q_PROPERTY(bool useQmake READ useQmake WRITE setUseQmake)
public:
    explicit CxxTransformBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

    QString src() const;
    CxxTransformBox& setSrc(const QString& src);

    QString srcExample() const;

    bool useQmake() const;
    CxxTransformBox& setUseQmake(bool useQmake);

    PortFormat paramFormat() const;
    QStringList paramNames() const;
    PortFormat inputFormat() const;
    QStringList inputNames() const;
    PortFormat outputFormat() const;
    QStringList outputNames() const;

    using Box::engine;

    class TransformLibProxy {
    public:
        TransformLibProxy() {}
        explicit TransformLibProxy(const QLibraryPtr &lib, const Box *box);
        TransformLibProxy(const TransformLibProxy& arg);
        TransformLibProxy& operator=(const TransformLibProxy& arg);
        ~TransformLibProxy();

        PortFormat paramFormat() const {
            return portFormat(m_paramDimension, m_paramSizes);
        }
        QStringList paramNames() const {
            return toNameList(m_paramNames(m_inst));
        }
        PortFormat inputFormat() const {
            return portFormat(m_inputDimension, m_inputSizes);
        }
        const int *inputSizes() const {
            return m_inputSizes(m_inst);
        }
        QStringList inputNames() const {
            return toNameList(m_inputNames(m_inst));
        }
        PortFormat outputFormat() const {
            return portFormat(m_outputDimension, m_outputSizes);
        }
        QStringList outputNames() const {
            return toNameList(m_outputNames(m_inst));
        }
        void prepare(const double *param) const {
            m_prepare(m_inst, param);
        }
        void transform(double *out, const double *param, const double *in) const {
            m_transform(m_inst, out, param, in);
        }
        QString hash() const {
            return m_hash();
        }
        bool isNull() const {
            return m_lib.isNull();
        }

    private:
        typedef int (*countFunc)(void*);
        typedef const int* (*sizesFunc)(void*);
        typedef const char* (*namesFunc)(void*);

        typedef void* (*newInstanceFunc)();
        typedef void (*deleteInstanceFunc)(void*);

        typedef countFunc paramDimensionFunc;
        typedef sizesFunc paramSizesFunc;
        typedef namesFunc paramNamesFunc;

        typedef countFunc inputDimensionFunc;
        typedef sizesFunc inputSizesFunc;
        typedef namesFunc inputNamesFunc;

        typedef countFunc outputDimensionFunc;
        typedef sizesFunc outputSizesFunc;
        typedef namesFunc outputNamesFunc;

        typedef void (*prepareFunc)(void*, const double*);
        typedef void (*transformFunc)(void*, double*, const double*, const double*);
        typedef const char* (*hashFunc)();

        QLibraryPtr m_lib;
        const Box *m_box;

        newInstanceFunc m_newInstance;
        deleteInstanceFunc m_deleteInstance;

        paramDimensionFunc m_paramDimension;
        paramSizesFunc m_paramSizes;
        paramNamesFunc m_paramNames;

        inputDimensionFunc m_inputDimension;
        inputSizesFunc m_inputSizes;
        inputNamesFunc m_inputNames;

        outputDimensionFunc m_outputDimension;
        outputSizesFunc m_outputSizes;
        outputNamesFunc m_outputNames;

        prepareFunc m_prepare;
        transformFunc m_transform;
        hashFunc m_hash;

        void *m_inst;
        void init();
        PortFormat portFormat(countFunc dim, sizesFunc sizes) const {
            QVector<int> szv( dim(m_inst) );
            const int *szp = sizes(m_inst);
            std::copy(szp, szp+szv.size(), szv.begin());
            return PortFormat(szv).setFixed();
        }
    };
    const TransformLibProxy *transformLibProxy() const;

public slots:
    QString buildDir(const QScriptValue& boxProps) const;

private:
    mutable InputPort m_param;
    mutable InputPort m_input;
    mutable OutputPort m_output;
    bool m_useQmake;
    QString m_src;

    TransformLibProxy m_libProxy;
};

class CxxTransformRuntimeBox : public RuntimeBox
{
public:
    explicit CxxTransformRuntimeBox(const CxxTransformBox *box);

private:
    RuntimeInputPort m_param;
    RuntimeInputPort m_input;
    RuntimeOutputPort m_output;

    const CxxTransformBox::TransformLibProxy *m_libProxy;

    bool m_hasParamData;
    QVector<double> m_paramData;
    QVector<double> m_outputData;

    bool setParameters(int);
    bool setInput(int);
};

#endif // CXXTRANSFORMBOX_H
