#ifndef SIGNALINPUTBOX_H
#define SIGNALINPUTBOX_H

#include "equares_core.h"
#include "equares_script.h"

class EQUARES_CORESHARED_EXPORT SignalInputBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QString signalName READ signalName WRITE setSignalName)
    Q_PROPERTY(bool sync READ sync WRITE setSync)
    Q_PROPERTY(bool loop READ loop WRITE setLoop)
public:
    explicit SignalInputBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    QString signalName() const;
    SignalInputBox& setSignalName(const QString& signalName);
    bool sync() const;
    SignalInputBox& setSync(bool sync);
    bool loop() const;
    SignalInputBox& setLoop(bool loop);

private:
    QString m_signalName;
    bool m_sync;
    bool m_loop;
    mutable InputPort m_activator;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT SignalInputRuntimeBox : public RuntimeBox
{
public:
    explicit SignalInputRuntimeBox(const SignalInputBox *box);
    InputInfoList inputInfo() const;
    void registerInput();

private:
    RuntimeInputPort m_activator;
    RuntimeOutputPort m_out;

    QString m_signalName;
    bool m_sync;
    bool m_loop;
    bool m_iinputDataValid;
    bool m_inputDataEmpty;
    int m_inputId;

    bool activate();
};

#endif // SIGNALINPUTBOX_H
