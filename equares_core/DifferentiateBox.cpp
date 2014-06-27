#include "DifferentiateBox.h"
#include "box_util.h"

REGISTER_BOX(DifferentiateBox, "Differentiate")

DifferentiateBox::DifferentiateBox(QObject *parent) :
    Box(parent),
    m_in("input", this),
    m_out("output", this)
{
}

InputPorts DifferentiateBox::inputPorts() const {
    return InputPorts() << &m_in;
}

OutputPorts DifferentiateBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void DifferentiateBox::checkPortFormat() const {
    if (!m_in.format().isValid())
        throwBoxException("DifferentiateBox: Invalid input port format");
    if (!m_out.format().isValid())
        throwBoxException("DifferentiateBox: Invalid output port format");
    if (m_in.format() != m_out.format())
        throwBoxException("DifferentiateBox: Different formats of input and output ports");
}

bool DifferentiateBox::propagatePortFormat() {
    return propagateCommonFormat(m_in, m_out);
}

RuntimeBox *DifferentiateBox::newRuntimeBox() const {
    return new DifferentiateRuntimeBox(this);
}



DifferentiateRuntimeBox::DifferentiateRuntimeBox(const DifferentiateBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(&DifferentiateRuntimeBox::processData));
    setInputPorts(RuntimeInputPorts() << &m_in);

    OutputPorts out = box->outputPorts();
    m_n = in[0]->format().dataSize();
    Q_ASSERT(out[0]->format().dataSize() == m_n);
    m_prevData.resize(m_n);
    m_outData.resize(m_n);
    m_buf = PortData(m_n, m_prevData.data());
    m_out.init(this, out[0], PortData(m_n, m_outData.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_prevDataValid = false;
}

void DifferentiateRuntimeBox::reset() {
    m_prevDataValid = false;
}

bool DifferentiateRuntimeBox::processData(int)
{
    Q_ASSERT (m_in.state().hasData());
    if (m_prevDataValid) {
        double *out = m_out.data().data();
        double *buf = m_buf.data();
        const double *in = m_in.data().data();
        for (int i=0; i<m_n; ++i) {
            out[i] = in[i] - buf[i];
            buf[i] = in[i];
        }
        m_out.state().setValid();
        return m_out.activateLinks();
    }
    else {
        m_in.data().copyTo(m_buf.data());
        m_prevDataValid = true;
        return true;
    }
}
