#include "InterpolatorBox.h"
#include "box_util.h"

REGISTER_BOX(InterpolatorBox, "Interpolator")

InterpolatorBox::InterpolatorBox(QObject *parent) :
    Box(parent),
    m_in("input", this),
    m_out("output", this),
    m_count(10)
{
}

InputPorts InterpolatorBox::inputPorts() const {
    return InputPorts() << &m_in;
}

OutputPorts InterpolatorBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void InterpolatorBox::checkPortFormat() const {
    if (m_in.format() != m_out.format())
        throwBoxException("InterpolatorBox: Incompatible port formats");
}

bool InterpolatorBox::propagatePortFormat() {
    return propagateCommonFormat(m_in, m_out);
}

RuntimeBox *InterpolatorBox::newRuntimeBox() const {
    return new InterpolatorRuntimeBox(this);
}

int InterpolatorBox::count() const {
    return m_count;
}

InterpolatorBox& InterpolatorBox::setCount(int count) {
    m_count = count;
    return *this;
}



InterpolatorRuntimeBox::InterpolatorRuntimeBox(const InterpolatorBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(&InterpolatorRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_in);

    m_n = in[0]->format().dataSize();
    m_buf[0].resize(m_n);
    m_buf[1].resize(m_n);
    m_buf[2].resize(m_n);
    m_d1 = m_buf[0].data();
    m_d2 = m_buf[1].data();
    m_count = box->count();
    if (m_count < 1)
        throwBoxException("InterpolatorRuntimeBox: Invalid interval count");
    m_hasPrevData = false;

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0], PortData(m_n, m_buf[2].data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);
}

bool InterpolatorRuntimeBox::processInput(int)
{
    qSwap(m_d1, m_d2);
    m_in.data().copyTo(m_d2);
    if (!m_hasPrevData) {
        m_hasPrevData = true;
        m_out.data().copyFrom(m_d2);
        m_out.state().setValid();
        return m_out.activateLinks();
    }
    PortData outData = m_out.data();
    double *dst = outData.data();
    double dt = 1. / m_count,   t = dt;
    for (int i=0; i<m_count; ++i, t+=dt) {
        interp(dst, t);
        m_out.state().setValid();
        if (!m_out.activateLinks())
            return false;
    }
    return true;
}

void InterpolatorRuntimeBox::interp(double *dst, double t)
{
    for (int i=0; i<m_n; ++i)
        dst[i] = m_d1[i] + t*(m_d2[i] - m_d1[i]);
}
