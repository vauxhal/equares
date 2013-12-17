#ifndef ODEBOX_H
#define ODEBOX_H

#include "equares_core.h"

template<class OdeTraits>
class OdeBox :
    public Box
{
public:
    explicit OdeBox(QObject *parent = 0) :
        Box(parent),
        m_param("parameters", this, PortFormat(OdeTraits::ParamCount).setFixed()),
        m_state("state", this, PortFormat(OdeTraits::VarCount+1).setFixed()),
        m_rhs("oderhs", this, PortFormat(OdeTraits::VarCount).setFixed())
        {}

    // Box interface
    InputPorts inputPorts() const {
        return InputPorts() << &m_param << &m_state;
    }
    OutputPorts outputPorts() const {
        return OutputPorts() << &m_rhs;
    }
    void checkPortFormat() const {
        if (m_param.format() != PortFormat(OdeTraits::ParamCount))
            throw EquaresException("OdeBox: port 'parameters' has an invalid size");
        if (m_state.format() != PortFormat(OdeTraits::VarCount+1))
            throw EquaresException("OdeBox: port 'state' has an invalid size");
        if (m_rhs.format() != PortFormat(OdeTraits::VarCount))
            throw EquaresException("OdeBox: port 'rhs' has an invalid size");
    }
    bool propagatePortFormat() {
        return false;
    }
    inline RuntimeBox *newRuntimeBox() const;

private:
    mutable InputPort m_param;
    mutable InputPort m_state;
    mutable OutputPort m_rhs;
};

template<class OdeTraits>
class OdeRuntimeBox : public RuntimeBox
{
public:
    explicit OdeRuntimeBox(const OdeBox<OdeTraits> *box) {
        setOwner(box);

        InputPorts in = box->inputPorts();
        m_param.init(this, in[0], toPortNotifier(&OdeRuntimeBox::setParameters));
        m_state.init(this, in[1], toPortNotifier(&OdeRuntimeBox::setState));
        setInputPorts(RuntimeInputPorts() << &m_param << &m_state);

        OutputPorts out = box->outputPorts();
        m_rhs.init(this, out[0], PortData(2, m_rhsData));
        setOutputPorts(RuntimeOutputPorts() << &m_rhs);

        m_paramData = 0;
    }

private:
    RuntimeInputPort m_param;
    RuntimeInputPort m_state;
    RuntimeOutputPort m_rhs;
    double m_rhsData[OdeTraits::VarCount];
    const double *m_paramData;
    OdeTraits m_odeTraits;

    bool setParameters() {
        RuntimeOutputPort *paramPort = m_param.outputPort();
        Q_ASSERT(paramPort->state().hasData());
        m_paramData = paramPort->data().data();
        m_odeTraits.computeAuxParam(m_paramData);
        return true;
    }

    bool setState() {
        if (!m_paramData)
            return false;
        RuntimeOutputPort *statePort = m_state.outputPort();
        Q_ASSERT(statePort->state().hasData());
        const double *state = statePort->data().data();
        double *out = m_rhs.data().data();
        m_odeTraits.rhs(out, m_paramData, state);
        m_rhs.state().setValid();
        return m_rhs.activateLinks();
    }
};

template<class OdeTraits>
inline RuntimeBox *OdeBox<OdeTraits>::newRuntimeBox() const {
    return new OdeRuntimeBox<OdeTraits>(this);
}

#endif // ODEBOX_H
