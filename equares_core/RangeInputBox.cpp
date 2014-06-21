#include "RangeInputBox.h"

REGISTER_BOX(RangeInputBox, "RangeInput")

template<class T> QScriptValue toScriptValue(QScriptEngine *e, const T& p);
template<class T> void fromScriptValue(const QScriptValue& v, T& result);

template<>
QScriptValue toScriptValue(QScriptEngine *e, const RangeInputBoxParamItem& p) {
    QScriptValue result = e->newObject();
    result.setProperty("index", p.index);
    result.setProperty("name", p.name);
    result.setProperty("vmin", p.vmin);
    result.setProperty("vmax", p.vmax);
    result.setProperty("resolution", p.resolution);
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, RangeInputBoxParamItem& result) {
    result = RangeInputBoxParamItem();
    result.index = v.property("index").toInt32();
    result.name = v.property("name").toString();
    result.vmin = v.property("vmin").toNumber();
    result.vmax = v.property("vmax").toNumber();
    result.resolution = v.property("resolution").toInt32();
}

template<>
QScriptValue toScriptValue(QScriptEngine *e, const RangeInputBoxParam& p) {
    int n = p.size();
    QScriptValue result = e->newArray(n);
    for (int i=0; i<n; ++i)
        result.setProperty(i, toScriptValue(e, p[i]));
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, RangeInputBoxParam& result) {
    int n = v.property("length").toInt32();
    result = RangeInputBoxParam(n);
    for (int i=0; i<n; ++i)
        fromScriptValue(v.property(i), result[i]);
}

static void scriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, toScriptValue<RangeInputBoxParamItem>, fromScriptValue<RangeInputBoxParamItem>);
    qScriptRegisterMetaType(e, toScriptValue<RangeInputBoxParam>, fromScriptValue<RangeInputBoxParam>);
}

REGISTER_SCRIPT_INIT_FUNC(scriptInit)



RangeInputBox::RangeInputBox(QObject *parent) : DataInputBox(parent)
{
}

void RangeInputBox::checkPortFormat() const
{
    int inputSize = inputPorts()[1]->format().size(0);
    foreach (const ParamItem& paramItem, m_param)
        if (paramItem.index < 0   ||   paramItem.index >= inputSize)
            throwBoxException("RangeInputBox: Invalid input port format or invalid coordinate indices");
}

RuntimeBox *RangeInputBox::newRuntimeBox() const {
    return new RangeInputRuntimeBox(this);
}

RangeInputBox::Param RangeInputBox::param() const {
    return m_param;
}

RangeInputBox& RangeInputBox::setParam(const Param& param) {
    m_param = param;
    return *this;
}



RangeInputRuntimeBox::RangeInputRuntimeBox(const RangeInputBox *box) :
    DataInputRuntimeBox(box),
    m_param(box->param())
{
}

InputInfoList RangeInputRuntimeBox::inputInfo() const {
    RangeInputInfo::Ranges ranges;
    foreach (const ParamItem& p, m_param)
        ranges << RangeInputInfo::Range(p.name, p.vmin, p.vmax, p.resolution);
    return InputInfoList() << InputInfo::Ptr(new RangeInputInfo(owner()->name(), ranges));
}

void RangeInputRuntimeBox::transformData(double *portData, const double *inputData) const
{
    for (int i=0; i<m_param.size(); ++i) {
        int idx = m_param[i].index;
        portData[idx] = inputData[i];
    }
}

QVector<double> RangeInputRuntimeBox::inputData(const double *portData) const
{
    QVector<double> result(m_param.size());
    for (int i=0; i<m_param.size(); ++i) {
        int idx = m_param[i].index;
        result[i] = portData[idx];
    }
    return result;
}
