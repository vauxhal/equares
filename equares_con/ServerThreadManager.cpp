#include "ServerThreadManager.h"
#include <QMutexLocker>

ServerOutputStream::ServerOutputStream(FILE *os) : m_bufLen(0)
{
    open(os, QIODevice::WriteOnly, QFile::DontCloseHandle);
}

ServerOutputStream::~ServerOutputStream() {
    close();
}

QString ServerOutputStream::prefix() const {
    return m_prefix;
}

ServerOutputStream& ServerOutputStream::setPrefix(const QString& prefix) {
    flushBuf();
    m_prefix = prefix;
    m_prefixBytes = prefix.toUtf8();
    clearBuf();
    return *this;
}

qint64 ServerOutputStream::writeData(const char *data, qint64 len)
{
    qint64 start = 0;
    while (start < len) {
        qint64 i;
        for (i=start; i<len && data[i]!='\n'; ++i) {}
        if (i == len) {
            appendToBuf(data+start, len-start);
            break;
        }
        appendToBuf(data+start, i+1-start);
        flushBuf();
        start = i+1;
    }
    return len;
}

void ServerOutputStream::close()
{
    flushBuf();
    QFile::close();
}

void ServerOutputStream::appendToBuf(const char *data, qint64 len)
{
    if (m_buf.size() < m_bufLen+len)
        m_buf.resize(m_bufLen+len);
    char *bufData = m_buf.data();
    for (qint64 i=0; i<len; ++i)
        bufData[m_bufLen+i] = data[i];
    m_bufLen += len;
}

void ServerOutputStream::clearBuf()
{
    m_bufLen = 0;
    appendToBuf(m_prefixBytes.data(), m_prefixBytes.size());
}

void ServerOutputStream::flushBuf()
{
    if (m_bufLen == m_prefixBytes.size())
        return;
    QFile::writeData(m_buf.data(), m_bufLen);
    // QFile::flush();
    clearBuf();
}



ServerThreadOutput::ServerThreadOutput() :
    m_stdoutStream(stdout),
    m_stderrStream(stderr),
    m_stdout(&m_stdoutStream),
    m_stderr(&m_stderrStream)
{
}

QTextStream& ServerThreadOutput::standardOutput() {
    return m_stdout;
}

QTextStream& ServerThreadOutput::standardError() {
    return m_stderr;
}

QString ServerThreadOutput::prefix() const {
    return m_stdoutStream.prefix();
}

ServerThreadOutput& ServerThreadOutput::setPrefix(const QString& prefix)
{
    m_stdoutStream.setPrefix(prefix);
    m_stderrStream.setPrefix(prefix);
    return *this;
}



ServerThread::ServerThread(ServerThreadManager *threadMan, Runnable *runnable, int jobId) :
    m_threadMan(threadMan),
    m_runnable(runnable),
    m_jobId(jobId)
{
}

void ServerThread::run()
{
    m_threadMan->initThread(this, m_jobId);
    try {
        EQUARES_COUT << "started" << endl;
        m_runnable->run();
        EQUARES_COUT << "finished" << endl;
    }
    catch (const std::exception& e) {
        EQUARES_CERR << "ERROR (terminated): " << e.what() << endl;
    }
    delete m_runnable;
    m_threadMan->cleanupThread();
}



ServerThreadManager::ThreadData::ThreadData(ServerThread *thread, int jobId) :
    thread(thread), jobId(jobId)
{
    ServerThreadOutput *o = new ServerThreadOutput();
    o->setPrefix(QString("==%1==> ").arg(jobId));
    threadOutput = ThreadOutput::Ptr(o);
}


ServerThreadManager::ServerThreadManager() {
    initThread(0, newJobId());
}

ServerThreadManager::~ServerThreadManager()
{
    while (!m_threads.isEmpty())
        m_threads.first()->wait();
    m_threadData.setLocalData(0);
}

ThreadOutput::Ptr ServerThreadManager::threadOutput() const
{
    Q_ASSERT(m_threadData.hasLocalData());
    return m_threadData.localData()->threadOutput;
}

ThreadManager& ServerThreadManager::setThreadOutput(ThreadOutput::Ptr threadOutput)
{
    Q_ASSERT(m_threadData.hasLocalData());
    m_threadData.localData()->threadOutput = threadOutput;
    return *this;
}

int ServerThreadManager::jobId() const
{
    Q_ASSERT(m_threadData.hasLocalData());
    return m_threadData.localData()->jobId;
}

ThreadManager& ServerThreadManager::start(Runnable *runnable)
{
    ServerThread *thread = new ServerThread(this, runnable, newJobId());
    thread->start();
    return *this;
}

ThreadManager& ServerThreadManager::reportProgress(const ProgressInfo& pi)
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
    if (pi.needSync())
        semSync(m_threadData.localData()->jobId).acquire();
    return *this;
}

ThreadManager& ServerThreadManager::endSync(int jobId)
{
    QMutexLocker lock(&m_mutex);
    SemMap::iterator it = m_semSync.find(jobId);
    if (it != m_semSync.end())
        it.value()->release();
    return *this;
}

void ServerThreadManager::initThread(ServerThread *thread, int jobId)
{
    m_threadData.setLocalData(new ThreadData(thread, jobId));
    if (thread)
        addThread(thread);
}

void ServerThreadManager::cleanupThread()
{
    Q_ASSERT(m_threadData.hasLocalData());
    removeThread(m_threadData.localData()->thread);
    m_threadData.setLocalData(0);
}

int ServerThreadManager::newJobId()
{
    static int jobId = 0;
    return ++jobId;
}

QSemaphore& ServerThreadManager::semSync(int jobId)
{
    QMutexLocker lock(&m_mutex);
    SemMap::iterator it = m_semSync.find(m_threadData.localData()->jobId);
    if (it == m_semSync.end())
        it = m_semSync.insert(jobId, SemPtr(new QSemaphore()));
    return *it.value().data();
}

void ServerThreadManager::addThread(ServerThread* thread)
{
    QMutexLocker lock(&m_mutex);
    m_finishedThreads.clear();
    m_threads << thread;
}

void ServerThreadManager::removeThread(ServerThread* thread)
{
    QMutexLocker lock(&m_mutex);
    int index = m_threads.indexOf(thread);
    Q_ASSERT(index != -1);
    m_threads.removeAt(index);
    SemMap::iterator it = m_semSync.find(m_threadData.localData()->jobId);
    if (it != m_semSync.end())
        m_semSync.erase(it);
    m_finishedThreads << QSharedPointer<ServerThread>(thread);
}
