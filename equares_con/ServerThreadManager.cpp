#include "ServerThreadManager.h"
#include <QMutexLocker>

ServerOutputStream::ServerOutputStream(FILE *os) :
    DefaultOutputStream(os), m_bufLen(0)
{
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
    DefaultOutputStream::close();
}

bool ServerOutputStream::isEmpty() const {
    return m_bufLen == m_prefixBytes.size() && DefaultOutputStream::isEmpty();
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
    DefaultOutputStream::writeData(m_buf.data(), m_bufLen);
    // DefaultOutputStream::flush();
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

bool ServerThreadOutput::hasErrors() const {
    return !m_stderrStream.isEmpty();
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
    {
        QMutexLocker lock(&m_mutex);
        delete m_runnable;
        m_runnable = 0;
    }
    m_threadMan->cleanupThread();
}

void ServerThread::requestTermination()
{
    QMutexLocker lock(&m_mutex);
    if (m_runnable)
        m_runnable->requestTermination();
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
    requestTermination();
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
    if (pi.needSync()) {
        QSemaphore *sem = semSync(m_threadData.localData()->jobId);
        Q_ASSERT(sem);
        sem->acquire();
    }
    return *this;
}

int ServerThreadManager::registerInput(InputInfo::Ptr inputInfo)
{
    Q_ASSERT(m_threadData.hasLocalData());
    ThreadData *td = m_threadData.localData();
    QList<ThreadManagerInputData>& dlst = td->inputData;
    int result = dlst.size();
    dlst << ThreadManagerInputData(inputInfo);
    return result;
}

QVector<double> ServerThreadManager::readInput(int inputId, bool wait)
{
    Q_ASSERT(m_threadData.hasLocalData());
    ThreadData *td = m_threadData.localData();
    QList<ThreadManagerInputData>& dlst = td->inputData;
    Q_ASSERT(inputId >= 0   &&   inputId < dlst.size());
    ThreadManagerInputData& d = dlst[inputId];

    while (d.buf.isEmpty()) {
        // Fetch data from thread input stream
        QMutexLocker lock(&m_mutex);
        ThreadMap::iterator it = m_threadSharedData.find(td->jobId);
        Q_ASSERT(it != m_threadSharedData.end());
        QStringList& threadInput = it.value().m_threadInput;
        if (threadInput.isEmpty()) {
            if (wait) {
                lock.unlock();
                td->thread->msleep(500);
                continue;
            }
            else
                return QVector<double>();
        }
        QString s = threadInput.first();
        threadInput.removeFirst();
        QStringList tokens = s.trimmed().split(QRegExp("\\s+"));
        if (tokens.isEmpty())
            continue;
        for (int id=0; id<dlst.size(); ++id) {
            ThreadManagerInputData& d = dlst[id];
            if (tokens[0] != d.info->consumerId())
                continue;
            QVector<double> chunk(tokens.length()-1);
            for (int i=1; i<tokens.length(); ++i)
                chunk[i-1] = tokens[i].toDouble();
            d.buf << chunk;
        }
    }

    // Pick item at the beginning of the buffer
    QVector<double> chunk = d.buf.first();
    d.buf.removeFirst();
    return chunk;
}

ThreadManager& ServerThreadManager::endSync(int jobId)
{
    QSemaphore *sem = semSync(jobId);
    if (sem)
        sem->release();
    return *this;
}

ThreadManager& ServerThreadManager::requestTermination(int jobId)
{
    QMutexLocker lock(&m_mutex);
    ThreadMap::iterator it = m_threadSharedData.find(jobId);
    if (it != m_threadSharedData.end()) {
        it.value().sem()->release(MaxSemLocks);
        it.value().thread()->requestTermination();
    }
    return *this;
}

ThreadManager& ServerThreadManager::requestTermination()
{
    QMutexLocker lock(&m_mutex);
    foreach(const ThreadSharedData& d, m_threadSharedData) {
        d.sem()->release(MaxSemLocks);
        d.thread()->requestTermination();
    }
    return *this;
}

ThreadManager& ServerThreadManager::sendInput(int jobId, const QString& input)
{
    QMutexLocker lock(&m_mutex);
    ThreadMap::iterator it = m_threadSharedData.find(jobId);
    if (it != m_threadSharedData.end())
        it.value().m_threadInput << input;
    return *this;
}

void ServerThreadManager::initThread(ServerThread *thread, int jobId)
{
    m_threadData.setLocalData(new ThreadData(thread, jobId));
    if (thread)
        addThread(jobId, thread);
}

void ServerThreadManager::cleanupThread()
{
    Q_ASSERT(m_threadData.hasLocalData());
    ThreadData *d = m_threadData.localData();
    removeThread(d->jobId, d->thread);
    m_threadData.setLocalData(0);
}

int ServerThreadManager::newJobId()
{
    static int jobId = 0;
    return ++jobId;
}

QSemaphore *ServerThreadManager::semSync(int jobId)
{
    QMutexLocker lock(&m_mutex);
    ThreadMap::iterator it = m_threadSharedData.find(jobId);
    if (it == m_threadSharedData.end())
        return 0;
    return it.value().sem();
}

void ServerThreadManager::addThread(int jobId, ServerThread* thread)
{
    QMutexLocker lock(&m_mutex);
    m_finishedThreads.clear();
    m_threads << thread;
    m_threadSharedData.insert(jobId, ThreadSharedData(thread));
}

void ServerThreadManager::removeThread(int jobId, ServerThread* thread)
{
    QMutexLocker lock(&m_mutex);
    int index = m_threads.indexOf(thread);
    Q_ASSERT(index != -1);
    m_threads.removeAt(index);
    m_threadSharedData.remove(jobId);
    m_finishedThreads << QSharedPointer<ServerThread>(thread);
}
