/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef SIGNALINPUTBOX_H
#define SIGNALINPUTBOX_H

#include "equares_core.h"
#include "equares_script.h"

class EQUARES_CORESHARED_EXPORT SignalInputBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QString signalName READ signalName WRITE setSignalName)
public:
    explicit SignalInputBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    QString signalName() const;
    SignalInputBox& setSignalName(const QString& signalName);

private:
    QString m_signalName;
    mutable InputPort m_activator;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT SignalInputRuntimeBox : public RuntimeBox
{
public:
    explicit SignalInputRuntimeBox(const SignalInputBox *box);
    InputInfoList inputInfo() const;
    void registerInput();
    void acquireInteractiveInput();

private:
    RuntimeInputPort m_activator;
    RuntimeOutputPort m_out;

    QString m_signalName;
    int m_inputId;

    bool activate(int);
};

#endif // SIGNALINPUTBOX_H
