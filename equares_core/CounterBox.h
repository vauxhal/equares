#ifndef COUNTERBOX_H
#define COUNTERBOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT CounterBox : public Box
{
    Q_OBJECT
public:
    explicit CounterBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

private:
    mutable InputPort m_in;
    mutable InputPort m_reset;
    mutable OutputPort m_count;
};

class EQUARES_CORESHARED_EXPORT CounterRuntimeBox : public RuntimeBox
{
public:
    explicit CounterRuntimeBox(const CounterBox *box);

private:
    RuntimeInputPort m_in;
    RuntimeInputPort m_reset;
    RuntimeOutputPort m_count;

    double m_countData;

    bool processData(int);
    bool resetCounter(int);
};

#endif // COUNTERBOX_H
