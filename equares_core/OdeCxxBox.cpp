#include "OdeCxxBox.h"
#include <QFile>

REGISTER_BOX(OdeCxxBox, "CxxOde")

OdeCxxBox::OdeCxxBox(QObject *parent) :
    Box(parent),
    m_param("parameters", this, PortFormat(0).setFixed()),
    m_state("state", this, PortFormat(0).setFixed()),
    m_rhs("oderhs", this, PortFormat(0).setFixed())
{
}

InputPorts OdeCxxBox::inputPorts() const {
    return InputPorts() << &m_param << &m_state;
}

OutputPorts OdeCxxBox::outputPorts() const {
    return OutputPorts() << &m_rhs;
}

void OdeCxxBox::checkPortFormat() const {
    if (m_param.format() != PortFormat(paramCount()))
        throw EquaresException("OdeCxxBox: port 'parameters' has an invalid size");
    if (m_state.format() != PortFormat(varCount()+1))
        throw EquaresException("OdeCxxBox: port 'state' has an invalid size");
    if (m_rhs.format() != PortFormat(varCount()))
        throw EquaresException("OdeCxxBox: port 'rhs' has an invalid size");
}

bool OdeCxxBox::propagatePortFormat() {
    return false;
}

RuntimeBox *OdeCxxBox::newRuntimeBox() const {
    return new OdeCxxRuntimeBox(this);
}

QString OdeCxxBox::src() const {
    return m_src;
}

OdeCxxBox& OdeCxxBox::setSrc(const QString& src) {
    // Check source file: no #include


    m_src = src;

//    QFile sour

//    m_param.format().setSize(paramCount());
//    m_state.format().setSize(varCount()+1);
//    m_rhs.format().setSize(varCount());
    return *this;
}

QString OdeCxxBox::srcExample() const
{
    QFile file(":/cxx/OdeClass.cpp");
    file.open(QIODevice::ReadOnly);
    return QString::fromUtf8(file.readAll());
}

int OdeCxxBox::paramCount() const {
    throw EquaresException("TODO: OdeCxxBox::paramCount()");
}

int OdeCxxBox::varCount() const {
    throw EquaresException("TODO: OdeCxxBox::varCount()");
}



OdeCxxRuntimeBox::OdeCxxRuntimeBox(const OdeCxxBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_param.init(this, in[0], toPortNotifier(&OdeCxxRuntimeBox::setParameters));
    m_state.init(this, in[1], toPortNotifier(&OdeCxxRuntimeBox::setState));
    setInputPorts(RuntimeInputPorts() << &m_param << &m_state);

    OutputPorts out = box->outputPorts();
    m_rhsData.resize(out[0]->format().dataSize());
    m_rhs.init(this, out[0], PortData(2, m_rhsData.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_rhs);

    m_hasParamData = false;
    m_paramData.resize(in[0]->format().dataSize());
    m_stateData.resize(in[1]->format().dataSize());

    m_src = box->src();
}

bool OdeCxxRuntimeBox::setParameters() {
    RuntimeOutputPort *paramPort = m_param.outputPort();
    Q_ASSERT(paramPort->state().hasData());
    paramPort->data().copyTo(m_paramData.data());
    m_hasParamData = true;
    throw EquaresException("TODO: OdeCxxBox::setParameters()");
//    if (m_hasPrepareFunc) {
//        QScriptValue paramArg = CustomDoubleArrayClass::toScriptValue(m_engine, m_paramData);
//        m_argsForPrepare.setProperty(0, paramArg);
//        m_prepareFunc.call(m_ode, m_argsForPrepare);
//    }
    return true;
}

bool OdeCxxRuntimeBox::setState() {
    if (!m_hasParamData)
        return false;
    throw EquaresException("TODO: OdeCxxBox::setState()");
    RuntimeOutputPort *statePort = m_state.outputPort();
    Q_ASSERT(statePort->state().hasData());
    if (m_stateData.data() != statePort->data().data())
        statePort->data().copyTo(m_stateData.data());
//    m_rhs.data().copyFrom(m_rhsData.data());
    m_rhs.state().setValid();
    return m_rhs.activateLinks();
}
