/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef RK4BOX_H
#define RK4BOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT Rk4Box : public Box
{
    Q_OBJECT
public:
    explicit Rk4Box(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RuntimeBox *newRuntimeBox() const;

private:
    mutable InputPort m_param;
    mutable InputPort m_initState;
    mutable InputPort m_rhs;
    mutable InputPort m_terminator;
    mutable OutputPort m_nextState;
    mutable OutputPort m_rhsState;
    mutable OutputPort m_finish;
};

class EQUARES_CORESHARED_EXPORT Rk4RuntimeBox : public RuntimeBox
{
public:
    explicit Rk4RuntimeBox(const Rk4Box *box);

private:
    RuntimeInputPort m_param;
    RuntimeInputPort m_initState;
    RuntimeInputPort m_rhs;
    RuntimeInputPort m_terminator;
    RuntimeOutputPort m_nextState;
    RuntimeOutputPort m_rhsState;
    RuntimeOutputPort m_finish;
    bool m_terminationRequested;

    bool start(int);
    bool stop(int);

    int m_n;
    QVector<double> m_nextData;
    QVector<double> m_k[4];
    QVector<double> m_buf;
    void lincomb(double *dst, const double *src1, const double *src2, double factor2) const;
};

#endif // RK4BOX_H
