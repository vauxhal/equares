#ifndef SERVERTHREADMANAGER_H
#define SERVERTHREADMANAGER_H

#include "equares_core/equares_exec.h"
#include <QThread>
#include <QThreadStorage>
#include <QMutex>
#include <QSemaphore>

class ServerOutputStream : public DefaultOutputStream
{
public:
    explicit ServerOutputStream(FILE *os);
    ~ServerOutputStream();

    QString prefix() const;
    ServerOutputStream& setPrefix(const QString& prefix);

    qint64 writeData(const char *data, qint64 len);
    void close();
    bool isEmpty() const;

private:
    QString m_prefix;

    QByteArray m_prefixBytes;
    QByteArray m_buf;
    qint64 m_bufLen;

    void appendToBuf(const char *data, qint64 len);
    void clearBuf();
    void flushBuf();
};

class ServerThreadOutput : public ThreadOutput
{
public:
    ServerThreadOutput();

    QTextStream& standardOutput();
    QTextStream& standardError();
    bool hasErrors() const;

    QString prefix() const;
    ServerThreadOutput& setPrefix(const QString& prefix);

private:
    ServerOutputStream m_stdoutStream;
    ServerOutputStream m_stderrStream;
    QTextStream m_stdout;
    QTextStream m_stderr;
};

class ServerThreadManager;

class ServerThread : public QThread
{
public:
    ServerThread(ServerThreadManager *threadMan, Runnable *runnable, int jobId);
    void requestTermination();
    using QThread::msleep;  // Protected in Qt4

protected:
    void run();

private:
    ServerThreadManager *m_threadMan;
    Runnable *m_runnable;
    int m_jobId;
    QMutex m_mutex;
};

class ServerThreadManager : public ThreadManager
{
public:
    ServerThreadManager();
    ~ServerThreadManager();

    ThreadOutput::Ptr threadOutput() const;
    ThreadManager& setThreadOutput(ThreadOutput::Ptr threadOutput);
    int jobId() const;
    ThreadManager& start(Runnable *runnable);
    ThreadManager& reportProgress(const ProgressInfo& pi);
    int registerInput(InputInfo::Ptr inputInfo);
    QVector<double> readInput(int inputId, bool wait);
    ThreadManager& endSync(int jobId);
    ThreadManager& requestTermination(int jobId);
    ThreadManager& requestTermination();
    ThreadManager& sendInput(int jobId, const QString& input);

    void initThread(ServerThread *thread, int jobId);
    void cleanupThread();

private:
    struct ThreadData
    {
        ServerThread *thread;
        int jobId;
        ThreadOutput::Ptr threadOutput;
        QList<ThreadManagerInputData> inputData;
        ThreadData(ServerThread *thread, int jobId);
    };

    QThreadStorage<ThreadData*> m_threadData;

    QMutex m_mutex;
    QList<ServerThread*> m_threads;
    QList< QSharedPointer<ServerThread> > m_finishedThreads;
    typedef QSharedPointer<QSemaphore> SemPtr;
    class ThreadSharedData
    {
    public:
        ThreadSharedData() {}
        explicit ThreadSharedData(ServerThread *thread) : m_thread(thread) {}
        QSemaphore *sem() const {
            if (!m_sem)
                m_sem = SemPtr(new QSemaphore());
            return m_sem.data();
        }
        ServerThread *thread() const {
            return m_thread;
        }
        QStringList m_threadInput;
    private:
        mutable SemPtr m_sem;
        ServerThread *m_thread;
    };
    enum { MaxSemLocks = 1000000 };

    typedef QMap<int, ThreadSharedData> ThreadMap;
    ThreadMap m_threadSharedData;  // Key = job id, value = thread shared data
    QSemaphore *semSync(int jobId);
    void addThread(int jobId, ServerThread* thread);
    void removeThread(int jobId, ServerThread* thread);

    static int newJobId();
};

#endif // SERVERTHREADMANAGER_H
