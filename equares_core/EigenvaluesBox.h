/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef EIGENVALUESBOX_H
#define EIGENVALUESBOX_H

#ifdef WITH_ACML

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT EigenvaluesBox : public Box
{
    Q_OBJECT
public:
    explicit EigenvaluesBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

private:
    mutable InputPort m_matrix;
    mutable OutputPort m_eigReal;
    mutable OutputPort m_eigImag;
};

class EQUARES_CORESHARED_EXPORT EigenvaluesRuntimeBox : public RuntimeBox
{
public:
    explicit EigenvaluesRuntimeBox(const EigenvaluesBox *box);

private:
    RuntimeInputPort m_matrix;
    RuntimeOutputPort m_eigReal;
    RuntimeOutputPort m_eigImag;

#ifdef WITH_ACML
    int m_n;
    QVector<double> m_eigr;
    QVector<double> m_eigi;
#endif // WITH_ACML

    bool computeEigenvalues(int);
};

#endif // WITH_ACML

#endif // EIGENVALUESBOX_H
