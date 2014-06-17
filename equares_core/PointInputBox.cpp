#include "PointInputBox.h"

REGISTER_BOX(PointInputBox, "PointInput")



template<class T> QScriptValue toScriptValue(QScriptEngine *e, const T& p);
template<class T> void fromScriptValue(const QScriptValue& v, T& result);

template<>
QScriptValue toScriptValue(QScriptEngine *e, const PointInputBoxDimParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("index", p.index);
    result.setProperty("vmin", p.vmin);
    result.setProperty("vmax", p.vmax);
    result.setProperty("resolution", p.resolution);
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, PointInputBoxDimParam& result) {
    result = PointInputBoxDimParam();
    result.index = v.property("index").toInt32();
    result.vmin = v.property("vmin").toNumber();
    result.vmax = v.property("vmax").toNumber();
    result.resolution = v.property("resolution").toInt32();
}

template<>
QScriptValue toScriptValue(QScriptEngine *e, const PointInputBoxParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("x", toScriptValue(e, p[0]));
    result.setProperty("y", toScriptValue(e, p[1]));
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, PointInputBoxParam& result) {
    result = PointInputBoxParam();
    fromScriptValue(v.property("x"), result[0]);
    fromScriptValue(v.property("y"), result[1]);
}

static void scriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, toScriptValue<PointInputBoxDimParam>, fromScriptValue<PointInputBoxDimParam>);
    qScriptRegisterMetaType(e, toScriptValue<PointInputBoxParam>, fromScriptValue<PointInputBoxParam>);
}

REGISTER_SCRIPT_INIT_FUNC(scriptInit)



PointInputBox::PointInputBox(QObject *parent) :
    Box(parent),
    m_sync(true),
    m_activator("activator", this),
    m_in("input", this),
    m_out("output", this)
{
}

InputPorts PointInputBox::inputPorts() const {
    return InputPorts() << &m_activator << &m_in;
}

OutputPorts PointInputBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void PointInputBox::checkPortFormat() const
{
    if (!m_in.format().isValid())
        throwBoxException("PointInputBox: no format is specified for port 'input'");
    if (m_in.format().dimension() != 1)
        throwBoxException("PointInputBox: an 1D format was expected for port 'input'");
    int inputSize = m_in.format().size(0);
    for (int i=0; i<2; ++i) {
        if (m_param[i].index < 0   ||   m_param[i].index >= inputSize)
            throwBoxException("PointInputBox: Invalid input port format or invalid coordinate indices");
        if (m_param[i].resolution <= 0)
            throwBoxException(QString("PointInputBox: Invalid grid resolution %1 - should be positive").arg(m_param[i].resolution));
    }
    if (m_in.format() != m_out.format())
        throwBoxException("PointInputBox: Incompatible input/output port formats");
}

bool PointInputBox::propagatePortFormat() {
    if (m_in.format().isValid() == m_out.format().isValid())
        return false;
    if (m_in.format().isValid())
        m_out.format() = m_in.format();
    else
        m_in.format() = m_out.format();
    return true;
}

RuntimeBox *PointInputBox::newRuntimeBox() const {
    return new PointInputRuntimeBox(this);
}

PointInputBox::Param PointInputBox::param() const {
    return m_param;
}

PointInputBox& PointInputBox::setParam(const Param& param) {
    for (int i=0; i<2; ++i) {
        if (m_param[i].resolution <= 0)
            throwBoxException(QString("PointInputBox: Unable to set parameter: invalid grid resolution %1 - should be positive").arg(m_param[i].resolution));
    }
    m_param = param;
    return *this;
}

bool PointInputBox::sync() const {
    return m_sync;
}

PointInputBox& PointInputBox::setSync(bool sync) {
    m_sync = sync;
    return *this;
}

QString PointInputBox::refBitmap() const {
    return m_refBitmap;
}

PointInputBox& PointInputBox::setRefBitmap(const QString& refBitmap) {
    m_refBitmap = refBitmap;
    return *this;
}



PointInputRuntimeBox::PointInputRuntimeBox(const PointInputBox *box) :
    m_param(box->param()),
    m_refBitmap(box->refBitmap())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_activator.init(this, in[0], toPortNotifier(&PointInputRuntimeBox::activate));
    m_in.init(this, in[1], toPortNotifier(&PointInputRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_activator << &m_in);

    OutputPorts out = box->outputPorts();
    m_data.resize(out[0]->format().dataSize());
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);
}

bool PointInputRuntimeBox::activate()
{
    return true; // TODO
}

bool PointInputRuntimeBox::processInput()
{
    // TODO
    Q_ASSERT(m_in.state().hasData());
    return m_out.activateLinks();
}
