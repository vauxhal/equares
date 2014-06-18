#include "equares_exec.h"
#include "EquaresException.h"
#include <QPainter>
#include <QImage>
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
    QString result = "{\"name\": \"" + name() + "\", \"type\": ";
    switch (type()) {
    case OutputFileInfo::Text:
        result += "\"text\"";
        break;
    case OutputFileInfo::Image:
        result += "\"image\"";
        result += ", \"size\": {\"width\": " + QString::number(width()) + ", \"height\": " + QString::number(height()) + "}";
        break;
    default:
        Q_ASSERT(false);
    }
    result += "}";
    return result;
}

void OutputFileInfo::createStubFile() const
{
    switch(m_type) {
    case Text: {
        QFile f(m_name);
        if (!f.open(QFile::WriteOnly))
            throw EquaresException(QObject::tr("Failed to create output file '%1'").arg(m_name));
        f.write(QByteArray("Waiting...\n"));
        break;
        }
    case Image: {
        QImage img(m_size, QImage::Format_Mono);
        {
            QPainter painter(&img);
            QRect rc(QPoint(0,0), m_size);
            painter.fillRect(rc, Qt::white);
            int w = m_size.width(), h = m_size.height();
            painter.drawLine(0, 0, w, h);
            painter.drawLine(0, h, w, 0);
            painter.drawLine(0, 0, w, 0);
            painter.drawLine(0, h-1, w, h-1);
            painter.drawLine(0, 0, 0, h);
            painter.drawLine(w-1, 0, w-1, h);
            // Note: The following would need QGuiApplication
            // painter.drawText(rc, Qt::AlignCenter, QString("Waiting..."));
        }
        img.save(m_name);
        break;
    }
    default:
        Q_ASSERT(false);
    }
}



QString ImageInputInfo::toString() const
{
    return QString("{\"consumer\": \"%1\", \"type\": \"image\", \"method\": \"%2\", \"refImage\": \"%3\"}")
        .arg(consumerId(), methodToString(m_method), m_refImage);
}

QString ImageInputInfo::methodToString(Method method) {
    switch (method) {
    case ClickImage: return "click";
    case HoverImage: return "hover";
    default: Q_ASSERT(false); return "unknown";
    }
}

QString SimpleInputInfo::toString() const
{
    QString result = QString("{\"consumer\": \"%1\", \"type\": \"simple\", \"items\": [");
    for (int i=0; i<m_names.size(); ++i) {
        if (i > 0)
            result += ", ";
        result += "\"" + m_names[i] + "\"";
    }
    result += "]}";
    return result;
}

QString RangeInputInfo::toString() const
{
    QString result = QString("{\"consumer\": \"%1\", \"type\": \"range\", \"items\": [");
    for (int i=0; i<m_ranges.size(); ++i) {
        if (i > 0)
            result += ", ";
        const Range& r = m_ranges[i];
        result += QString("{\"name\": \"%1\", \"vmin\": \"%2\", \"vmax\": \"%3\", \"resolution\": \"%4\"}")
            .arg(r.name, QString::number(r.vmin), QString::number(r.vmax), QString::number(r.resolution));
    }
    result += "]}";
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

int DefaultThreadManager::registerInput(InputInfo::Ptr inputInfo)
{
    int result = m_inputData.size();
    m_inputData << ThreadManagerInputData(inputInfo);
    return result;
}

QVector<double> DefaultThreadManager::readInput(int inputId, bool wait)
{
    // TODO
    Q_ASSERT(inputId >= 0   &&   inputId < m_inputData.size());
    ThreadManagerInputData::Buf& buf = m_inputData[inputId].buf;
    if (buf.isEmpty()) {
        if (wait) {
            // TODO: wait for input
            Q_ASSERT(false);
        }
        return QVector<double>();
    }
    else {
        QVector<double> result = buf.first();
        buf.removeFirst();
        return result;
    }
}
