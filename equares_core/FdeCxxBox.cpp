#include "FdeCxxBox.h"
#include "box_util.h"

REGISTER_BOX(FdeCxxBox, "CxxFde")

FdeCxxBox::FdeCxxBox(QObject *parent) :
    Box(parent),
    m_param("parameters", this, PortFormat(0).setFixed()),
    m_state("state", this, PortFormat(0).setFixed()),
    m_nextState("nextState", this, PortFormat(0).setFixed()),
    m_useQmake(true)
{
}

InputPorts FdeCxxBox::inputPorts() const {
    return InputPorts() << &m_param << &m_state;
}

OutputPorts FdeCxxBox::outputPorts() const {
    return OutputPorts() << &m_nextState;
}

void FdeCxxBox::checkPortFormat() const {
    if (m_param.format() != PortFormat(paramCount()))
        throwBoxException("FdeCxxBox: port 'parameters' has an invalid size");
    if (m_state.format() != PortFormat(varCount()))
        throwBoxException("FdeCxxBox: port 'state' has an invalid size");
    if (m_nextState.format() != PortFormat(varCount()))
        throwBoxException("FdeCxxBox: port 'nextState' has an invalid size");
}

bool FdeCxxBox::propagatePortFormat() {
    return false;
}

RuntimeBox *FdeCxxBox::newRuntimeBox() const {
    return new FdeCxxRuntimeBox(this);
}

QString FdeCxxBox::src() const {
    return m_src;
}

#define JSX_BEGIN try {

#define JSX_END } catch(const std::exception& e) { \
    context()->throwError(QString::fromUtf8(e.what())); \
    }

FdeCxxBox& FdeCxxBox::setSrc(const QString& src)
{
    JSX_BEGIN

    // Find/build & check & Load library
    m_libProxy = FdeLibProxy(
        CxxBuildHelper(this, src, "fde", ":/cxx/FdeFileFooter.cpp", m_useQmake).library(),
        this);

    // Assign source property
    m_src = src;

    // Update port formats
    m_param.format().setSize(paramCount());
    m_param.hints().setEntryHints(paramNames());
    m_state.format().setSize(varCount());
    m_state.hints().setEntryHints(varNames());
    m_nextState.format().setSize(varCount());
    m_nextState.hints().setEntryHints(varNames());

    return *this;

    JSX_END
    return *this;
}

QString FdeCxxBox::buildDir(const QScriptValue &boxProps) const {
    return CxxBuildHelper(this, boxProps.property("src").toString(), "fde", ":/cxx/FdeFileFooter.cpp", m_useQmake).buildDirName();
}

QString FdeCxxBox::srcExample() const {
    return readFile(":/cxx/FdeClass.cpp", this);
}

bool FdeCxxBox::useQmake() const {
    return m_useQmake;
}

FdeCxxBox& FdeCxxBox::setUseQmake(bool useQmake) {
    m_useQmake = useQmake;
    return *this;
}

int FdeCxxBox::paramCount() const {
    if (m_libProxy.isNull())
        throwBoxException("FdeCxxBox::paramCount: No source is specified");
    return m_libProxy.paramCount();
}

QStringList FdeCxxBox::paramNames() const
{
    if (m_libProxy.isNull())
        throwBoxException("FdeCxxBox::paramNames: No source is specified");
    return m_libProxy.paramNames();
}

int FdeCxxBox::varCount() const {
    if (m_libProxy.isNull())
        throwBoxException("FdeCxxBox::varCount: No source is specified");
    return m_libProxy.varCount();
}

QStringList FdeCxxBox::varNames() const
{
    if (m_libProxy.isNull())
        throwBoxException("FdeCxxBox::varNames: No source is specified");
    return m_libProxy.varNames();
}

const FdeCxxBox::FdeLibProxy *FdeCxxBox::fdeLibProxy() const {
    return &m_libProxy;
}

FdeCxxBox::FdeLibProxy::FdeLibProxy(const QLibraryPtr& lib, const Box *box) :
    m_lib(lib),
    m_box(box)
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
    RESOLVE_SYMBOL(nextState)
    RESOLVE_SYMBOL(hash)
#undef RESOLVE_SYMBOL
    m_inst = m_newInstance();
}

FdeCxxBox::FdeLibProxy::~FdeLibProxy() {
    if (!m_lib.isNull())
        m_deleteInstance(m_inst);
}



FdeCxxRuntimeBox::FdeCxxRuntimeBox(const FdeCxxBox *box) :
    m_fdeLibProxy(box->fdeLibProxy())
{
    if (!m_fdeLibProxy)
        throwBoxException("FdeCxxRuntimeBox: No FDE source is currently set");

    setOwner(box);

    InputPorts in = box->inputPorts();
    m_param.init(this, in[0], toPortNotifier(&FdeCxxRuntimeBox::setParameters));
    m_state.init(this, in[1], toPortNotifier(&FdeCxxRuntimeBox::setState));
    setInputPorts(RuntimeInputPorts() << &m_param << &m_state);

    OutputPorts out = box->outputPorts();
    m_nextStateData.resize(out[0]->format().dataSize());
    m_nextState.init(this, out[0], PortData(2, m_nextStateData.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_nextState);

    m_hasParamData = false;
    m_paramData.resize(in[0]->format().dataSize());
}

bool FdeCxxRuntimeBox::setParameters(int)
{
    RuntimeOutputPort *paramPort = m_param.outputPort();
    Q_ASSERT(paramPort->state().hasData());
    paramPort->data().copyTo(m_paramData.data());
    m_hasParamData = true;
    m_fdeLibProxy->prepare(m_paramData.data());
    return true;
}

bool FdeCxxRuntimeBox::setState(int)
{
    if (!m_hasParamData)
        return false;
    RuntimeOutputPort *statePort = m_state.outputPort();
    Q_ASSERT(statePort->state().hasData());
    m_fdeLibProxy->rhs(m_nextState.data().data(), m_paramData.data(), statePort->data().data());
    m_nextState.state().setValid();
    return m_nextState.activateLinks();
}
