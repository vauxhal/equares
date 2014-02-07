#include "Rk4Box.h"
#include <limits>

REGISTER_BOX(Rk4Box, "Rk4")

Rk4Box::Rk4Box(QObject *parent) :
    Box(parent),
    m_param("parameters", this, PortFormat(3).setFixed()),  // h, n, outputControl
    m_initState("initState", this),
    m_rhs("rhs", this),
    m_terminator("stop", this),
    m_nextState("nextState", this),
    m_rhsState("rhsState", this),
    m_finish("finish", this, PortFormat(0).setFixed())
{
}

InputPorts Rk4Box::inputPorts() const {
    return InputPorts() << &m_param << &m_initState << &m_rhs << &m_terminator;
}

OutputPorts Rk4Box::outputPorts() const {
    return OutputPorts() << &m_nextState << &m_rhsState << &m_finish;
}

void Rk4Box::checkPortFormat() const
{
    if (m_param.format() != PortFormat(3))
        throwBoxException("Rk4Box: port 'parameters' has an invalid size");
    PortFormat rhsfmt = m_rhs.format();
    if (!rhsfmt.isValid())
        throwBoxException("Rk4Box: no format is specified for port 'rhs'");
    if (rhsfmt.dimension() != 1)
        throwBoxException("Rk4Box: port 'rhs' has an invalid dimension");
    if (rhsfmt.size(0) == 0)
        throwBoxException("Rk4Box: port 'rhs' has an empty length");
    PortFormat stfmt = rhsfmt;
    stfmt.setSize(0, rhsfmt.size(0)+1);
    if (m_initState.format() != stfmt)
        throwBoxException("Rk4Box: port 'initState' has an incompatible format");
    if (m_rhsState.format() != stfmt)
        throwBoxException("Rk4Box: port 'rhsState' has an incompatible format");
    if (m_nextState.format() != stfmt)
        throwBoxException("Rk4Box: port 'nextState' has an incompatible format");
}

bool Rk4Box::propagatePortFormat()
{
    PortFormat rhsfmt = m_rhs.format();
    if (!rhsfmt.isValid())
        return false;
    PortFormat stfmt = rhsfmt;
    stfmt.setSize(0, rhsfmt.size(0)+1);
    m_initState.format() = stfmt;
    m_rhsState.format() = stfmt;
    m_nextState.format() = stfmt;
    return true;
}

RuntimeBox *Rk4Box::newRuntimeBox() const {
    return new Rk4RuntimeBox(this);
}



Rk4RuntimeBox::Rk4RuntimeBox(const Rk4Box *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_param.init(this, in[0]);
    m_initState.init(this, in[1], toPortNotifier(&Rk4RuntimeBox::start));
    m_rhs.init(this, in[2]);
    m_terminator.init(this, in[3], toPortNotifier(&Rk4RuntimeBox::stop));
    setInputPorts(RuntimeInputPorts() << &m_param << &m_initState << &m_rhs << &m_terminator);

    m_n = in[2]->format().size(0);
    m_nextData.resize(m_n+1);
    m_k[0].resize(m_n);
    m_k[1].resize(m_n);
    m_k[2].resize(m_n);
    m_k[3].resize(m_n);
    m_buf.resize(m_n+1);

    OutputPorts out = box->outputPorts();
    m_nextState.init(this, out[0], PortData(m_n+1, m_nextData.data()));
    m_rhsState.init(this, out[1], PortData(m_n+1, m_k[0].data()));
    m_finish.init(this, out[2]);
    setOutputPorts(RuntimeOutputPorts() << &m_nextState << &m_rhsState << &m_finish);
}

bool Rk4RuntimeBox::start()
{
    RuntimeOutputPort
        *paramPort = m_param.outputPort(),
        *initPort = m_initState.outputPort();
    if (!paramPort->state().hasData())
        return false;
    if (!initPort->state().hasData())
        return false;

    const double *param = m_param.data().data();
    double h = param[0],   h2 = 0.5*h,   h6 = h/6;
    int nSteps = static_cast<int>(param[1]);
    int outputControl = static_cast<int>(param[2]);
    if (nSteps == 0)
        nSteps = std::numeric_limits<int>::max();
    PortData initData = m_initState.data();
    double t = initData.data()[m_n];
    int nStepsBetweenRunnerTerminationCheck = std::max(1, 500 / m_n);

    // Check that rhs can be computed
    m_rhsState.setData(initPort->data());
    m_rhsState.state().setValid();
    m_rhs.setData(PortData(m_n+1, m_k[0].data()));
    m_rhs.state().invalidate();
    if (!m_rhsState.activateLinks())
        // Something is missing for computing rhs
        return false;
    if (!m_rhs.state().hasData())
        throwBoxException("Rk4RuntimeBox::start() failed: probably wrong connections of ODE box");

    double *nextData = m_nextState.data().data();
    const double *k1 = m_k[0].data(),   *k2 = m_k[1].data(),   *k3 = m_k[2].data(),   *k4 = m_k[3].data();

    m_terminationRequested = false;

    int outputCounter = outputControl > 0 ?   outputControl :   nSteps;
    int runnerTermCheckCounter = nStepsBetweenRunnerTerminationCheck;
    for (int step=0; step<nSteps; ++step) {
        m_rhsState.setData(initData);
        m_rhsState.state().setValid();
        m_rhs.setData(PortData(m_n+1, m_k[0].data()));
        m_rhsState.activateLinks();

        lincomb(m_buf.data(), initData.data(), m_k[0].data(), h2);
        m_buf[m_n] = t + h2;
        m_rhsState.setData(PortData(m_n+1, m_buf.data()));
        m_rhs.setData(PortData(m_n+1, m_k[1].data()));
        m_rhsState.activateLinks();

        lincomb(m_buf.data(), initData.data(), m_k[1].data(), h2);
        m_rhs.setData(PortData(m_n+1, m_k[2].data()));
        m_rhsState.activateLinks();

        lincomb(m_buf.data(), initData.data(), m_k[2].data(), h);
        m_buf[m_n] = t + h;
        m_rhs.setData(PortData(m_n+1, m_k[3].data()));
        m_rhsState.activateLinks();

        for (int i=0; i<m_n; ++i)
            nextData[i] = initData.data()[i] + h6*(k1[i] + 2*(k2[i] + k3[i]) + k4[i]);
        nextData[m_n] = initData.data()[m_n] + h;   // Time
        if (--outputCounter == 0) {
            m_nextState.state().setValid();
            if (!m_nextState.activateLinks())
                return false;
            outputCounter = outputControl;
        }
        initData = m_nextState.data();

        // Check termination request from the terminator port
        if (m_terminationRequested)
            break;

        // Check termination request for the runner
        if (--runnerTermCheckCounter == 0) {
            if (runner()->terminationRequested())
                break;
            runnerTermCheckCounter = nStepsBetweenRunnerTerminationCheck;
        }
    }
    return m_finish.activateLinks();
}

bool Rk4RuntimeBox::stop() {
    m_terminationRequested = true;
    return true;
}

void Rk4RuntimeBox::lincomb(double *dst, const double *src1, const double *src2, double factor2) const
{
    for (int i=0; i<m_n; ++i)
        dst[i] = src1[i] + src2[i]*factor2;
}
