/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "Rk4AdjustParamBox.h"
#include <cmath>

REGISTER_BOX(Rk4AdjustParamBox, "Rk4ParamAdjust")

Rk4AdjustParamBox::Rk4AdjustParamBox(QObject *parent) :
    Box(parent),
    m_step("h", this, PortFormat(1).setFixed()),
    m_stepCount("n", this, PortFormat(1).setFixed()),
    m_duration("T", this, PortFormat(1).setFixed()),
    m_outputControl("nout", this, PortFormat(1).setFixed()),
    m_rk4param("rk4param", this, PortFormat(3).setFixed())
{
}

InputPorts Rk4AdjustParamBox::inputPorts() const {
    return InputPorts() << &m_step << &m_stepCount << &m_duration << &m_outputControl;
}

OutputPorts Rk4AdjustParamBox::outputPorts() const {
    return OutputPorts() << &m_rk4param;
}

void Rk4AdjustParamBox::checkPortFormat() const
{
    if (m_step.format() != PortFormat(1))
        throwBoxException("Rk4AdjustParamBox: port 'h' has an invalid size");
    bool hasStepCount = m_stepCount.isConnected();
    bool hasDuration = m_duration.isConnected();
    if (hasStepCount == hasDuration)
        throwBoxException("Rk4AdjustParamBox: exactly one of ports port 'n', 'T' should be connected");
    if (hasDuration && m_duration.format() != PortFormat(1))
        throwBoxException("Rk4AdjustParamBox: port 't' has an invalid size");
    if (hasStepCount && m_stepCount.format() != PortFormat(1))
        throwBoxException("Rk4AdjustParamBox: port 'n' has an invalid size");
    if (m_outputControl.format() != PortFormat(1))
        throwBoxException("Rk4AdjustParamBox: port 'nout' has an invalid size");
    if (m_rk4param.format() != PortFormat(3))
        throwBoxException("Rk4AdjustParamBox: port 'rk4param' has an invalid size");
}

bool Rk4AdjustParamBox::propagatePortFormat() {
    return false;
}

RuntimeBox *Rk4AdjustParamBox::newRuntimeBox() const {
    return new Rk4AdjustParamRuntimeBox(this);
}



Rk4AdjustParamRuntimeBox::Rk4AdjustParamRuntimeBox(const Rk4AdjustParamBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    PortNotifier notifier = toPortNotifier(&Rk4AdjustParamRuntimeBox::processInput);
    m_step.init(this, in[0], notifier);
    m_stepCount.init(this, in[1], notifier);
    m_duration.init(this, in[2], notifier);
    m_outputControl.init(this, in[3], notifier);
    setInputPorts(RuntimeInputPorts() << &m_step << &m_stepCount << &m_duration << &m_outputControl);

    OutputPorts out = box->outputPorts();
    m_rk4param.init(this, out[0], PortData(3, m_data));
    setOutputPorts(RuntimeOutputPorts() << &m_rk4param);
}

bool Rk4AdjustParamRuntimeBox::processInput(int)
{
    bool hasStepCount = m_stepCount.isConnected();
    bool hasDuration = m_duration.isConnected();
    RuntimeOutputPort *stepPort = m_step.outputPort();
    RuntimeOutputPort *stepCountPort = hasStepCount? m_stepCount.outputPort(): 0;
    RuntimeOutputPort *durationPort = hasDuration? m_duration.outputPort(): 0;
    RuntimeOutputPort *outputControlPort = m_outputControl.outputPort();
    if (!(stepPort->state().hasData() && outputControlPort->state().hasData()))
        return false;
    if (hasDuration && !durationPort->state().hasData())
        return false;
    if (hasStepCount && !stepCountPort->state().hasData())
        return false;
    double h = stepPort->data().data()[0];
    double n;
    if (hasStepCount)
        n = stepCountPort->data().data()[0];
    else {
        double T = durationPort->data().data()[0];
        n = floor(T/h);
        if (n < 1)
            n = 1;
        h = T/n;
    }
    m_data[0] = h;
    m_data[1] = n;
    m_data[2] = outputControlPort->data().data()[0];
    m_rk4param.state().setValid();
    m_rk4param.activateLinks();
    return true;
}
