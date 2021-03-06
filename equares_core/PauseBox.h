/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef PAUSEBOX_H
#define PAUSEBOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT PauseBox : public Box
{
    Q_OBJECT
public:

    explicit PauseBox(QObject *parent = 0);
    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

private:
    mutable InputPort m_activator;
};

class EQUARES_CORESHARED_EXPORT PauseRuntimeBox : public RuntimeBox
{
public:
    explicit PauseRuntimeBox(const PauseBox *box);
    PortNotifier postprocessor() const;

private:
    RuntimeInputPort m_activator;

    bool activate(int);
};

#endif // PAUSEBOX_H
