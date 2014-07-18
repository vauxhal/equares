/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "OdeCxxBox.h"

REGISTER_BOX(OdeCxxBox, "CxxOde")

OdeCxxBox::OdeCxxBox(QObject *parent) :
    Box(parent),
    m_param("parameters", this, PortFormat(0).setFixed()),
    m_state("state", this, PortFormat(0).setFixed()),
    m_rhs("oderhs", this, PortFormat(0).setFixed()),
    m_useQmake(true)
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
        throwBoxException("OdeCxxBox: port 'parameters' has an invalid size");
    if (m_state.format() != PortFormat(varCount()+1))
        throwBoxException("OdeCxxBox: port 'state' has an invalid size");
    if (m_rhs.format() != PortFormat(varCount()))
        throwBoxException("OdeCxxBox: port 'rhs' has an invalid size");
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

#define JSX_BEGIN try {

#define JSX_END } catch(const std::exception& e) { \
    context()->throwError(QString::fromUtf8(e.what())); \
    }

OdeCxxBox& OdeCxxBox::setSrc(const QString& src)
{
    JSX_BEGIN

    // Find/build & check & Load library
    m_libProxy = OdeLibProxy(
        CxxBuildHelper(this, src, "ode", ":/cxx/OdeFileFooter.cpp", m_useQmake).library(),
        this);

    // Assign source property
    m_src = src;

    // Update port formats
    m_param.format().setSize(paramCount());
    m_param.hints().setEntryHints(paramNames());
    m_state.format().setSize(varCount()+1);
    {
        QStringList stateHints = varNames();
        if (stateHints.size() == varCount())
            stateHints << "t";
        m_state.hints().setEntryHints(stateHints);
    }
    m_rhs.format().setSize(varCount());

    return *this;

    JSX_END
    return *this;
}

QString OdeCxxBox::buildDir(const QScriptValue &boxProps) const {
    return CxxBuildHelper(this, boxProps.property("src").toString(), "ode", ":/cxx/OdeFileFooter.cpp", m_useQmake).buildDirName();
}

QString OdeCxxBox::srcExample() const {
    return readFile(":/cxx/OdeClass.cpp", this);
}

bool OdeCxxBox::useQmake() const {
    return m_useQmake;
}

OdeCxxBox& OdeCxxBox::setUseQmake(bool useQmake) {
    m_useQmake = useQmake;
    return *this;
}

int OdeCxxBox::paramCount() const {
    if (m_libProxy.isNull())
        throwBoxException("OdeCxxBox::paramCount: No source is specified");
    return m_libProxy.paramCount();
}

QStringList OdeCxxBox::paramNames() const
{
    if (m_libProxy.isNull())
        throwBoxException("OdeCxxBox::paramNames: No source is specified");
    return m_libProxy.paramNames();
}

int OdeCxxBox::varCount() const {
    if (m_libProxy.isNull())
        throwBoxException("OdeCxxBox::varCount: No source is specified");
    return m_libProxy.varCount();
}

QStringList OdeCxxBox::varNames() const
{
    if (m_libProxy.isNull())
        throwBoxException("OdeCxxBox::varNames: No source is specified");
    return m_libProxy.varNames();
}

const OdeCxxBox::OdeLibProxy *OdeCxxBox::odeLibProxy() const {
    return &m_libProxy;
}

OdeCxxBox::OdeLibProxy::OdeLibProxy(const QLibraryPtr& lib, const Box *box) :
    m_lib(lib),
    m_box(box)
{
    init();
}

OdeCxxBox::OdeLibProxy::OdeLibProxy(const OdeLibProxy& arg) :
    m_lib(arg.m_lib),
    m_box(arg.m_box)
{
    init();
}

OdeCxxBox::OdeLibProxy& OdeCxxBox::OdeLibProxy::operator=(const OdeLibProxy& arg)
{
    if (&arg != this) {
        m_box = arg.m_box;
        m_lib = arg.m_lib;
        init();
    }
    return *this;
}

void OdeCxxBox::OdeLibProxy::init()
{
    Q_ASSERT(m_lib->isLoaded());
#define RESOLVE_SYMBOL(name) \
    m_##name = reinterpret_cast<name##Func>(m_lib->resolve(#name)); \
    if (!m_##name) \
        throw EquaresBoxException(m_box, QString("Failed to resolve symbol '%1' in library '%2'").arg(#name, m_lib->fileName()));
    RESOLVE_SYMBOL(newInstance)
    RESOLVE_SYMBOL(deleteInstance)
    RESOLVE_SYMBOL(paramCount)
    RESOLVE_SYMBOL(paramNames)
    RESOLVE_SYMBOL(varCount)
    RESOLVE_SYMBOL(varNames)
    RESOLVE_SYMBOL(prepare)
    RESOLVE_SYMBOL(rhs)
    RESOLVE_SYMBOL(hash)
#undef RESOLVE_SYMBOL
    m_inst = m_newInstance();
}

OdeCxxBox::OdeLibProxy::~OdeLibProxy() {
    if (!m_lib.isNull())
        m_deleteInstance(m_inst);
}



OdeCxxRuntimeBox::OdeCxxRuntimeBox(const OdeCxxBox *box) :
    m_odeLibProxy(box->odeLibProxy())
{
    if (!m_odeLibProxy)
        throwBoxException("OdeCxxRuntimeBox: No ODE source is currently set");

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
}

bool OdeCxxRuntimeBox::setParameters(int)
{
    RuntimeOutputPort *paramPort = m_param.outputPort();
    Q_ASSERT(paramPort->state().hasData());
    paramPort->data().copyTo(m_paramData.data());
    m_hasParamData = true;
    m_odeLibProxy->prepare(m_paramData.data());
    return true;
}

bool OdeCxxRuntimeBox::setState(int)
{
    if (!m_hasParamData)
        return false;
    RuntimeOutputPort *statePort = m_state.outputPort();
    Q_ASSERT(statePort->state().hasData());
    m_odeLibProxy->rhs(m_rhs.data().data(), m_paramData.data(), statePort->data().data());
    m_rhs.state().setValid();
    return m_rhs.activateLinks();
}
