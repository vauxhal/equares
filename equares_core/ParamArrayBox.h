#ifndef PARAMARRAYBOX_H
#define PARAMARRAYBOX_H

#include "equares_core.h"
#include "equares_script.h"

class EQUARES_CORESHARED_EXPORT ParamArrayBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QVector<double> data READ getData WRITE setData)
    Q_PROPERTY(bool withActivator READ withActivator WRITE setWithActivator)
public:
    explicit ParamArrayBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    double *data() const;

    const QVector<double>& getData() const;
    void setData(const QVector<double>& data);
    Port *getOut() const;

    bool withActivator() const;
    ParamArrayBox& setWithActivator(bool withActivator);

private:
    mutable InputPort m_activator;
    mutable OutputPort m_out;
    mutable OutputPort m_flush;
    mutable QVector<double> m_data;
    bool m_withActivator;
};

class EQUARES_CORESHARED_EXPORT ParamArrayRuntimeBox : public RuntimeBox
{
public:
    explicit ParamArrayRuntimeBox(const ParamArrayBox *box);
    PortNotifier generator() const;

private:
    RuntimeInputPort m_activator;
    RuntimeOutputPort m_out;
    RuntimeOutputPort m_flush;
    double *m_data;
    bool m_withActivator;
    int m_frameSize;
    int m_frameCount;
    bool generate(int);
};

#endif // PARAMARRAYBOX_H
