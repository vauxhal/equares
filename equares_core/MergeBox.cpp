/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "MergeBox.h"
#include "box_util.h"

REGISTER_BOX(MergeBox, "Merge")

MergeBox::MergeBox(QObject *parent) :
    Box(parent),
    m_out("output", this)
{
    resizePorts(2);
}

InputPorts MergeBox::inputPorts() const {
    InputPorts result;
    for (int i=0; i<m_in.size(); ++i)
        result << &m_in[i];
    return result;
}

OutputPorts MergeBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void MergeBox::checkPortFormat() const
{
    if (!m_out.format().isValid())
        throwBoxException("MergeBox: no format is specified for port 'output'");
    for (int i=0; i<m_in.size(); ++i)
        if (m_out.format() != m_in[i].format())
            throwBoxException("MergeBox: Incompatible input/output port formats");
}

bool MergeBox::propagatePortFormat()
{
    QList<Port*> ports;
    for (int i=0; i<m_in.size(); ++i)
        ports << &m_in[i];
    ports << &m_out;
    return propagateCommonFormat(ports);
}

RuntimeBox *MergeBox::newRuntimeBox() const {
    return new MergeRuntimeBox(this);
}

int MergeBox::inputPortCount() const {
    return m_in.size();
}

MergeBox& MergeBox::setInputPortCount(int inputPortCount)
{
    const int MaxInputPorts = 10;
    if (inputPortCount < 1   ||   inputPortCount > MaxInputPorts)
        throwBoxException(QString("MergeBox: output port count parameter is out of range, must be between 1 and %1").arg(MaxInputPorts));
    resizePorts(inputPortCount);
    return *this;
}

void MergeBox::resizePorts(int n)
{
    int n0 = m_in.size();
    m_in.resize(n);
    for (; n0<n; ++n0) {
        InputPort& p = m_in[n0];
        p.setName(QString("in_%1").arg(n0+1));
        p.setOwner(this);
    }
    if (context()) {
        QScriptValue jsThis = thisObject();
        addPortProperties(jsThis, inputPorts());
        // TODO better: Remove ports if new size is less than old size
    }
}



MergeRuntimeBox::MergeRuntimeBox(const MergeBox *box)
{
    setOwner(box);
    InputPorts in = box->inputPorts();
    m_in.resize(in.size());
    RuntimeInputPorts rtin;
    for (int i=0; i<in.size(); ++i) {
        m_in[i].init(this, in[i], toPortNotifier(&MergeRuntimeBox::processInput), i);
        rtin << &m_in[i];
    }
    setInputPorts(rtin);

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0]);
    setOutputPorts(RuntimeOutputPorts() << &m_out);
}

bool MergeRuntimeBox::processInput(int portId)
{
    ScopedInc incEc(m_ec);
    if (m_ec != 1)
        throw EquaresBoxException(owner(), "Recursive port activation is not supported for boxes of type 'Merge'");

    Q_ASSERT(portId >= 0   &&   portId < m_in.size());
    RuntimeInputPort& in = m_in[portId];
    Q_ASSERT(in.state().hasData());

    m_out.setData(in.data());
    m_out.state().setValid();
    return m_out.activateLinks();
}
