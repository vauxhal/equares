#include "IntervalFilterBox.h"
#include <cmath>

REGISTER_BOX(IntervalFilterBox, "IntervalFilter")



static QScriptValue ifParamToScriptValue(QScriptEngine *e, const IntervalFilterBoxParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("index", p.index);
    result.setProperty("offset", p.offset);
    result.setProperty("interval", p.interval);
    return result;
}

static void ifParamFromScriptValue(const QScriptValue& v, IntervalFilterBoxParam& result) {
    result = IntervalFilterBoxParam();
    result.index = v.property("index").toInt32();
    result.offset = v.property("offset").toNumber();
    result.interval = v.property("interval").toNumber();
}

static void ifParamScriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, ifParamToScriptValue, ifParamFromScriptValue);
}

REGISTER_SCRIPT_INIT_FUNC(ifParamScriptInit)



IntervalFilterBox::IntervalFilterBox(QObject *parent) :
    Box(parent),
    m_in("input", this),
    m_out("output", this)
{
}

InputPorts IntervalFilterBox::inputPorts() const {
    return InputPorts() << &m_in;
}

OutputPorts IntervalFilterBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void IntervalFilterBox::checkPortFormat() const {
    if (m_in.format() != m_out.format())
        throwBoxException("IntervalFilterBox: Incompatible port formats");
}

bool IntervalFilterBox::propagatePortFormat() {
    if (m_in.format().isValid() == m_out.format().isValid())
        return false;
    if (m_in.format().isValid())
        m_out.format() = m_in.format();
    else
        m_in.format() = m_out.format();
    return true;
}

RuntimeBox *IntervalFilterBox::newRuntimeBox() const {
    return new IntervalFilterRuntimeBox(this);
}

IntervalFilterBox::Param IntervalFilterBox::param() const {
    return m_param;
}

IntervalFilterBox& IntervalFilterBox::setParam(const Param& param) {
    m_param = param;
    return *this;
}



IntervalFilterRuntimeBox::IntervalFilterRuntimeBox(const IntervalFilterBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(&IntervalFilterRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_in);

    m_n = in[0]->format().dataSize();
    m_buf[0].resize(m_n);
    m_buf[1].resize(m_n);
    m_buf[2].resize(m_n);
    m_d1 = m_buf[0].data();
    m_d2 = m_buf[1].data();
    m_param = box->param();
    if (m_param.index < 0   ||   m_param.index >= m_n)
        throwBoxException("IntervalFilterRuntimeBox: Invalid index");
    m_hasPrevData = false;

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0], PortData(m_n, m_buf[2].data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);
}

bool IntervalFilterRuntimeBox::processInput(int)
{
    qSwap(m_d1, m_d2);
    m_in.data().copyTo(m_d2);
    double pos = m_d2[m_param.index];
    if (!m_hasPrevData) {
        m_hasPrevData = true;
        m_nextPos = ceil(pos/m_param.interval)*m_param.interval;
        return true;
    }
    double prevPos = m_d1[m_param.index];
    if (prevPos >= pos)
        return true;
    PortData outData = m_out.data();
    double *dst = outData.data();
    while (pos >= m_nextPos) {
        double t = (m_nextPos - prevPos)/(pos-prevPos);
        interp(dst, t);
        m_out.state().setValid();
        if (!m_out.activateLinks())
            return false;
        m_nextPos += m_param.interval;
    }
    return true;
}

void IntervalFilterRuntimeBox::interp(double *dst, double t)
{
    for (int i=0; i<m_n; ++i)
        dst[i] = m_d1[i] + t*(m_d2[i] - m_d1[i]);
}
