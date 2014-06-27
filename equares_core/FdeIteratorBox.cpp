#include "FdeIteratorBox.h"
#include "box_util.h"
#include <limits>

REGISTER_BOX(FdeIteratorBox, "FdeIterator")

FdeIteratorBox::FdeIteratorBox(QObject *parent) :
    Box(parent),
    m_param("parameters", this, PortFormat(3).setFixed()),  // h, n, nOut
    m_initState("initState", this),
    m_fdeIn("fdeIn", this),
    m_terminator("stop", this),
    m_nextState("nextState", this),
    m_fdeOut("fdeOut", this),
    m_finish("finish", this, PortFormat(0).setFixed())
{
}

InputPorts FdeIteratorBox::inputPorts() const {
    return InputPorts() << &m_param << &m_initState << &m_fdeIn << &m_terminator;
}

OutputPorts FdeIteratorBox::outputPorts() const {
    return OutputPorts() << &m_nextState << &m_fdeOut << &m_finish;
}

void FdeIteratorBox::checkPortFormat() const
{
    if (m_param.format() != PortFormat(3))
        throwBoxException("FdeIteratorBox: port 'parameters' has an invalid size");
    PortFormat fdefmt = m_fdeIn.format();
    if (!fdefmt.isValid())
        throwBoxException("FdeIteratorBox: no format is specified for port 'fdeIn'");
    if (fdefmt.dimension() != 1)
        throwBoxException("FdeIteratorBox: port 'fdeIn' has an invalid dimension");
    if (fdefmt.size(0) == 0)
        throwBoxException("FdeIteratorBox: port 'fdeIn' has an empty length");
    if (m_initState.format() != fdefmt)
        throwBoxException("FdeIteratorBox: port 'initState' has an incompatible format");
    if (m_fdeOut.format() != fdefmt)
        throwBoxException("FdeIteratorBox: port 'fdeOut' has an incompatible format");
    if (m_nextState.format() != fdefmt)
        throwBoxException("FdeIteratorBox: port 'nextState' has an incompatible format");
}

bool FdeIteratorBox::propagatePortFormat()
{
    PortFormat fdefmt = m_fdeIn.format();
    if (!fdefmt.isValid())
        return false;
    bool result = false;
    result = specifyFormat(m_initState.format(), fdefmt) || result;
    result = specifyFormat(m_fdeOut.format(), fdefmt) || result;
    result = specifyFormat(m_nextState.format(), fdefmt) || result;
    return result;
}

RuntimeBox *FdeIteratorBox::newRuntimeBox() const {
    return new FdeIteratorRuntimeBox(this);
}



FdeIteratorRuntimeBox::FdeIteratorRuntimeBox(const FdeIteratorBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_param.init(this, in[0]);
    m_initState.init(this, in[1], toPortNotifier(&FdeIteratorRuntimeBox::start));
    m_fdeIn.init(this, in[2]);
    m_terminator.init(this, in[3], toPortNotifier(&FdeIteratorRuntimeBox::stop));
    setInputPorts(RuntimeInputPorts() << &m_param << &m_initState << &m_fdeIn << &m_terminator);

    m_n = in[2]->format().size(0);
    m_buf.resize(m_n);

    OutputPorts out = box->outputPorts();
    m_nextState.init(this, out[0]);
    m_fdeOut.init(this, out[1]);
    m_finish.init(this, out[2]);
    setOutputPorts(RuntimeOutputPorts() << &m_nextState << &m_fdeOut << &m_finish);
}

bool FdeIteratorRuntimeBox::start(int)
{
    if (!m_param.state().hasData())
        return false;
    if (!m_initState.state().hasData())
        return false;

    const double *param = m_param.data().data();
    int nSteps = static_cast<int>(param[0]),
        nSkip = static_cast<int>(param[1]),
        nOut = static_cast<int>(param[2]);
    if (nSteps == 0)
        nSteps = std::numeric_limits<int>::max();
    int nStepsBetweenRunnerTerminationCheck = std::max(1, 500 / m_n);

    // Check that next state can be computed
    m_fdeOut.setData(m_initState.data());
    m_fdeOut.state().setValid();
    m_fdeIn.state().invalidate();
    if (!m_fdeOut.activateLinks())
        // Something is missing for computing next state
        return false;
    if (!m_fdeIn.state().hasData())
        throwBoxException("FdeIteratorRuntimeBox::start() failed: probably wrong connections of FDE box");

    m_initState.data().copyTo(m_buf.data());
    m_fdeOut.setData(PortData(m_n, m_buf.data()));
    m_fdeOut.state().setValid();

    m_terminationRequested = false;

    int outputCounter = nOut > 0 ?   nOut :   nSteps-nSkip;
    int runnerTermCheckCounter = nStepsBetweenRunnerTerminationCheck;
    for (int step=0; step<nSteps; ++step) {
        m_fdeOut.activateLinks();
        PortData nextState = m_fdeIn.data();
        if (step >= nSkip) {
            if (--outputCounter == 0) {
                m_nextState.setData(nextState);
                m_nextState.state().setValid();
                if (!m_nextState.activateLinks())
                    return false;
                outputCounter = nOut;
            }
        }
        PortData curState = m_fdeOut.data();
        m_fdeIn.setData(curState);
        m_fdeOut.setData(nextState);

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

bool FdeIteratorRuntimeBox::stop(int) {
    m_terminationRequested = true;
    return true;
}
