#ifndef BITMAPBOX_H
#define BITMAPBOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT BitmapBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)

public:
    explicit BitmapBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RuntimeBox *newRuntimeBox() const;

    QString fileName() const;
    BitmapBox& setFileName(const QString& fileName);
private:
    enum { ResolutionLimit = 2000 };
    mutable InputPort m_dump;
    QString m_fileName;
};

class EQUARES_CORESHARED_EXPORT BitmapRuntimeBox : public RuntimeBox
{
public:
    explicit BitmapRuntimeBox(const BitmapBox *box);
    OutputFileInfoList outputFileInfo() const;

private:
    RuntimeInputPort m_dump;
    QString m_fileName;

    bool dump();
};

#endif // BITMAPBOX_H
