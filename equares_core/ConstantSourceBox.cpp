#include "ConstantSourceBox.h"
#include <QScriptContext>

REGISTER_BOX(ConstantSourceBox, "Param")

ConstantSourceBox::ConstantSourceBox(QObject *parent) :
    Box(parent),
    m_outputPort("output", this)
{
}

InputPorts ConstantSourceBox::inputPorts() const {
    return InputPorts();
}

OutputPorts ConstantSourceBox::outputPorts() const {
    return OutputPorts() << &m_outputPort;
}

RuntimeBox *ConstantSourceBox::newRuntimeBox() const {
    return new ConstantSourceRuntimeBox(this);
}

void ConstantSourceBox::checkPortFormat() const
{
}

bool ConstantSourceBox::propagatePortFormat()
{
    return false;
}

double *ConstantSourceBox::data() const
{
    int dataSize = m_outputPort.format().dataSize();
    if (m_data.size() != dataSize)
        m_data.resize(dataSize);
    return m_data.data();
}

const QVector<double>& ConstantSourceBox::getData() const {
    data();
    return m_data;
}

void ConstantSourceBox::setData(const QVector<double>& data) {
    /* TODO: Remove (relax data check)
    this->data();
    if (data.size() != m_data.size()) {
        context()->throwError(QScriptContext::RangeError, "ConstantSourceBox::setData: Invalid data size");
        return;
    }
    */
    m_data = data;
}

Port *ConstantSourceBox::getOut() const {
    return &m_outputPort;
}



ConstantSourceRuntimeBox::ConstantSourceRuntimeBox(const ConstantSourceBox *box)
{
    setOwner(box);

    OutputPort *port = box->outputPorts()[0];
    m_outputPort.init(this, port, PortData(port->format().dataSize(), box->data()));
    m_outputPort.state().setValid();
    setOutputPorts(RuntimeOutputPorts() << &m_outputPort);
}
