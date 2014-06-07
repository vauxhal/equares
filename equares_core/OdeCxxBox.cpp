#include "OdeCxxBox.h"
#include "PerTypeStorage.h"
#include "check_lib.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QProcess>
#include <QCryptographicHash>
#include <QCoreApplication>

#include <iostream>

static void throwBoxException(const Box *box, const QString& what) {
    throw EquaresBoxException(box, what);
}

// http://stackoverflow.com/questions/11050977/removing-a-non-empty-folder-in-qt
static bool removeDir(const QString& dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir())
                result = removeDir(info.absoluteFilePath());
            else
                result = QFile::remove(info.absoluteFilePath());
            if (!result)
                return result;
        }
        result = dir.rmdir(dirName);
    }
    return result;
}

class DirRemover
{
public:
    explicit DirRemover(const QString& dirName) : m_dirName(dirName), m_cancel(false) {}
    ~DirRemover() {
        if (!m_cancel)
            removeDir(m_dirName);
    }
    void cancel() {
        m_cancel = true;
    }
private:
    QString m_dirName;
    bool m_cancel;
};

#ifdef _WIN32
#define PLATFORM WINDOWS

static void buildLibNoQmake(const QDir& dir, const Box *box)
{
    QString buildFileName = "build_msvs.bat";
    QFileInfo fi(dir.absoluteFilePath(buildFileName));
    if (!fi.exists()) {
        if (!QFile::copy(":/cxx/"+buildFileName, fi.absoluteFilePath()))
            throwBoxException(box, QString("Failed to copy library build file to '%1'")
                .arg(fi.absoluteFilePath()));
    }
    QProcess proc;
    QString cmdline = "cmd /C " + buildFileName + " ode.cpp";
    proc.setWorkingDirectory(dir.absolutePath());
    proc.start(cmdline);
    if (!proc.waitForFinished())
        throwBoxException(box, "Failed to build dll-file (timed out)");
    if (proc.exitCode() != 0)
        throwBoxException(box, QString("Failed to build library file:\n%1").arg(
            QString::fromUtf8(proc.readAllStandardError())));
    Q_ASSERT(QFileInfo(dir.absoluteFilePath("ode.dll")).exists());
}

#endif // _WIN32

#ifdef __linux__
#define PLATFORM LINUX

static void buildLibNoQmake(const QDir& dir, const Box *box)
{
    QString cmdline = "g++ -Wall -shared -Wl,-soname,ode.so -fPIC -O2 -o ode.so ode.cpp";
    QProcess proc;
    proc.setWorkingDirectory(dir.absolutePath());
    proc.start(cmdline);
    if (!proc.waitForFinished())
        throwBoxException(box, "Failed to build so-file (timed out)");
    if (proc.exitCode() != 0)
        throwBoxException(box, QString("Failed to build library file:\n%1").arg(
            QString::fromUtf8(proc.readAllStandardError())));
    Q_ASSERT(QFileInfo(dir.absoluteFilePath("ode.so")).exists());
}

#endif // __linux__

#ifndef PLATFORM
#error "Unknown platform"
#endif // !PLATFORM

static QString readFile(const QString& fileName, const Box *box)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        throwBoxException(box, QString("readFile(): failed to open file %1").arg(fileName));
    return QString::fromUtf8(file.readAll());
}

static void buildLibWithQmake(const QDir& dir, const Box *box)
{
    QFileInfo fiPro(dir.absoluteFilePath("ode.pro"));
    if (!fiPro.exists()) {
        if (!QFile::copy(":/cxx/ode.pro", fiPro.absoluteFilePath()))
            throwBoxException(box, QString("Failed to copy qmake project file to '%1'")
                .arg(fiPro.absoluteFilePath()));
    }
    QProcess proc;
    QStringList env = QProcess::systemEnvironment();
    QDir appDir(QCoreApplication::applicationDirPath());
    QString buildTimePath = readFile(appDir.absoluteFilePath("buildpath.txt"), box);
    buildTimePath.remove('\r').remove('\n');
    QString buildTimeInclude = readFile(appDir.absoluteFilePath("buildinclude.txt"), box);
    buildTimeInclude.remove('\r').remove('\n');
    QString buildTimeLib = readFile(appDir.absoluteFilePath("buildlib.txt"), box);
    buildTimeLib.remove('\r').remove('\n');
    QString makeCmd = readFile(appDir.absoluteFilePath("makecmd.txt"), box);
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
        throwBoxException(box, "Failed to generate Makefile (timed out)");
    if (proc.exitCode() != 0)
        throwBoxException(box, QString("Failed to generate Makefile (qmake returned %1):\n%2").arg(
            QString::number(proc.exitCode()),
            QString::fromUtf8(proc.readAllStandardError())));

    proc.start(makeCmd);
    if (!proc.waitForFinished())
        throwBoxException(box, "Failed to build library (timed out)");

    if (proc.exitCode() != 0) {
        EQUARES_COUT << proc.readAllStandardOutput() << endl;
        throwBoxException(box, QString("Failed to build library :\n%1").arg(
            QString::fromUtf8(proc.readAllStandardError())));
    }
}

static void buildLib(const QDir& dir, bool withQmake, const Box *box)
{
    DirRemover drm(dir.absolutePath());
    if (withQmake)
        buildLibWithQmake(dir, box);
    else
        buildLibNoQmake(dir, box);
    drm.cancel();
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

    // Get library dir name, class name, and hash string
    QString className, hashString;
    QString libDir = buildDirPriv(src, &className, &hashString);

    // Generate library subdir path
    QString subdirPath = "equares/" + libDir;

    // Library file full path name will be stored here
    QString libName;

    // Library file base name
    QString baseName = "ode";

    // Obtain global settings instance
    const QVariantMap& globalSettings = PerTypeStorage::instance<QVariantMap>();

    // At first, look up the library in the cache directory
    QString cacheDir = globalSettings["cacheDir"].toString();
    if (!cacheDir.isEmpty()) {
        QDir dir(cacheDir);
        if (dir.exists() && dir.cd(subdirPath) && libUpToDate(dir.absoluteFilePath(baseName), hashString))
            libName = dir.absoluteFilePath(baseName);
    }

    if (libName.isEmpty()) {
        // The library is missing in the cache, try looking up in the current dir; build if necessary and allowed
        bool canBuild = !globalSettings["denyBuild"].toBool();

        // Prepare work directory
        QDir dir = QDir::current();
        if (canBuild && !dir.mkpath(subdirPath))
            throwBoxException(QString("Failed to create directory %1").arg(dir.absoluteFilePath(subdirPath)));
        if (!dir.cd(subdirPath))
            throwBoxException(QString("Failed to switch to directory %1").arg(dir.absoluteFilePath(subdirPath)));

        if (!libUpToDate(dir.absoluteFilePath(baseName), hashString)) {
            if (!canBuild)
                throwBoxException("Failed to build library because build is denied");
            // Generate source file
            QFileInfo fi(dir.absoluteFilePath(baseName + ".cpp"));
            {
                QString srcFileContent =
                    src + "\n" +
                    readFile(":/cxx/OdeFileFooter.cpp", this).replace("<X>", className) + "\n" +
                        "extern \"C\" ODE_EXPORT const char *hash() { return \"" + hashString + "\"; }\n";
                QFile srcFile(fi.absoluteFilePath());
                if (!srcFile.open(QIODevice::WriteOnly))
                    throwBoxException(QString("Failed to open file %1").arg(fi.absoluteFilePath()));
                srcFile.write(srcFileContent.toUtf8());
            }

            // Build library
            buildLib(dir, m_useQmake, this);
        }
        libName = dir.absoluteFilePath(baseName);
    }

    // Load library
    m_libProxy.clear();
    m_libProxy = OdeLibProxy::Ptr(new OdeLibProxy(libName, this));

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
    return buildDirPriv(boxProps.property("src").toString());
}

QString OdeCxxBox::buildDirPriv(const QString& src, QString *className, QString* hashString) const
{
    // Extract class name that will also be part of directory name
    QString className_;
    {
        QRegExp rx("^\\s*struct\\s+([a-zA-Z_][a-zA-Z0-9_]*)");
        foreach (const QString& line, src.split('\n'))
            if (rx.indexIn(line) != -1) {
                className_ = rx.capturedTexts()[1];
                Q_ASSERT(!className_.isEmpty());
                break;
            }
        if (className_.isEmpty())
            throwBoxException("Failed to set source: unable to retrieve struct name");
    }
    if (className)
        *className = className_;

    // Generate md5 checksum
    QString hashString_;
    {
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(src.toUtf8());
        const char *xdigits = "0123456789abcdef";
        foreach(unsigned char c, hash.result()) {
            hashString_ += xdigits[(c>>4) & 0xf];
            hashString_ += xdigits[c & 0xf];
        }
    }
    if (hashString)
        *hashString = hashString_;

    // Generate library subdir path
    return className_ + "_" + hashString_;
}

QString OdeCxxBox::srcExample() const {
    return "#include <cmath>\n\n" + readFile(":/cxx/OdeClass.cpp", this);
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
    return m_libProxy->paramCount();
}

QStringList OdeCxxBox::paramNames() const
{
    if (m_libProxy.isNull())
        throwBoxException("OdeCxxBox::paramNames: No source is specified");
    return m_libProxy->paramNames();
}

int OdeCxxBox::varCount() const {
    if (m_libProxy.isNull())
        throwBoxException("OdeCxxBox::varCount: No source is specified");
    return m_libProxy->varCount();
}

QStringList OdeCxxBox::varNames() const
{
    if (m_libProxy.isNull())
        throwBoxException("OdeCxxBox::varNames: No source is specified");
    return m_libProxy->varNames();
}

const OdeCxxBox::OdeLibProxy *OdeCxxBox::odeLibProxy() const {
    return m_libProxy.data();
}

OdeCxxBox::OdeLibProxy::OdeLibProxy(const QString& libName, const Box *box) :
    m_lib(libName),
    m_box(box)
{
    try {
        checkLib(libName);
    }
    catch(const std::exception& e) {
        ::throwBoxException(m_box, QString("Security check has failed on library file '%1':\n%2").arg(libName, QString::fromUtf8(e.what())));
    }
    m_lib.load();
    if (!m_lib.isLoaded())
        ::throwBoxException(m_box, QString("Failed to open library file '%1'").arg(libName));

#define RESOLVE_SYMBOL(name) \
    m_##name = reinterpret_cast<name##Func>(m_lib.resolve(#name)); \
    if (!m_##name) \
        ::throwBoxException(m_box, QString("Failed to resolve symbol '%1' in library '%1'").arg(#name, libName));
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
    m_deleteInstance(m_inst);
}

QStringList OdeCxxBox::OdeLibProxy::toNameList(const char *s)
{
    QStringList result;
    if (!s)
        return result;
    QString qs = QString::fromUtf8(s);
    if (qs.isEmpty())
        return result;
    result = qs.split(",", QString::KeepEmptyParts);
    for (int i=0; i<result.size(); ++i)
        result[i] = result[i].trimmed();
    return result;
}



bool OdeCxxBox::libUpToDate(const QString &libName, const QString& hashString)
{
    try {
        QFileInfo fi(libName);
        QString libFileName = fi.dir().absoluteFilePath("lib" + fi.baseName() + ".so");
        if (QFileInfo(libFileName).exists()) {
            OdeLibProxy lib(libName, this);
            return lib.hash() == hashString;
        }
        else
            return false;
    }
    catch (const EquaresException&) {
        return false;
    }
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
