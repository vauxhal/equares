#ifndef CONSTANTSOURCEBOX_H
#define CONSTANTSOURCEBOX_H

#include "equares_core.h"
#include <QScriptable>

class EQUARES_CORESHARED_EXPORT ConstantSourceBox :
    public Box
{
    Q_OBJECT
    Q_PROPERTY(QVector<double> data READ getData WRITE setData)
public:
    explicit ConstantSourceBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    double *data() const;

    const QVector<double>& getData() const;
    void setData(const QVector<double>& data);
    Port *getOut() const;

private:
    mutable OutputPort m_outputPort;
    mutable QVector<double> m_data;
};

class EQUARES_CORESHARED_EXPORT ConstantSourceRuntimeBox : public RuntimeBox
{
public:
    explicit ConstantSourceRuntimeBox(const ConstantSourceBox *box);

private:
    RuntimeOutputPort m_outputPort;
};

#endif // CONSTANTSOURCEBOX_H
