#include "equares_exec.h"
#include <iostream>

DefaultOutputStream::DefaultOutputStream(FILE *os) : m_empty(true)
{
    open(os, QIODevice::WriteOnly, QFile::DontCloseHandle);
}

qint64 DefaultOutputStream::writeData(const char *data, qint64 len)
{
    if (len > 0)
        m_empty = false;
    return QFile::writeData(data, len);
}

bool DefaultOutputStream::isEmpty() const {
    return m_empty;
}


DefaultThreadOutput::DefaultThreadOutput() :
    m_stdoutStream(stdout),
    m_stderrStream(stderr),
    m_stdout(&m_stdoutStream),
    m_stderr(&m_stderrStream)
{
}

QTextStream& DefaultThreadOutput::standardOutput() {
    return m_stdout;
}

QTextStream& DefaultThreadOutput::standardError() {
    return m_stderr;
}

bool DefaultThreadOutput::hasErrors() const {
    return !m_stderrStream.isEmpty();
}



QString OutputFileInfo::toString() const
{
    QString result = "{'name': '" + name() + "', 'type': ";
    switch (type()) {
    case OutputFileInfo::Text:
        result += "'text'";
        break;
    case OutputFileInfo::Image:
        result += "'image'";
        result += ", 'size': {'width': " + QString::number(width()) + ", 'height': " + QString::number(height()) + "}";
        break;
    default:
        Q_ASSERT(false);
    }
    result += "}";
    return result;
}



QList<ThreadManager*> ThreadManager::m_instances;

ThreadManager::ThreadManager()
{
    m_instances << this;
}

ThreadManager::~ThreadManager()
{
    int index = m_instances.indexOf(this);
    Q_ASSERT(index != -1);
    m_instances.removeAt(index);
}

ThreadManager *ThreadManager::instance()
{
    // If this assertion fails, create an instance of
    // a class derived from ThreadManager on the stack.
    Q_ASSERT(!m_instances.isEmpty());
    return m_instances.last();
}



DefaultThreadManager::DefaultThreadManager() :
    m_threadOutput(new DefaultThreadOutput())
{
}

ThreadOutput::Ptr DefaultThreadManager::threadOutput() const {
    return m_threadOutput;
}

ThreadManager& DefaultThreadManager::setThreadOutput(ThreadOutput::Ptr threadOutput) {
    m_threadOutput = threadOutput;
    return *this;
}

ThreadManager& DefaultThreadManager::start(Runnable *runnable)
{
    try {
        runnable->run();
    } catch (std::exception& e) {
        EQUARES_CERR << "ERROR: " << e.what() << endl;
    }
    delete runnable;

    return *this;
}

ThreadManager& DefaultThreadManager::reportProgress(const ProgressInfo& pi)
{
    QStringList msg;
    if (pi.hasProgress()) {
        int percent = static_cast<int>( pi.progress()*100 + 0.5 );
        msg << QString("progress: %1%%").arg(percent);
    }
    foreach(const QString& file, pi.files())
        msg << "file: " + file;
    if (pi.needSync())
        msg << "sync";
    if (!msg.isEmpty())
        EQUARES_COUT << msg.join("\n") << endl;
    return *this;
}
