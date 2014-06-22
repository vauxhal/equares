#include "SignalInputBox.h"

REGISTER_BOX(SignalInputBox, "SignalInput")



SignalInputBox::SignalInputBox(QObject *parent) :
    Box(parent),
    m_sync(true),
    m_loop(true),
    m_activator("activator", this),
    m_out("output", this, PortFormat(0).setFixed())
{
}

InputPorts SignalInputBox::inputPorts() const {
    return InputPorts() << &m_activator;
}

OutputPorts SignalInputBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void SignalInputBox::checkPortFormat() const
{
}

bool SignalInputBox::propagatePortFormat() {
    return false;
}

RuntimeBox *SignalInputBox::newRuntimeBox() const {
    return new SignalInputRuntimeBox(this);
}

QString SignalInputBox::signalName() const {
    return m_signalName;
}

SignalInputBox& SignalInputBox::setSignalName(const QString& signalName) {
    m_signalName = signalName;
    return *this;
}

bool SignalInputBox::sync() const {
    return m_sync;
}

SignalInputBox& SignalInputBox::setSync(bool sync) {
    m_sync = sync;
    return *this;
}

bool SignalInputBox::loop() const {
    return m_loop;
}

SignalInputBox& SignalInputBox::setLoop(bool loop) {
    m_loop = loop;
    return *this;
}



SignalInputRuntimeBox::SignalInputRuntimeBox(const SignalInputBox *box) :
    m_signalName(box->signalName()),
    m_sync(box->sync()),
    m_loop(box->loop())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_activator.init(this, in[0], toPortNotifier(&SignalInputRuntimeBox::activate));
    setInputPorts(RuntimeInputPorts() << &m_activator);

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0]);
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_inputId = -1;
    m_iinputDataValid = false;
}

InputInfoList SignalInputRuntimeBox::inputInfo() const {
    return InputInfoList() << InputInfo::Ptr(new SignalInputInfo(owner()->name(), m_signalName));
}

void SignalInputRuntimeBox::registerInput()
{
    m_inputId = ThreadManager::instance()->registerInput(inputInfo()[0]);
}

bool SignalInputRuntimeBox::activate(int)
{
    forever {
        if (!m_iinputDataValid) {
            m_inputDataEmpty = ThreadManager::instance()->readInput(m_inputId, m_sync).isEmpty();
            m_iinputDataValid = true;
        }
        if (m_inputDataEmpty) {
            if (!m_sync)
                m_iinputDataValid = false;
            return true;
        }
        if (m_out.activateLinks())
            m_iinputDataValid = false;
        else
            return false;
        if (!(m_loop && m_sync))
            return true;
    }
}
