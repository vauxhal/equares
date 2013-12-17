#include "ValveBox.h"

REGISTER_BOX(ValveBox, "Valve")

ValveBox::ValveBox(QObject *parent) :
    Box(parent),
    m_valve("valve", this, PortFormat(1).setFixed()),
    m_in("input", this),
    m_out("output", this)
{
}

InputPorts ValveBox::inputPorts() const {
    return InputPorts() << &m_valve << &m_in;
}

OutputPorts ValveBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void ValveBox::checkPortFormat() const {
    if (m_valve.format() != PortFormat(1))
        throw EquaresException("ValveBox: Incompatible valve port format");
    if (m_in.format() != m_out.format())
        throw EquaresException("ValveBox: Incompatible input/output port formats");
}

bool ValveBox::propagatePortFormat() {
    if (m_in.format().isValid() == m_out.format().isValid())
        return false;
    if (m_in.format().isValid())
        m_out.format() = m_in.format();
    else
        m_in.format() = m_out.format();
    return true;
}

RuntimeBox *ValveBox::newRuntimeBox() const {
    return new ValveRuntimeBox(this);
}



ValveRuntimeBox::ValveRuntimeBox(const ValveBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_valve.init(this, in[0], toPortNotifier(&ValveRuntimeBox::setValve));
    m_in.init(this, in[1], toPortNotifier(&ValveRuntimeBox::setInput));
    setInputPorts(RuntimeInputPorts() << &m_valve << &m_in);

    m_hasValve = false;
    m_hasData = false;
    m_data.resize(in[1]->format().dataSize());

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);
}

bool ValveRuntimeBox::setValve()
{
    Q_ASSERT(m_valve.outputPort()->state().hasData());
    m_valveOpen = m_valve.data().data()[0] != 0;
    m_hasValve = true;
    return processData();
}

bool ValveRuntimeBox::setInput()
{
    Q_ASSERT(m_in.outputPort()->state().hasData());
    m_out.data().copyFrom(m_in.outputPort()->data().data());
    m_out.state().setValid();
    m_hasData = true;
    return processData();
}

bool ValveRuntimeBox::processData()
{
    if (!(m_hasValve && m_hasData))
        // Waiting for all inputs
        return true;

    // Prepare to accept new inputs
    m_hasValve = m_hasData = false;

    if (!m_valveOpen)
        // Valve is closed, ignore data input and return true
        return true;

    // Valve is open, all inputs are available - try to pass data
    return m_out.activateLinks();
}
