#include "ThresholdDetectorBox.h"
#include "box_util.h"
#include "equares_script.h"

REGISTER_BOX(ThresholdDetectorBox, "ThresholdDetector")

static NameMapper<ThresholdDetectorBoxParam>::Item scalarizeBoxParamNames[] = {
    {ThresholdLess, "less"},
    {ThresholdLessOrEqual, "less_or_equal"},
    {ThresholdGreater, "greater"},
    {ThresholdGreaterOrEqual, "greater_or_equal"},
    {ThresholdEqual, "equal"},
    {ThresholdNotEqual, "not_equal"},
    {static_cast<ThresholdDetectorBoxParam>(-1), 0 }
};

static NameMapper<ThresholdDetectorBoxParam> scalarizeBoxParamNameMap(scalarizeBoxParamNames);

template<class T> QScriptValue toScriptValue(QScriptEngine *e, const T& p);
template<class T> void fromScriptValue(const QScriptValue& v, T& result);

template<>
QScriptValue toScriptValue(QScriptEngine *e, const ThresholdDetectorBoxParam& p) {
    Q_UNUSED(e);
    return scalarizeBoxParamNameMap.toName(p);
}

template<>
void fromScriptValue(const QScriptValue& v, ThresholdDetectorBoxParam& result) {
    result = scalarizeBoxParamNameMap.fromName(v.toString());
}

static void scriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, toScriptValue<ThresholdDetectorBoxParam>, fromScriptValue<ThresholdDetectorBoxParam>);
}

REGISTER_SCRIPT_INIT_FUNC(scriptInit)



ThresholdDetectorBox::ThresholdDetectorBox(QObject *parent) :
    Box(parent),
    m_param(ThresholdLess),
    m_quiet(false),
    m_threshold("threshold", this, PortFormat(1).setFixed()),
    m_in("input", this, PortFormat(1).setFixed()),
    m_out("output", this, PortFormat(1).setFixed())
{
}

InputPorts ThresholdDetectorBox::inputPorts() const {
    return InputPorts() << &m_threshold << &m_in;
}

OutputPorts ThresholdDetectorBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void ThresholdDetectorBox::checkPortFormat() const {
    if (m_threshold.format() != PortFormat(1))
        throwBoxException("ThresholdDetectorBox: Invalid threshold port format (must be scalar)");
    if (m_in.format() != PortFormat(1))
        throwBoxException("ThresholdDetectorBox: Invalid input port format (must be scalar)");
    if (m_out.format() != PortFormat(1))
        throwBoxException("ThresholdDetectorBox: Invalid output port format (must be scalar)");
}

bool ThresholdDetectorBox::propagatePortFormat() {
    return propagateCommonFormat(m_in, m_out);
}

RuntimeBox *ThresholdDetectorBox::newRuntimeBox() const {
    return new ThresholdDetectorRuntimeBox(this);
}

ThresholdDetectorBox::Param ThresholdDetectorBox::param() const {
    return m_param;
}

ThresholdDetectorBox& ThresholdDetectorBox::setParam(const Param& param) {
    m_param = param;
    return *this;
}
bool ThresholdDetectorBox::quiet() const {
    return m_quiet;
}

ThresholdDetectorBox& ThresholdDetectorBox::setQuiet(bool quiet) {
    m_quiet = quiet;
    return *this;
}



ThresholdDetectorRuntimeBox::ThresholdDetectorRuntimeBox(const ThresholdDetectorBox *box) :
    m_quiet(box->quiet())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_threshold.init(this, in[0], toPortNotifier(&ThresholdDetectorRuntimeBox::setThreshold));
    m_in.init(this, in[1], toPortNotifier(&ThresholdDetectorRuntimeBox::processData));
    setInputPorts(RuntimeInputPorts() << &m_threshold << &m_in);

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0]);
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    Q_ASSERT(in[0]->format().dataSize() == 1);
    Q_ASSERT(in[1]->format().dataSize() == 1);
    Q_ASSERT(out[0]->format().dataSize() == 1);

    switch (box->param()) {
    case ThresholdLess:   m_thresholdFunc = &ThresholdDetectorRuntimeBox::thresholdLess;   break;
    case ThresholdLessOrEqual:   m_thresholdFunc = &ThresholdDetectorRuntimeBox::thresholdLessOrEqual;   break;
    case ThresholdGreater:   m_thresholdFunc = &ThresholdDetectorRuntimeBox::thresholdGreater;   break;
    case ThresholdGreaterOrEqual:   m_thresholdFunc = &ThresholdDetectorRuntimeBox::thresholdGreaterOrEqual;   break;
    case ThresholdEqual:   m_thresholdFunc = &ThresholdDetectorRuntimeBox::thresholdEqual;   break;
    case ThresholdNotEqual:   m_thresholdFunc = &ThresholdDetectorRuntimeBox::thresholdNotEqual;   break;
    default:
        throwBoxException("Invalid threshold parameter");
    }
    m_thresholdDataValid = false;
}

bool ThresholdDetectorRuntimeBox::setThreshold(int)
{
    m_thresholdData = m_threshold.data().data()[0];
    m_thresholdDataValid = true;
    return true;
}

bool ThresholdDetectorRuntimeBox::processData(int)
{
    if (!m_threshold.state().hasData())
        return false;
    Q_ASSERT(m_in.state().hasData());
    m_outData = (this->*m_thresholdFunc)(m_in.data().data()[0]);
    if (m_quiet && !m_outData)
        return true;
    m_out.setData(PortData(1, &m_outData));
    m_out.state().setValid();
    return m_out.activateLinks();
}

double ThresholdDetectorRuntimeBox::thresholdLess(double src) const { return src < m_thresholdData; }
double ThresholdDetectorRuntimeBox::thresholdLessOrEqual(double src) const { return src <= m_thresholdData; }
double ThresholdDetectorRuntimeBox::thresholdGreater(double src) const { return src > m_thresholdData; }
double ThresholdDetectorRuntimeBox::thresholdGreaterOrEqual(double src) const { return src >= m_thresholdData; }
double ThresholdDetectorRuntimeBox::thresholdEqual(double src) const { return src == m_thresholdData; }
double ThresholdDetectorRuntimeBox::thresholdNotEqual(double src) const { return src != m_thresholdData; }
