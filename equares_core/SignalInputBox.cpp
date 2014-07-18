/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "SignalInputBox.h"

REGISTER_BOX(SignalInputBox, "SignalInput")



SignalInputBox::SignalInputBox(QObject *parent) :
    Box(parent),
    m_activator("activator", this),
    m_out("output", this, PortFormat(0).setFixed())
{
}

InputPorts SignalInputBox::inputPorts() const {
    return InputPorts() << &m_activator;
}

OutputPorts SignalInputBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void SignalInputBox::checkPortFormat() const
{
}

bool SignalInputBox::propagatePortFormat() {
    return false;
}

RuntimeBox *SignalInputBox::newRuntimeBox() const {
    return new SignalInputRuntimeBox(this);
}

QString SignalInputBox::signalName() const {
    return m_signalName;
}

SignalInputBox& SignalInputBox::setSignalName(const QString& signalName) {
    m_signalName = signalName;
    return *this;
}



SignalInputRuntimeBox::SignalInputRuntimeBox(const SignalInputBox *box) :
    m_signalName(box->signalName())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_activator.init(this, in[0], toPortNotifier(&SignalInputRuntimeBox::activate));
    setInputPorts(RuntimeInputPorts() << &m_activator);

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0]);
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_inputId = -1;
}

InputInfoList SignalInputRuntimeBox::inputInfo() const {
    return InputInfoList() << InputInfo::Ptr(new SignalInputInfo(owner()->decoratedName(), m_signalName));
}

void SignalInputRuntimeBox::registerInput()
{
    m_inputId = ThreadManager::instance()->registerInput(inputInfo()[0]);
}

void SignalInputRuntimeBox::acquireInteractiveInput()
{
    QVector<double> input;
    if (ThreadManager::instance()->readInput(input, m_inputId, false))
        m_out.activateLinks();
}

bool SignalInputRuntimeBox::activate(int)
{
    acquireInteractiveInput();
    return true;
}
