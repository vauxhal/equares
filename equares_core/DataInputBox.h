#ifndef DATAINPUTBOX_H
#define DATAINPUTBOX_H

#include "equares_core.h"
#include "equares_script.h"
#include <QTime>

class EQUARES_CORESHARED_EXPORT DataInputBox : public Box
{
    Q_OBJECT
public:

    explicit DataInputBox(QObject *parent = 0);
    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

private:
    mutable InputPort m_activator;
    mutable InputPort m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT DataInputRuntimeBox : public RuntimeBox
{
public:
    explicit DataInputRuntimeBox(const DataInputBox *box);
    void registerInput();
    void acquireInteractiveInput();

protected:
    virtual void transformData(double *portData, const double *inputData) const = 0;
    virtual QVector<double> inputData(const double *portData) const = 0;

private:
    RuntimeInputPort m_activator;
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;

    QVector<double> m_data;
    bool m_dataValid;
    bool fetchInputPortData();
    QVector<double> m_iinputData;
    bool m_iinputDataValid;
    int m_inputId;
    QTime m_inputFeedbackTime;
    bool m_unititializedInputPort;

    bool activate(int);
    bool processInput(int);
};

#endif // DATAINPUTBOX_H
