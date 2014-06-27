#include "CountedFilterBox.h"
#include "box_util.h"

REGISTER_BOX(CountedFilterBox, "CountedFilter")

CountedFilterBox::CountedFilterBox(QObject *parent) :
    Box(parent),
    m_count("count", this, PortFormat(1).setFixed()),
    m_in("input", this),
    m_out("output", this)
{
}

InputPorts CountedFilterBox::inputPorts() const {
    return InputPorts() << &m_count << &m_in;
}

OutputPorts CountedFilterBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void CountedFilterBox::checkPortFormat() const {
    if (m_count.format() != PortFormat(1))
        throwBoxException("CountedFilterBox: Incompatible count port format");
    if (m_in.format() != m_out.format())
        throwBoxException("CountedFilterBox: Incompatible input/output port formats");
}

bool CountedFilterBox::propagatePortFormat() {
    return propagateCommonFormat(m_in, m_out);
}

RuntimeBox *CountedFilterBox::newRuntimeBox() const {
    return new CountedFilterRuntimeBox(this);
}



CountedFilterRuntimeBox::CountedFilterRuntimeBox(const CountedFilterBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_count.init(this, in[0], toPortNotifier(&CountedFilterRuntimeBox::setCount));
    m_in.init(this, in[1], toPortNotifier(&CountedFilterRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_count << &m_in);

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0]);
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_hasCounter = false;
}

bool CountedFilterRuntimeBox::setCount(int)
{
    Q_ASSERT(m_count.outputPort()->state().hasData());
    m_c0 = static_cast<int>(m_count.data().data()[0]);
    m_c = m_c0;
    m_hasCounter = true;
    return true;
}

bool CountedFilterRuntimeBox::processInput(int)
{
    if (!m_hasCounter)
        return false;
    if (--m_c)
        return true;
    m_c = m_c0;
    if(m_in.outputPort()->state().hasData()) {
        m_out.setData(m_in.outputPort()->data());
        m_out.state().setValid();
    }
    return m_out.activateLinks();
}
