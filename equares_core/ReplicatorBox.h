/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef REPLICATORBOX_H
#define REPLICATORBOX_H

#include "equares_core.h"
#include "equares_script.h"

class EQUARES_CORESHARED_EXPORT ReplicatorBox : public Box
{
    Q_OBJECT
public:
    ReplicatorBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

private:
    mutable InputPort m_controlIn;
    mutable InputPort m_valueIn;
    mutable OutputPort m_controlOut;
    mutable OutputPort m_valueOut;
};

class EQUARES_CORESHARED_EXPORT ReplicatorRuntimeBox : public RuntimeBox
{
public:
    explicit ReplicatorRuntimeBox(const ReplicatorBox *box);

private:
    RuntimeInputPort m_controlIn;
    RuntimeInputPort m_valueIn;
    RuntimeOutputPort m_controlOut;
    RuntimeOutputPort m_valueOut;

    bool control(int);
};

#endif // REPLICATORBOX_H
