/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef INTERPOLATORBOX_H
#define INTERPOLATORBOX_H

#include "equares_core.h"
#include "equares_script.h"

class EQUARES_CORESHARED_EXPORT InterpolatorBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(int count READ count WRITE setCount)
public:
    explicit InterpolatorBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RuntimeBox *newRuntimeBox() const;

    int count() const;
    InterpolatorBox& setCount(int count);

private:
    mutable InputPort m_in;
    mutable OutputPort m_out;
    int m_count;
};

class EQUARES_CORESHARED_EXPORT InterpolatorRuntimeBox : public RuntimeBox
{
public:
    explicit InterpolatorRuntimeBox(const InterpolatorBox *box);

private:
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;

    QVector<double> m_buf[3];
    int m_n;
    int m_count;
    double *m_d1;
    double *m_d2;
    bool m_hasPrevData;

    bool processInput(int);
    void interp(double *dst, double t);
};

#endif // INTERPOLATORBOX_H
