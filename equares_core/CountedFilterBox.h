/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef COUNTEDFILTERBOX_H
#define COUNTEDFILTERBOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT CountedFilterBox : public Box
{
    Q_OBJECT
public:
    explicit CountedFilterBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RuntimeBox *newRuntimeBox() const;

private:
    mutable InputPort m_count;
    mutable InputPort m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT CountedFilterRuntimeBox : public RuntimeBox
{
public:
    explicit CountedFilterRuntimeBox(const CountedFilterBox *box);

private:
    RuntimeInputPort m_count;
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;

    int m_c;
    int m_c0;
    bool m_hasCounter;

    bool setCount(int);
    bool processInput(int);
};

#endif // COUNTEDFILTERBOX_H
