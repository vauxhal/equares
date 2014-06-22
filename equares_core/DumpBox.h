#ifndef DUMPBOX_H
#define DUMPBOX_H

#include "equares_core.h"
#include <cstdio>

class EQUARES_CORESHARED_EXPORT DumpBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)

public:
    explicit DumpBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RuntimeBox *newRuntimeBox() const;

    QString fileName() const;
    DumpBox& setFileName(const QString& fileName);
private:
    mutable InputPort m_dump;
    mutable InputPort m_flush;
    QString m_fileName;
};

class EQUARES_CORESHARED_EXPORT DumpRuntimeBox : public RuntimeBox
{
public:
    explicit DumpRuntimeBox(const DumpBox *box);
    ~DumpRuntimeBox();
    OutputFileInfoList outputFileInfo() const;
    void reset();

private:
    RuntimeInputPort m_dump;
    RuntimeInputPort m_flush;

    enum { DataLimit = 1000000 };
    bool dump();
    bool flush();
    QString fileName() const;

    FILE *m_cfile;
    int m_totalDataWritten;
};

#endif // DUMPBOX_H
