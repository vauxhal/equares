/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef ODEJSBOX_H
#define ODEJSBOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT OdeJsBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QScriptValue ode READ ode WRITE setOde)
public:
    explicit OdeJsBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

    QScriptValue ode() const;
    OdeJsBox& setOde(const QScriptValue& ode);

    int paramCount() const;
    int varCount() const;

    using Box::engine;

private:
    mutable InputPort m_param;
    mutable InputPort m_state;
    mutable OutputPort m_rhs;
    QScriptValue m_ode;
};

class OdeJsRuntimeBox : public RuntimeBox
{
public:
    explicit OdeJsRuntimeBox(const OdeJsBox *box);

private:
    RuntimeInputPort m_param;
    RuntimeInputPort m_state;
    RuntimeOutputPort m_rhs;
    bool m_hasParamData;
    QVector<double> m_paramData;
    QVector<double> m_stateData;
    QVector<double> m_rhsData;

    QScriptEngine *m_engine;
    bool m_hasPrepareFunc;
    QScriptValue m_ode;
    QScriptValue m_prepareFunc;
    QScriptValue m_argsForPrepare;
    QScriptValue m_rhsFunc;
    QScriptValue m_argsForRhs;

    bool setParameters(int);
    bool setState(int);
};

#endif // ODEJSBOX_H
