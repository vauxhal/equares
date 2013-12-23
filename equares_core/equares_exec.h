#ifndef EQUARES_EXEC_H
#define EQUARES_EXEC_H

#include <QString>
#include <QSharedPointer>
#include <QTextStream>
#include <QStringList>
#include "equares_core_global.h"
#include "equares_common.h"

struct EQUARES_CORESHARED_EXPORT Runnable
{
    virtual ~Runnable() {}
    virtual void run() = 0;
};



class EQUARES_CORESHARED_EXPORT ThreadOutput
{
public:
    typedef QSharedPointer<ThreadOutput> Ptr;

    virtual ~ThreadOutput() {}
    virtual QTextStream& standardOutput() = 0;
    virtual QTextStream& standardError() = 0;
};

class EQUARES_CORESHARED_EXPORT DefaultThreadOutput : public ThreadOutput
{
public:
    DefaultThreadOutput();

    QTextStream& standardOutput();
    QTextStream& standardError();

private:
    QTextStream m_stdout;
    QTextStream m_stderr;
};



class ProgressInfo
{
public:
    ProgressInfo() : m_progress(-1), m_needSync(false) {}
    ProgressInfo& setProgress(double progress) {
        m_progress = progress;
        return *this;
    }
    ProgressInfo& operator<<(const QString& fileName) {
        m_files << fileName;
        return *this;
    }
    ProgressInfo& setSync(bool needSync = true) {
        m_needSync = needSync;
        return *this;
    }


    bool hasProgress() const {
        return m_progress >= 0;
    }
    double progress() const {
        return m_progress;
    }
    bool hasFiles() const {
        return !m_files.isEmpty();
    }
    const QStringList& files() const {
        return m_files;
    }
    bool needSync() const {
        return m_needSync;
    }
private:
    double m_progress;
    QStringList m_files;
    bool m_needSync;
};

class EQUARES_CORESHARED_EXPORT ThreadManager
{
    DENY_INSTANCE_COPYING(ThreadManager)
public:
    typedef QSharedPointer<ThreadManager> Ptr;

    ThreadManager();
    virtual ~ThreadManager();

    static ThreadManager *instance();

    virtual ThreadOutput::Ptr threadOutput() const = 0;
    virtual ThreadManager& setThreadOutput(ThreadOutput::Ptr threadOutput) = 0;
    virtual ThreadManager& start(Runnable *runnable) = 0;
    virtual ThreadManager& reportProgress(const ProgressInfo& pi) = 0;

private:
    static QList<ThreadManager*> m_instances;
};

class EQUARES_CORESHARED_EXPORT DefaultThreadManager : public ThreadManager
{
public:
    DefaultThreadManager();

    ThreadOutput::Ptr threadOutput() const;
    ThreadManager& setThreadOutput(ThreadOutput::Ptr threadOutput);
    ThreadManager& start(Runnable *runnable);
    ThreadManager& reportProgress(const ProgressInfo& pi);

private:
    ThreadOutput::Ptr m_threadOutput;
};

#define EQUARES_COUT ThreadManager::instance()->threadOutput()->standardOutput()
#define EQUARES_CERR ThreadManager::instance()->threadOutput()->standardError()

#endif // EQUARES_EXEC_H
