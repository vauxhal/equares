#ifndef VALVEBOX_H
#define VALVEBOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT ValveBox : public Box
{
    Q_OBJECT
public:
    explicit ValveBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RuntimeBox *newRuntimeBox() const;

private:
    mutable InputPort m_valve;
    mutable InputPort m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT ValveRuntimeBox : public RuntimeBox
{
public:
    explicit ValveRuntimeBox(const ValveBox *box);
    void restart();

private:
    RuntimeInputPort m_valve;
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;

    bool m_hasValve;
    bool m_valveOpen;
    bool m_hasData;
    QVector<double> m_data;

    bool setValve(int);
    bool setInput(int);

    bool processData();
};

#endif // VALVEBOX_H
