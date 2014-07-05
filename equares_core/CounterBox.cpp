#include "CounterBox.h"

REGISTER_BOX(CounterBox, "Counter")

CounterBox::CounterBox(QObject *parent) :
    Box(parent),
    m_in("input", this),
    m_reset("reset", this),
    m_count("count", this, PortFormat(1).setFixed())
{
}

InputPorts CounterBox::inputPorts() const {
    return InputPorts() << &m_in << &m_reset;
}

OutputPorts CounterBox::outputPorts() const {
    return OutputPorts() << &m_count;
}

void CounterBox::checkPortFormat() const {
    if (m_count.format() != PortFormat(1))
        throwBoxException("CounterBox: Invalid output port format (must be scalar)");
}

bool CounterBox::propagatePortFormat() {
    return false;
}

RuntimeBox *CounterBox::newRuntimeBox() const {
    return new CounterRuntimeBox(this);
}



CounterRuntimeBox::CounterRuntimeBox(const CounterBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(&CounterRuntimeBox::processData));
    m_reset.init(this, in[1], toPortNotifier(&CounterRuntimeBox::resetCounter));
    setInputPorts(RuntimeInputPorts() << &m_in << &m_reset);

    OutputPorts out = box->outputPorts();
    m_countData = 0;
    m_count.init(this, out[0], PortData(1, &m_countData));
    m_count.state().setValid();
    setOutputPorts(RuntimeOutputPorts() << &m_count);

    Q_ASSERT(out[0]->format().dataSize() == 1);
}

bool CounterRuntimeBox::processData(int)
{
    ++*m_count.data().data();
    return m_count.activateLinks();
}

bool CounterRuntimeBox::resetCounter(int)
{
    *m_count.data().data() = 0;
    return true;
}
