#include "OdeCxxBox.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QProcess>

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

OdeCxxBox& OdeCxxBox::setSrc(const QString& src)
{
    // Check source file: no #include
    checkSrc(src);

    // Prepare work directory
    QRegExp rxName("^[-\\w]+$");
    if (!rxName.exactMatch(name()))
        throw EquaresException("Failed to set source: specify a valid box name first");
    QDir dir = QDir::current();
    QString subdirPath = "equares/" + name();
    if (!dir.mkpath(subdirPath))
        throw EquaresException(QString("Failed to create directory %1").arg(dir.absoluteFilePath(subdirPath)));
    if (!dir.cd(subdirPath))
        throw EquaresException(QString("Failed to switch to directory %1").arg(dir.absoluteFilePath(subdirPath)));

    // Generate source file
    QFileInfo fi(dir.absoluteFilePath("ode.cpp"));
    {
        QString srcFileContent =
            readFile(":/cxx/OdeFileHeader.cpp") + "\n" +
            src + "\n" +
            readFile(":/cxx/OdeFileFooter.cpp") + "\n";
        QFile srcFile(fi.absoluteFilePath());
        if (!srcFile.open(QIODevice::WriteOnly))
            throw EquaresException(QString("Failed to open file %1").arg(fi.absoluteFilePath()));
        srcFile.write(srcFileContent.toUtf8());
    }

    // Build library
    {
        QString cmdline = "g++ -Wall -shared -Wl,-soname,ode.so -fPIC -O2 -o ode.so ode.cpp";
        QProcess proc;
        proc.setWorkingDirectory(dir.absolutePath());
        proc.start(cmdline);
        if (!proc.waitForFinished())
            throw EquaresException("Failed to build so-file (timed out)");
        if (proc.exitCode() != 0)
            throw EquaresException(QString("Failed to build library file:\n%1").arg(
                QString::fromUtf8(proc.readAllStandardError())));
        Q_ASSERT(QFileInfo(dir.absoluteFilePath("ode.so")).exists());
    }

    // Load library
    QString libName = dir.absoluteFilePath(fi.baseName());
    m_libProxy.clear();
    m_libProxy = OdeLibProxy::Ptr(new OdeLibProxy(libName));

    // Assign source property
    m_src = src;

    // Update port formats
    m_param.format().setSize(paramCount());
    m_state.format().setSize(varCount()+1);
    m_rhs.format().setSize(varCount());

    return *this;
}

QString OdeCxxBox::srcExample() const {
    return readFile(":/cxx/OdeClass.cpp");
}

int OdeCxxBox::paramCount() const {
    if (m_libProxy.isNull())
        throw EquaresException("OdeCxxBox::paramCount: No source is specified");
    return m_libProxy->paramCount();
}

int OdeCxxBox::varCount() const {
    if (m_libProxy.isNull())
        throw EquaresException("OdeCxxBox::varCount: No source is specified");
    return m_libProxy->varCount();
}

const OdeCxxBox::OdeLibProxy *OdeCxxBox::odeLibProxy() const {
    return m_libProxy.data();
}

OdeCxxBox::OdeLibProxy::OdeLibProxy(const QString& libName) :
    m_lib(libName)
{
    m_lib.load();
    if (!m_lib.isLoaded())
        throw EquaresException(QString("Failed to open library file '%1'").arg(libName));

#define RESOLVE_SYMBOL(name) \
    m_##name = reinterpret_cast<name##Func>(m_lib.resolve(#name)); \
    if (!m_##name) \
        throw EquaresException(QString("Failed to resolve symbol '%1' in library '%1'").arg(#name, libName));
    RESOLVE_SYMBOL(newInstance)
    RESOLVE_SYMBOL(deleteInstance)
    RESOLVE_SYMBOL(paramCount)
    RESOLVE_SYMBOL(varCount)
    RESOLVE_SYMBOL(prepare)
    RESOLVE_SYMBOL(rhs)
#undef RESOLVE_SYMBOL
    m_inst = m_newInstance();
}

OdeCxxBox::OdeLibProxy::~OdeLibProxy() {
    m_deleteInstance(m_inst);
}



QString OdeCxxBox::readFile(const QString& fileName) {
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        throw EquaresException(QString("OdeCxxBox::readFile(): failed to open file %1").arg(fileName));
    return QString::fromUtf8(file.readAll());
}

void OdeCxxBox::checkSrc(const QString& src) {
    // TODO
    Q_UNUSED(src);
}



OdeCxxRuntimeBox::OdeCxxRuntimeBox(const OdeCxxBox *box) :
    m_odeLibProxy(box->odeLibProxy())
{
    if (!m_odeLibProxy)
        throw EquaresException("OdeCxxRuntimeBox: No ODE source is currently set");

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

bool OdeCxxRuntimeBox::setParameters()
{
    RuntimeOutputPort *paramPort = m_param.outputPort();
    Q_ASSERT(paramPort->state().hasData());
    paramPort->data().copyTo(m_paramData.data());
    m_hasParamData = true;
    m_odeLibProxy->prepare(m_paramData.data());
    return true;
}

bool OdeCxxRuntimeBox::setState()
{
    if (!m_hasParamData)
        return false;
    RuntimeOutputPort *statePort = m_state.outputPort();
    Q_ASSERT(statePort->state().hasData());
    m_odeLibProxy->rhs(m_rhs.data().data(), m_paramData.data(), statePort->data().data());
    m_rhs.state().setValid();
    return m_rhs.activateLinks();
}
