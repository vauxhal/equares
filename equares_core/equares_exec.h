#ifndef EQUARES_EXEC_H
#define EQUARES_EXEC_H

#include <QString>
#include <QSharedPointer>
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QSize>
#include "equares_core_global.h"
#include "equares_common.h"

struct EQUARES_CORESHARED_EXPORT Runnable
{
    virtual ~Runnable() {}
    virtual void run() = 0;
    virtual void requestTermination() = 0;
};



class EQUARES_CORESHARED_EXPORT ThreadOutput
{
public:
    typedef QSharedPointer<ThreadOutput> Ptr;

    virtual ~ThreadOutput() {}
    virtual QTextStream& standardOutput() = 0;
    virtual QTextStream& standardError() = 0;
    virtual bool hasErrors() const = 0;
};

class EQUARES_CORESHARED_EXPORT DefaultOutputStream : public QFile
{
public:
    explicit DefaultOutputStream(FILE *os);

    qint64 writeData(const char *data, qint64 len);
    virtual bool isEmpty() const;

private:
    bool m_empty;
};

class EQUARES_CORESHARED_EXPORT DefaultThreadOutput : public ThreadOutput
{
public:
    DefaultThreadOutput();

    QTextStream& standardOutput();
    QTextStream& standardError();
    bool hasErrors() const;

private:
    DefaultOutputStream m_stdoutStream;
    DefaultOutputStream m_stderrStream;
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

class EQUARES_CORESHARED_EXPORT OutputFileInfo
{
public:
    enum Type { Text, Image };
    OutputFileInfo() : m_type(Text) {}
    QString name() const { return m_name; }
    Type type() const { return m_type; }
    QSize size() const { return m_size; }
    int width() const { return m_size.width(); }
    int height() const { return m_size.height(); }
    static OutputFileInfo text(const QString& name) {
        return OutputFileInfo(name, Text);
    }
    static OutputFileInfo image(const QString& name, const QSize& size) {
        return OutputFileInfo(name, Image, size);
    }
    static OutputFileInfo image(const QString& name, int width, int height) {
        return OutputFileInfo(name, Image, QSize(width, height));
    }
    QString toString() const;
private:
    explicit OutputFileInfo(const QString& name, Type type, const QSize& size = QSize()) :
        m_name(name), m_type(type), m_size(size) {}
    QString m_name;
    Type m_type;
    QSize m_size;
};

typedef QList<OutputFileInfo> OutputFileInfoList;

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
