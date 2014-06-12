#include "ParamArrayBox.h"

REGISTER_BOX(ParamArrayBox, "ParamArray")

ParamArrayBox::ParamArrayBox(QObject *parent) :
    Box(parent),
    m_out("output", this),
    m_flush("flush", this, PortFormat(0).setFixed())
{
}

InputPorts ParamArrayBox::inputPorts() const {
    return InputPorts();
}

OutputPorts ParamArrayBox::outputPorts() const {
    return OutputPorts() << &m_out << &m_flush;
}

RuntimeBox *ParamArrayBox::newRuntimeBox() const {
    return new ParamArrayRuntimeBox(this);
}

void ParamArrayBox::checkPortFormat() const
{
}

bool ParamArrayBox::propagatePortFormat()
{
    return false;
}

double *ParamArrayBox::data() const
{
    int frameSize = m_out.format().dataSize();
    if (m_data.size() % frameSize != 0) {
        int n = m_data.size() / frameSize + 1;
        m_data.resize(n * frameSize);
    }
    return m_data.data();
}

const QVector<double>& ParamArrayBox::getData() const {
    data();
    return m_data;
}

void ParamArrayBox::setData(const QVector<double>& data) {
    m_data = data;
}

Port *ParamArrayBox::getOut() const {
    return &m_out;
}



ParamArrayRuntimeBox::ParamArrayRuntimeBox(const ParamArrayBox *box)
{
    setOwner(box);

    OutputPorts out = box->outputPorts();
    m_frameSize = out[0]->format().dataSize();
    m_frameCount = m_frameSize > 0 ?   box->getData().size() / m_frameSize :   0;
    m_data = box->data();
    m_out.init(this, out[0]);
    m_flush.init(this, out[1]);
    setOutputPorts(RuntimeOutputPorts() << &m_out << &m_flush);
}

RuntimeBox::PortNotifier ParamArrayRuntimeBox::generator() const
{
    return toPortNotifier(&ParamArrayRuntimeBox::generate);
}

bool ParamArrayRuntimeBox::generate()
{
    double *data = m_data;
    for (int i=0; i<m_frameCount; ++i, data += m_frameSize) {
        m_out.setData(PortData(m_frameSize, data));
        m_out.state().setValid();
        if (!m_out.activateLinks())
            return false;
    }
    return m_flush.activateLinks();
}
