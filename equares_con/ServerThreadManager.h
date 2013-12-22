#ifndef SERVERTHREADMANAGER_H
#define SERVERTHREADMANAGER_H

#include "equares_core/equares_exec.h"
#include <QFile>
#include <QThread>
#include <QThreadStorage>
#include <QMutex>
#include <QSemaphore>

class ServerOutputStream : public QFile
{
public:
    explicit ServerOutputStream(FILE *os);
    ~ServerOutputStream();

    QString prefix() const;
    ServerOutputStream& setPrefix(const QString& prefix);

    qint64 writeData(const char *data, qint64 len);
    void close();

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

    QTextStream& stdout();
    QTextStream& stderr();

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

protected:
    void run();

private:
    ServerThreadManager *m_threadMan;
    Runnable *m_runnable;
    int m_jobId;
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
    ThreadManager& endSync(int jobId);

    void initThread(ServerThread *thread, int jobId);
    void cleanupThread();

private:
    struct ThreadData
    {
        ServerThread *thread;
        int jobId;
        ThreadOutput::Ptr threadOutput;
        ThreadData(ServerThread *thread, int jobId);
    };

    QThreadStorage<ThreadData*> m_threadData;
    ThreadOutput::Ptr m_threadOutput;

    QMutex m_mutex;
    QList<ServerThread*> m_threads;
    QList< QSharedPointer<ServerThread> > m_finishedThreads;
    typedef QSharedPointer<QSemaphore> SemPtr;
    typedef QMap<int, SemPtr> SemMap;
    SemMap m_semSync;  // Key = job id, value = semaphore
    QSemaphore& semSync(int jobId);
    void addThread(ServerThread* thread);
    void removeThread(ServerThread* thread);

    static int newJobId();
};

#endif // SERVERTHREADMANAGER_H
