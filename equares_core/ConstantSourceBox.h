/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

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
    virtual void setData(const QVector<double>& data);
    Port *getOut() const;

protected:
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
