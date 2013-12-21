#include "equares_exec.h"
#include <iostream>

DefaultThreadOutput::DefaultThreadOutput() :
    m_stdout(::stdout, QIODevice::WriteOnly),
    m_stderr(::stderr, QIODevice::WriteOnly)
{
}

QTextStream& DefaultThreadOutput::stdout() {
    return m_stdout;
}

QTextStream& DefaultThreadOutput::stderr() {
    return m_stderr;
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
    runnable->run();
    return *this;
}
