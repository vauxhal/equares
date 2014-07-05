#ifndef SPLITBOX_H
#define SPLITBOX_H

#include "equares_core.h"
#include "equares_script.h"

class EQUARES_CORESHARED_EXPORT SplitBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(int outputPortCount READ outputPortCount WRITE setOutputPortCount)
public:
    explicit SplitBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

    int outputPortCount() const;
    SplitBox& setOutputPortCount(int outputPortCount);

private:
    void resizePorts(int n);

    mutable InputPort m_in;
    mutable QVector<OutputPort> m_out;
};

class EQUARES_CORESHARED_EXPORT SplitRuntimeBox : public RuntimeBox
{
public:
    explicit SplitRuntimeBox(const SplitBox *box);

private:
    RuntimeInputPort m_in;
    QVector<RuntimeOutputPort> m_out;

    bool processInput(int);
};

#endif // SPLITBOX_H
