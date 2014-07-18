/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "SimpleInputBox.h"

REGISTER_BOX(SimpleInputBox, "SimpleInput")

template<class T> QScriptValue toScriptValue(QScriptEngine *e, const T& p);
template<class T> void fromScriptValue(const QScriptValue& v, T& result);

template<>
QScriptValue toScriptValue(QScriptEngine *e, const SimpleInputBoxParamItem& p) {
    QScriptValue result = e->newObject();
    result.setProperty("index", p.index);
    result.setProperty("name", p.name);
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, SimpleInputBoxParamItem& result) {
    result = SimpleInputBoxParamItem();
    result.index = v.property("index").toInt32();
    result.name = v.property("name").toString();
}

template<>
QScriptValue toScriptValue(QScriptEngine *e, const SimpleInputBoxParam& p) {
    int n = p.size();
    QScriptValue result = e->newArray(n);
    for (int i=0; i<n; ++i)
        result.setProperty(i, toScriptValue(e, p[i]));
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, SimpleInputBoxParam& result) {
    int n = v.property("length").toInt32();
    result = SimpleInputBoxParam(n);
    for (int i=0; i<n; ++i)
        fromScriptValue(v.property(i), result[i]);
}

static void scriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, toScriptValue<SimpleInputBoxParamItem>, fromScriptValue<SimpleInputBoxParamItem>);
    qScriptRegisterMetaType(e, toScriptValue<SimpleInputBoxParam>, fromScriptValue<SimpleInputBoxParam>);
}

REGISTER_SCRIPT_INIT_FUNC(scriptInit)



SimpleInputBox::SimpleInputBox(QObject *parent) : DataInputBox(parent)
{
}

void SimpleInputBox::checkPortFormat() const
{
    int inputSize = inputPorts()[1]->format().size(0);
    foreach (const ParamItem& paramItem, m_param)
        if (paramItem.index < 0   ||   paramItem.index >= inputSize)
            throwBoxException("SimpleInputBox: Invalid input port format or invalid coordinate indices");
}

RuntimeBox *SimpleInputBox::newRuntimeBox() const {
    return new SimpleInputRuntimeBox(this);
}

SimpleInputBox::Param SimpleInputBox::param() const {
    return m_param;
}

SimpleInputBox& SimpleInputBox::setParam(const Param& param) {
    m_param = param;
    return *this;
}



SimpleInputRuntimeBox::SimpleInputRuntimeBox(const SimpleInputBox *box) :
    DataInputRuntimeBox(box),
    m_param(box->param())
{
}

InputInfoList SimpleInputRuntimeBox::inputInfo() const {
    QStringList names;
    foreach (const ParamItem& paramItem, m_param)
        names << paramItem.name;
    return InputInfoList() << InputInfo::Ptr(new SimpleInputInfo(owner()->decoratedName(), names));
}

void SimpleInputRuntimeBox::transformData(double *portData, const double *inputData) const
{
    for (int i=0; i<m_param.size(); ++i) {
        int idx = m_param[i].index;
        portData[idx] = inputData[i];
    }
}

QVector<double> SimpleInputRuntimeBox::inputData(const double *portData) const
{
    QVector<double> result(m_param.size());
    for (int i=0; i<m_param.size(); ++i) {
        int idx = m_param[i].index;
        result[i] = portData[idx];
    }
    return result;
}

