#ifndef EQUARES_EXEC_H
#define EQUARES_EXEC_H

#include <QString>
#include <QSharedPointer>
#include <QTextStream>
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
    virtual QTextStream& stdout() = 0;
    virtual QTextStream& stderr() = 0;
};

class EQUARES_CORESHARED_EXPORT DefaultThreadOutput : public ThreadOutput
{
public:
    DefaultThreadOutput();

    QTextStream& stdout();
    QTextStream& stderr();

private:
    QTextStream m_stdout;
    QTextStream m_stderr;
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

private:
    ThreadOutput::Ptr m_threadOutput;
};

#define EQUARES_COUT ThreadManager::instance()->threadOutput()->stdout()
#define EQUARES_CERR ThreadManager::instance()->threadOutput()->stderr()

#endif // EQUARES_EXEC_H
