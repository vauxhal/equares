#include "JoinBox.h"

REGISTER_BOX(JoinBox, "Join")

JoinBox::JoinBox(QObject *parent) :
    Box(parent),
    m_in1("in_1", this),
    m_in2("in_2", this),
    m_out("out", this)
{
}

InputPorts JoinBox::inputPorts() const {
    return InputPorts() << &m_in1 << &m_in2;
}

OutputPorts JoinBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void JoinBox::checkPortFormat() const {
    if (m_in1.format().dimension() != 1)
        throwBoxException("JoinBox: 1D data was expected in port 'in_1'");
    if (m_in2.format().dimension() != 1)
        throwBoxException("JoinBox: 1D data was expected in port 'in_2'");
    if (m_out.format().dataSize() != m_in1.format().dataSize() + m_in2.format().dataSize())
        throwBoxException("JoinBox: Incompatible input/output port formats");
}

bool JoinBox::propagatePortFormat() {
    if (m_out.format().isValid())
        return false;
    PortFormat f1 = m_in1.format(), f2 = m_in2.format();
    if (f1.dimension() != 1   ||   f2.dimension() != 1)
        return false;
    m_out.format() = PortFormat(f1.dataSize() + f2.dataSize());
    return true;
}

RuntimeBox *JoinBox::newRuntimeBox() const {
    return new JoinRuntimeBox(this);
}



JoinRuntimeBox::JoinRuntimeBox(const JoinBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    for (int i=0; i<2; ++i)
        m_in[i].init(this, in[i], toPortNotifier(&JoinRuntimeBox::setInput), i);
    setInputPorts(RuntimeInputPorts() << &m_in[0] << &m_in[1]);

    m_hasData[0] = m_hasData[1] = false;
    m_availInputs = 0;
    m_idx[0] = 0;
    m_idx[1] = in[0]->format().dataSize();
    m_idx[2] = m_idx[1] + in[1]->format().dataSize();
    m_data.resize(m_idx[2]);

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);
}

bool JoinRuntimeBox::setInput(int portId)
{
    Q_ASSERT(m_in[portId].state().hasData());
    if (m_hasData[portId]) {
        // Overflow
        m_hasData[0] = m_hasData[1] = false;
        // TODO better
        // return false;
        throwBoxException("Join box overflow");
    }
    PortData(m_idx[portId+1]-m_idx[portId], m_out.data() + m_idx[portId]).copyFrom(m_in[portId].data().data());
    if (++m_availInputs < 2) {
        m_hasData[portId] = true;
        return true;
    }
    else {
        m_hasData[1-portId] = false;
        m_availInputs = 0;
        m_out.state().setValid();
        return m_out.activateLinks();
    }
}
