/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef MERGEBOX_H
#define MERGEBOX_H

#include "equares_core.h"
#include "equares_script.h"

class EQUARES_CORESHARED_EXPORT MergeBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(int inputPortCount READ inputPortCount WRITE setInputPortCount)
public:
    explicit MergeBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

    int inputPortCount() const;
    MergeBox& setInputPortCount(int inputPortCount);

private:
    void resizePorts(int n);

    mutable QVector<InputPort> m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT MergeRuntimeBox : public RuntimeBox
{
public:
    explicit MergeRuntimeBox(const MergeBox *box);

private:
    QVector<RuntimeInputPort> m_in;
    RuntimeOutputPort m_out;
    EntryCounter m_ec;

    bool processInput(int portId);
};

#endif // MERGEBOX_H
