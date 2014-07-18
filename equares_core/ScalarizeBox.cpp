/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "ScalarizeBox.h"
#include "box_util.h"
#include "equares_script.h"
#include <cmath>

REGISTER_BOX(ScalarizeBox, "Scalarize")

static NameMapper<ScalarizeBoxParam>::Item scalarizeBoxParamNames[] = {
    {ScalarizeNorm1, "norm_1"},
    {ScalarizeNorm2, "norm_2"},
    {ScalarizeNormInf, "norm_inf"},
    {ScalarizeMin, "min"},
    {ScalarizeMax, "max"},
    {static_cast<ScalarizeBoxParam>(-1), 0 }
};

static NameMapper<ScalarizeBoxParam> scalarizeBoxParamNameMap(scalarizeBoxParamNames);

template<class T> QScriptValue toScriptValue(QScriptEngine *e, const T& p);
template<class T> void fromScriptValue(const QScriptValue& v, T& result);

template<>
QScriptValue toScriptValue(QScriptEngine *e, const ScalarizeBoxParam& p) {
    Q_UNUSED(e);
    return scalarizeBoxParamNameMap.toName(p);
}

template<>
void fromScriptValue(const QScriptValue& v, ScalarizeBoxParam& result) {
    result = scalarizeBoxParamNameMap.fromName(v.toString());
}

static void scriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, toScriptValue<ScalarizeBoxParam>, fromScriptValue<ScalarizeBoxParam>);
}

REGISTER_SCRIPT_INIT_FUNC(scriptInit)



ScalarizeBox::ScalarizeBox(QObject *parent) :
    Box(parent),
    m_param(ScalarizeNorm2),
    m_in("input", this),
    m_out("output", this, PortFormat(1).setFixed())
{
}

InputPorts ScalarizeBox::inputPorts() const {
    return InputPorts() << &m_in;
}

OutputPorts ScalarizeBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void ScalarizeBox::checkPortFormat() const {
    if (m_in.format().dataSize() < 1)
        throwBoxException("ScalarizeBox: Invalid input port format (data size must be positive)");
    if (m_out.format() != PortFormat(1))
        throwBoxException("ScalarizeBox: Invalid output port format (must be scalar)");
}

bool ScalarizeBox::propagatePortFormat() {
    return false;
}

RuntimeBox *ScalarizeBox::newRuntimeBox() const {
    return new ScalarizeRuntimeBox(this);
}

ScalarizeBox::Param ScalarizeBox::param() const {
    return m_param;
}

ScalarizeBox& ScalarizeBox::setParam(const Param& param) {
    m_param = param;
    return *this;
}



ScalarizeRuntimeBox::ScalarizeRuntimeBox(const ScalarizeBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(&ScalarizeRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_in);

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0]);
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_n = in[0]->format().dataSize();
    Q_ASSERT(m_n > 0);

    switch(box->param()) {
    case ScalarizeNorm1:   m_scalarize = &ScalarizeRuntimeBox::scalarizeNorm1;   break;
    case ScalarizeNorm2:   m_scalarize = &ScalarizeRuntimeBox::scalarizeNorm2;   break;
    case ScalarizeNormInf:   m_scalarize = &ScalarizeRuntimeBox::scalarizeNormInf;   break;
    case ScalarizeMin:   m_scalarize = &ScalarizeRuntimeBox::scalarizeNormMin;   break;
    case ScalarizeMax:   m_scalarize = &ScalarizeRuntimeBox::scalarizeNormMax;   break;
    default:
        throwBoxException("Invalid scalarizer parameter");
    }
}

bool ScalarizeRuntimeBox::processInput(int)
{
    Q_ASSERT(m_in.state().hasData());
    m_outData = (this->*m_scalarize)(m_in.data().data());
    m_out.setData(PortData(1, &m_outData));
    m_out.state().setValid();
    return m_out.activateLinks();
}

double ScalarizeRuntimeBox::scalarizeNorm1(const double *src) const {
    double result = 0;
    for (int i=0; i<m_n; ++i)
        result += fabs(src[i]);
    return result;
}

static inline double sqr(double x) { return x*x; }

double ScalarizeRuntimeBox::scalarizeNorm2(const double *src) const {
    double result = 0;
    for (int i=0; i<m_n; ++i)
        result += sqr(src[i]);
    return sqrt(result);
}

double ScalarizeRuntimeBox::scalarizeNormInf(const double *src) const {
    double result = fabs(src[0]);
    for (int i=1; i<m_n; ++i) {
        double x = fabs(src[i]);
        if (result < x)
            result = x;
    }
    return result;
}

double ScalarizeRuntimeBox::scalarizeNormMin(const double *src) const {
    double result = src[0];
    for (int i=1; i<m_n; ++i) {
        double x = src[i];
        if (result > x)
            result = x;
    }
    return result;
}

double ScalarizeRuntimeBox::scalarizeNormMax(const double *src) const {
    double result = src[0];
    for (int i=1; i<m_n; ++i) {
        double x = src[i];
        if (result < x)
            result = x;
    }
    return result;
}
