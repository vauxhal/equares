#ifndef EQUARES_EXEC_H
#define EQUARES_EXEC_H

#include <QString>
#include <QSharedPointer>
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QSize>
#include <QVector>
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
    void createStubFile() const;

private:
    OutputFileInfo(const QString& name, Type type, const QSize& size = QSize()) :
        m_name(name), m_type(type), m_size(size) {}
    QString m_name;
    Type m_type;
    QSize m_size;
};

typedef QList<OutputFileInfo> OutputFileInfoList;


class EQUARES_CORESHARED_EXPORT InputInfo
{
public:
    typedef QSharedPointer<InputInfo> Ptr;
    explicit InputInfo(const QString& consumerId) :
        m_consumerId(consumerId) {}
    virtual ~InputInfo() {}
    const QString& consumerId() const {
        return m_consumerId;
    }

    virtual QString toString() const = 0;

private:
    QString m_consumerId;
};

typedef QList<InputInfo::Ptr> InputInfoList;

class EQUARES_CORESHARED_EXPORT ImageInputInfo : public InputInfo
{
public:
    enum Method {
        ClickImage,
        HoverImage
    };
    ImageInputInfo(const QString& consumerId, const QString& refImage, Method method = ClickImage) :
        InputInfo(consumerId),
        m_refImage(refImage), m_method(method) {}
    QString toString() const;
private:
    QString m_refImage;
    Method m_method;
    static QString methodToString(Method method);
};

class EQUARES_CORESHARED_EXPORT SimpleInputInfo : public InputInfo
{
public:
    SimpleInputInfo(const QString& consumerId, const QStringList& names) :
        InputInfo(consumerId),
        m_names(names) {}
    QString toString() const;
private:
    QStringList m_names;
};

class EQUARES_CORESHARED_EXPORT SignalInputInfo : public InputInfo
{
public:
    SignalInputInfo(const QString& consumerId, const QString& signalName) :
        InputInfo(consumerId), m_signalName(signalName) {}
    QString toString() const;
private:
    QString m_signalName;
};

class EQUARES_CORESHARED_EXPORT RangeInputInfo : public InputInfo
{
public:
    struct Range {
        QString name;
        double vmin;
        double vmax;
        int resolution;
        Range() : vmin(0), vmax(10), resolution(100) {}
        Range(const QString& name, double vmin, double vmax, int resolution = 100) :
            name(name), vmin(vmin), vmax(vmax), resolution(resolution) {}
    };
    typedef QList<Range> Ranges;

    RangeInputInfo(const QString& consumerId, const Ranges& ranges) :
        InputInfo(consumerId),
        m_ranges(ranges) {}
    QString toString() const;
private:
    Ranges m_ranges;
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

    virtual int registerInput(InputInfo::Ptr inputInfo) = 0;
    virtual QVector<double> readInput(int inputId, bool wait) = 0;

private:
    static QList<ThreadManager*> m_instances;
};

struct ThreadManagerInputData
{
    typedef QVector<double> Chunk;
    typedef QList<Chunk> Buf;
    InputInfo::Ptr info;
    Buf buf;
    ThreadManagerInputData() {}
    explicit ThreadManagerInputData(InputInfo::Ptr info) : info(info) {}
};

class EQUARES_CORESHARED_EXPORT DefaultThreadManager : public ThreadManager
{
public:
    DefaultThreadManager();

    ThreadOutput::Ptr threadOutput() const;
    ThreadManager& setThreadOutput(ThreadOutput::Ptr threadOutput);
    ThreadManager& start(Runnable *runnable);
    ThreadManager& reportProgress(const ProgressInfo& pi);

    int registerInput(InputInfo::Ptr inputInfo);
    QVector<double> readInput(int inputId, bool wait);

private:
    ThreadOutput::Ptr m_threadOutput;
    QList<ThreadManagerInputData> m_inputData;
};

#define EQUARES_COUT ThreadManager::instance()->threadOutput()->standardOutput()
#define EQUARES_CERR ThreadManager::instance()->threadOutput()->standardError()

#endif // EQUARES_EXEC_H
