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



SimpleInputBox::SimpleInputBox(QObject *parent) :
    Box(parent),
    m_sync(false),
    m_loop(false),
    m_activator("activator", this),
    m_in("input", this),
    m_out("output", this)
{
}

InputPorts SimpleInputBox::inputPorts() const {
    return InputPorts() << &m_activator << &m_in;
}

OutputPorts SimpleInputBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void SimpleInputBox::checkPortFormat() const
{
    if (!m_in.format().isValid())
        throwBoxException("SimpleInputBox: no format is specified for port 'input'");
    if (m_in.format().dimension() != 1)
        throwBoxException("SimpleInputBox: an 1D format was expected for port 'input'");
    int inputSize = m_in.format().size(0);
    foreach (const ParamItem& paramItem, m_param) {
        if (paramItem.index < 0   ||   paramItem.index >= inputSize)
            throwBoxException("SimpleInputBox: Invalid input port format or invalid coordinate indices");
    }
    if (m_in.format() != m_out.format())
        throwBoxException("SimpleInputBox: Incompatible input/output port formats");
}

bool SimpleInputBox::propagatePortFormat() {
    if (m_in.format().isValid() == m_out.format().isValid())
        return false;
    if (m_in.format().isValid())
        m_out.format() = m_in.format();
    else
        m_in.format() = m_out.format();
    return true;
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

bool SimpleInputBox::sync() const {
    return m_sync;
}

SimpleInputBox& SimpleInputBox::setSync(bool sync) {
    m_sync = sync;
    return *this;
}

bool SimpleInputBox::loop() const {
    return m_loop;
}

SimpleInputBox& SimpleInputBox::setLoop(bool loop) {
    m_loop = loop;
    return *this;
}



SimpleInputRuntimeBox::SimpleInputRuntimeBox(const SimpleInputBox *box) :
    m_param(box->param()),
    m_sync(box->sync()),
    m_loop(box->loop())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_activator.init(this, in[0], toPortNotifier(&SimpleInputRuntimeBox::activate));
    m_in.init(this, in[1], toPortNotifier(&SimpleInputRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_activator << &m_in);

    OutputPorts out = box->outputPorts();
    m_data.resize(out[0]->format().dataSize());
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_inputId = -1;
    m_dataValid = false;
    m_iinputDataValid = false;
}

InputInfoList SimpleInputRuntimeBox::inputInfo() const {
    QStringList names;
    foreach (const ParamItem& paramItem, m_param)
        names << paramItem.name;
    return InputInfoList() << InputInfo::Ptr(new SimpleInputInfo(owner()->name(), names));
}

void SimpleInputRuntimeBox::registerInput()
{
    m_inputId = ThreadManager::instance()->registerInput(inputInfo()[0]);
}

bool SimpleInputRuntimeBox::fetchInputPortData()
{
    if (m_dataValid)
        return true;
    if (!m_in.state().hasData())
        return false;
    Q_ASSERT(m_in.data().size() == m_data.size());
    m_in.data().copyTo(m_data.data());
    m_out.state().setValid();
    m_dataValid = true;
    return true;
}

bool SimpleInputRuntimeBox::activate()
{
    forever {
        if (!fetchInputPortData())
            return false;
        if (!m_iinputDataValid) {
            m_iinputData = ThreadManager::instance()->readInput(m_inputId, m_sync);
            m_iinputDataValid = true;
        }
        if (m_iinputData.isEmpty()) {
            if (!m_sync)
                m_iinputDataValid = false;
            return true;
        }
        Q_ASSERT(m_iinputData.size() == m_param.size());
        for (int i=0; i<m_param.size(); ++i) {
            int idx = m_param[i].index;
            Q_ASSERT(idx >= 0   &&   idx < m_data.size());
            m_data[idx] = m_iinputData[i];
        }
        m_out.state().setValid();
        if (m_out.activateLinks())
            m_iinputDataValid = false;
        else
            return false;
        if (!(m_loop && m_sync))
            return true;
    }
}

bool SimpleInputRuntimeBox::processInput()
{
    Q_ASSERT(m_in.state().hasData());
    m_dataValid = false;
    if (m_sync)
        return activate();
    else {
        fetchInputPortData();
        return m_out.activateLinks();
    }
}
