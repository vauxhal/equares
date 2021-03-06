/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "ReplicatorBox.h"
#include "box_util.h"

REGISTER_BOX(ReplicatorBox, "Replicator")

ReplicatorBox::ReplicatorBox(QObject *parent) :
    Box(parent),
    m_controlIn("control_in", this),
    m_valueIn("value_in", this),
    m_controlOut("control_out", this),
    m_valueOut("value_out", this)
{
}

InputPorts ReplicatorBox::inputPorts() const {
    return InputPorts() << &m_controlIn << &m_valueIn;
}

OutputPorts ReplicatorBox::outputPorts() const {
    return OutputPorts() << &m_controlOut << &m_valueOut;
}

void ReplicatorBox::checkPortFormat() const
{
    if (!m_controlIn.format().isValid())
        throwBoxException("ReplicatorBox: no format is specified for port 'control_in'");
    if (!m_valueIn.format().isValid())
        throwBoxException("ReplicatorBox: no format is specified for port 'value_in'");
    if (!m_controlOut.format().isValid())
        throwBoxException("ReplicatorBox: no format is specified for port 'control_out'");
    if (!m_valueOut.format().isValid())
        throwBoxException("ReplicatorBox: no format is specified for port 'value_out'");
    if (m_controlOut.format() != m_controlIn.format())
        throwBoxException("ReplicatorBox: different formats of ports 'control_in' and 'control_out'");
    if (m_valueOut.format() != m_valueIn.format())
        throwBoxException("ReplicatorBox: different formats of ports 'value_in' and 'value_out'");
}

bool ReplicatorBox::propagatePortFormat()
{
    bool result = false;
    result = propagateCommonFormat(m_controlIn, m_controlOut) || result;
    result = propagateCommonFormat(m_valueIn, m_valueOut) || result;
    return result;
}

RuntimeBox *ReplicatorBox::newRuntimeBox() const {
    return new ReplicatorRuntimeBox(this);
}



ReplicatorRuntimeBox::ReplicatorRuntimeBox(const ReplicatorBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_controlIn.init(this, in[0], toPortNotifier(&ReplicatorRuntimeBox::control));
    m_valueIn.init(this, in[1]);
    setInputPorts(RuntimeInputPorts() << &m_controlIn << &m_valueIn);

    OutputPorts out = box->outputPorts();
    m_controlOut.init(this, out[0]);
    m_valueOut.init(this, out[1]);
    setOutputPorts(RuntimeOutputPorts() << &m_controlOut << &m_valueOut);
}

bool ReplicatorRuntimeBox::control(int)
{
    if (!m_valueIn.state().hasData())
        return false;

    if(m_controlIn.state().hasData()) {
        m_controlOut.setData(m_controlIn.data());
        m_controlOut.state().setValid();
    }
    if (!m_controlOut.activateLinks())
        return false;

    m_valueOut.setData(m_valueIn.data());
    m_valueOut.state().setValid();
    return m_valueOut.activateLinks();
}
