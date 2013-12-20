#include "OdeCxxBox.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QProcess>
#include <QCryptographicHash>
#include <QCoreApplication>

#include <iostream>

#ifdef _WIN32
#define PLATFORM WINDOWS

static void buildLibNoQmake(const QDir& dir)
{
    QString buildFileName = "build_msvs.bat";
    QFileInfo fi(dir.absoluteFilePath(buildFileName));
    if (!fi.exists()) {
        if (!QFile::copy(":/cxx/"+buildFileName, fi.absoluteFilePath()))
            throw EquaresException(QString("Failed to copy library build file to '%1'")
                .arg(fi.absoluteFilePath()));
    }
    QProcess proc;
    QString cmdline = "cmd /C " + buildFileName + " ode.cpp";
    proc.setWorkingDirectory(dir.absolutePath());
    proc.start(cmdline);
    if (!proc.waitForFinished())
        throw EquaresException("Failed to build dll-file (timed out)");
    if (proc.exitCode() != 0)
        throw EquaresException(QString("Failed to build library file:\n%1").arg(
            QString::fromUtf8(proc.readAllStandardError())));
    Q_ASSERT(QFileInfo(dir.absoluteFilePath("ode.dll")).exists());
}

#endif // _WIN32

#ifdef __linux__
#define PLATFORM LINUX

static void buildLibNoQmake(const QDir& dir)
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

#endif // __linux__

#ifndef PLATFORM
#error "Unknown platform"
#endif // !PLATFORM

static QString readFile(const QString& fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        throw EquaresException(QString("readFile(): failed to open file %1").arg(fileName));
    return QString::fromUtf8(file.readAll());
}

static void buildLibWithQmake(const QDir& dir)
{
    QFileInfo fiPro(dir.absoluteFilePath("ode.pro"));
    if (!fiPro.exists()) {
        if (!QFile::copy(":/cxx/ode.pro", fiPro.absoluteFilePath()))
            throw EquaresException(QString("Failed to copy qmake project file to '%1'")
                .arg(fiPro.absoluteFilePath()));
    }
    QProcess proc;
    QStringList env = QProcess::systemEnvironment();
    QDir appDir(QCoreApplication::applicationDirPath());
    QString buildTimePath = readFile(appDir.absoluteFilePath("buildpath.txt"));
    buildTimePath.remove('\r').remove('\n');
    QString buildTimeInclude = readFile(appDir.absoluteFilePath("buildinclude.txt"));
    buildTimeInclude.remove('\r').remove('\n');
    QString buildTimeLib = readFile(appDir.absoluteFilePath("buildlib.txt"));
    buildTimeLib.remove('\r').remove('\n');
    QString makeCmd = readFile(appDir.absoluteFilePath("makecmd.txt"));
    makeCmd.remove('\r').remove('\n');
#ifdef _WIN32
    // Because it might happen that makeCmd is not found in the path
    // for this process
    makeCmd.prepend("cmd /C ");
#endif // _WIN32
    QRegExp rx("^(PATH|INCLUDE|LIB)=");
    for (int i=0; i<env.size();) {
        if (rx.indexIn(env[i]) != -1)
            env.removeAt(i);
        else
            ++i;
    }
    env << "PATH="+buildTimePath
        << "INCLUDE="+buildTimeInclude
        << "LIB="+buildTimeLib;
    proc.setWorkingDirectory(dir.absolutePath());
    proc.setEnvironment(env);
    proc.start("qmake ode.pro");
    if (!proc.waitForFinished())
        throw EquaresException("Failed to generate Makefile (timed out)");
    if (proc.exitCode() != 0)
        throw EquaresException(QString("Failed to generate Makefile (qmake returned %1):\n%2").arg(
            QString::number(proc.exitCode()),
            QString::fromUtf8(proc.readAllStandardError())));

    proc.start(makeCmd);
    if (!proc.waitForFinished())
        throw EquaresException("Failed to build library (timed out)");

    if (proc.exitCode() != 0) {
        using namespace std;
        cout << proc.readAllStandardOutput().constData() << endl;
        throw EquaresException(QString("Failed to build library :\n%1").arg(
            QString::fromUtf8(proc.readAllStandardError())));
    }
}

static void buildLib(const QDir& dir, bool withQmake)
{
    if (withQmake)
        buildLibWithQmake(dir);
    else
        buildLibNoQmake(dir);
}

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

BoxPropertyList OdeCxxBox::boxProperties() const {
    return BoxPropertyList()
        << BoxProperty("src", tr("Source code of the C++ class that describes the ODE system. See srcExample property"))
        << BoxProperty("srcExample", tr("Example of source code of the C++ class that describes the ODE system for simple pendulum"))
        << BoxProperty("useQmake", tr("Determine whether to use qmake when building the library file"));
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

#define JSX_BEGIN try {

#define JSX_END } catch(const std::exception& e) { \
    context()->throwError(QString::fromUtf8(e.what())); \
    }

OdeCxxBox& OdeCxxBox::setSrc(const QString& src)
{
    JSX_BEGIN

    // Check source file: no #include
    checkSrc(src);

    // Extract class name that will also be part of directory name
    QString className;
    {
        QRegExp rx("^\\s*struct\\s+([a-zA-Z_][a-zA-Z0-9_]*)");
        foreach (const QString& line, src.split('\n'))
            if (rx.indexIn(line) != -1) {
                className = rx.capturedTexts()[1];
                Q_ASSERT(!className.isEmpty());
                break;
            }
        if (className.isEmpty())
            throw EquaresException("Failed to set source: unable to retrieve struct name");
    }

    // Generate md5 checksum
    QString hashString;
    {
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(src.toUtf8());
        const char *xdigits = "0123456789abcdef";
        foreach(unsigned char c, hash.result()) {
            hashString += xdigits[(c>>4) & 0xf];
            hashString += xdigits[c & 0xf];
        }
    }

    // Prepare work directory
    QDir dir = QDir::current();
    QString subdirPath = "equares/" + className + "_" + hashString;
    if (!dir.mkpath(subdirPath))
        throw EquaresException(QString("Failed to create directory %1").arg(dir.absoluteFilePath(subdirPath)));
    if (!dir.cd(subdirPath))
        throw EquaresException(QString("Failed to switch to directory %1").arg(dir.absoluteFilePath(subdirPath)));

    QString baseName = "ode";

    if (!libUpToDate(dir.absoluteFilePath(baseName), hashString)) {
        // Generate source file
        QFileInfo fi(dir.absoluteFilePath(baseName + ".cpp"));
        {
            QString srcFileContent =
                readFile(":/cxx/OdeFileHeader.cpp") + "\n" +
                src + "\n" +
                readFile(":/cxx/OdeFileFooter.cpp").replace("<X>", className) + "\n" +
                    "extern \"C\" ODE_EXPORT const char *hash() { return \"" + hashString + "\"; }\n";
            QFile srcFile(fi.absoluteFilePath());
            if (!srcFile.open(QIODevice::WriteOnly))
                throw EquaresException(QString("Failed to open file %1").arg(fi.absoluteFilePath()));
            srcFile.write(srcFileContent.toUtf8());
        }

        // Build library
        buildLib(dir, m_useQmake);
    }

    // Load library
    QString libName = dir.absoluteFilePath(baseName);
    m_libProxy.clear();
    m_libProxy = OdeLibProxy::Ptr(new OdeLibProxy(libName));

    // Assign source property
    m_src = src;

    // Update port formats
    m_param.format().setSize(paramCount());
    m_state.format().setSize(varCount()+1);
    m_rhs.format().setSize(varCount());

    return *this;

    JSX_END
    return *this;
}

QString OdeCxxBox::srcExample() const {
    return readFile(":/cxx/OdeClass.cpp");
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
    RESOLVE_SYMBOL(hash)
#undef RESOLVE_SYMBOL
    m_inst = m_newInstance();
}

OdeCxxBox::OdeLibProxy::~OdeLibProxy() {
    m_deleteInstance(m_inst);
}



void OdeCxxBox::checkSrc(const QString& src) {
    // TODO
    Q_UNUSED(src);
}

bool OdeCxxBox::libUpToDate(const QString &libName, const QString& hashString)
{
    try {
        OdeLibProxy lib(libName);
        return lib.hash() == hashString;
    }
    catch (const EquaresException&) {
        return false;
    }
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
