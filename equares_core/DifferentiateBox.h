#ifndef DIFFERENTIATEBOX_H
#define DIFFERENTIATEBOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT DifferentiateBox : public Box
{
    Q_OBJECT
public:
    explicit DifferentiateBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

private:
    mutable InputPort m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT DifferentiateRuntimeBox : public RuntimeBox
{
public:
    explicit DifferentiateRuntimeBox(const DifferentiateBox *box);
    void reset();

private:
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;
    int m_n;
    QVector<double> m_prevData;
    QVector<double> m_outData;
    bool m_prevDataValid;
    PortData m_buf;

    bool processData(int);
};

#endif // DIFFERENTIATEBOX_H
