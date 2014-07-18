/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef FDEITERATORBOX_H
#define FDEITERATORBOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT FdeIteratorBox : public Box
{
    Q_OBJECT
public:
    explicit FdeIteratorBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

private:
    mutable InputPort m_param;
    mutable InputPort m_initState;
    mutable InputPort m_fdeIn;
    mutable InputPort m_terminator;
    mutable OutputPort m_nextState;
    mutable OutputPort m_fdeOut;
    mutable OutputPort m_finish;
};

class EQUARES_CORESHARED_EXPORT FdeIteratorRuntimeBox : public RuntimeBox
{
public:
    explicit FdeIteratorRuntimeBox(const FdeIteratorBox *box);

private:
    RuntimeInputPort m_param;
    RuntimeInputPort m_initState;
    RuntimeInputPort m_fdeIn;
    RuntimeInputPort m_terminator;
    RuntimeOutputPort m_nextState;
    RuntimeOutputPort m_fdeOut;
    RuntimeOutputPort m_finish;
    bool m_terminationRequested;

    bool start(int);
    bool stop(int);

    int m_n;
    QVector<double> m_buf;
};

#endif // FDEITERATORBOX_H
