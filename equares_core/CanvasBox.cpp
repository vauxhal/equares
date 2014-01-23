#include "CanvasBox.h"

REGISTER_BOX(CanvasBox, "Canvas")



static QScriptValue canvasDimParamToScriptValue(QScriptEngine *e, const CanvasBoxDimParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("vmin", p.vmin);
    result.setProperty("vmax", p.vmax);
    result.setProperty("resolution", p.resolution);
    return result;
}

static void canvasDimParamFromScriptValue(const QScriptValue& v, CanvasBoxDimParam& result) {
    result = CanvasBoxDimParam();
    result.vmin = v.property("vmin").toNumber();
    result.vmax = v.property("vmax").toNumber();
    result.resolution = v.property("resolution").toInt32();
}

static QScriptValue canvasParamToScriptValue(QScriptEngine *e, const CanvasBoxParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("x", canvasDimParamToScriptValue(e, p[0]));
    result.setProperty("y", canvasDimParamToScriptValue(e, p[1]));
    return result;
}

static void canvasParamFromScriptValue(const QScriptValue& v, CanvasBoxParam& result) {
    result = CanvasBoxParam();
    canvasDimParamFromScriptValue(v.property("x"), result[0]);
    canvasDimParamFromScriptValue(v.property("y"), result[1]);
}

static void canvasParamScriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, canvasDimParamToScriptValue, canvasDimParamFromScriptValue);
    qScriptRegisterMetaType(e, canvasParamToScriptValue, canvasParamFromScriptValue);
}

REGISTER_SCRIPT_INIT_FUNC(canvasParamScriptInit)



CanvasBox::CanvasBox(QObject *parent) :
    Box(parent),
    m_refreshInterval(0),
    m_in("input", this, PortFormat(2).setFixed()),
    m_flush("flush", this),
    m_out("output", this, PortFormat(m_param[0].resolution, m_param[1].resolution).setFixed())
{
}

InputPorts CanvasBox::inputPorts() const {
    return InputPorts() << &m_in << &m_flush;
}

OutputPorts CanvasBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void CanvasBox::checkPortFormat() const
{
    if (m_in.format() != PortFormat(2))
        throw EquaresException("CanvasBox: Invalid input port format");
    if (m_out.format() != PortFormat(m_param[0].resolution, m_param[1].resolution))
        throw EquaresException("CanvasBox: Invalid output port format");
}

bool CanvasBox::propagatePortFormat() {
    return false;
}

RuntimeBox *CanvasBox::newRuntimeBox() const {
    return new CanvasRuntimeBox(this);
}

CanvasBox::Param CanvasBox::param() const {
    return m_param;
}

CanvasBox& CanvasBox::setParam(const Param& param) {
    m_param = param;
    m_out.format().setSize(0, param[0].resolution);
    m_out.format().setSize(1, param[1].resolution);
    return *this;
}

int CanvasBox::refreshInterval() const {
    return m_refreshInterval;
}

CanvasBox& CanvasBox::setRefreshInterval(int refreshInterval) {
    m_refreshInterval = refreshInterval;
    return *this;
}



CanvasRuntimeBox::CanvasRuntimeBox(const CanvasBox *box) :
    m_param(box->param()),
    m_refreshInterval(box->refreshInterval())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(&CanvasRuntimeBox::processInput));
    m_flush.init(this, in[1], PortNotifier(&CanvasRuntimeBox::flush));
    setInputPorts(RuntimeInputPorts() << &m_in << &m_flush);

    OutputPorts out = box->outputPorts();
    m_data = QVector<double>(out[0]->format().dataSize(), 0);
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_time.start();
}

bool CanvasRuntimeBox::processInput()
{
    Q_ASSERT(m_in.state().hasData());
    int i = m_param.index(m_in.outputPort()->data().data());
    if (i >= 0) {
        double *data = m_out.data().data();
        data[i] = 1;
        if (m_refreshInterval > 0   &&   m_time.elapsed() >= m_refreshInterval) {
            flush();
            m_time.restart();
        }
    }
    return true;
}

bool CanvasRuntimeBox::flush()
{
    m_out.state().setValid();
    return m_out.activateLinks();
}
