#ifndef DATAINPUTBOX_H
#define DATAINPUTBOX_H

#include "equares_core.h"
#include "equares_script.h"
#include <QTime>

class EQUARES_CORESHARED_EXPORT DataInputBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(bool sync READ sync WRITE setSync)
    Q_PROPERTY(bool loop READ loop WRITE setLoop)
public:

    explicit DataInputBox(QObject *parent = 0);
    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    bool sync() const;
    DataInputBox& setSync(bool sync);
    bool loop() const;
    DataInputBox& setLoop(bool loop);

private:
    bool m_sync;
    bool m_loop;

    mutable InputPort m_activator;
    mutable InputPort m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT DataInputRuntimeBox : public RuntimeBox
{
public:
    explicit DataInputRuntimeBox(const DataInputBox *box);
    void registerInput();

protected:
    virtual void transformData(double *portData, const double *inputData) const = 0;
    virtual QVector<double> inputData(const double *portData) const = 0;

private:
    RuntimeInputPort m_activator;
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;

    bool m_sync;
    bool m_loop;
    QVector<double> m_data;
    bool m_dataValid;
    bool fetchInputPortData();
    QVector<double> m_iinputData;
    bool m_iinputDataValid;
    int m_inputId;
    EntryCounter m_ec;
    QTime m_time;
    bool m_unititializedInputPort;

    bool activate(int);
    bool processInput(int);
};

#endif // DATAINPUTBOX_H
