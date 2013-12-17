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
    QString m_fileName;
};

class EQUARES_CORESHARED_EXPORT DumpRuntimeBox : public RuntimeBox
{
public:
    explicit DumpRuntimeBox(const DumpBox *box);
    ~DumpRuntimeBox();

private:
    RuntimeInputPort m_dump;

    bool dump();

    FILE *m_cfile;
};

#endif // DUMPBOX_H
