/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "DataInputBox.h"
#include "box_util.h"

DataInputBox::DataInputBox(QObject *parent) :
    Box(parent),
    m_activator("activator", this),
    m_in("input", this),
    m_out("output", this),
    m_restartOnInput(false),
    m_activateBeforeRestart(false)
{
}

InputPorts DataInputBox::inputPorts() const {
    return InputPorts() << &m_activator << &m_in;
}

OutputPorts DataInputBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void DataInputBox::checkPortFormat() const
{
    if (!m_in.format().isValid())
        throwBoxException("DataInputBox: no format is specified for port 'input'");
    if (m_in.format().dimension() != 1)
        throwBoxException("DataInputBox: an 1D format was expected for port 'input'");
    if (m_in.format() != m_out.format())
        throwBoxException("DataInputBox: Incompatible input/output port formats");
}

bool DataInputBox::propagatePortFormat() {
    return propagateCommonFormat(m_in, m_out);
}

bool DataInputBox::restartOnInput() const {
    return m_restartOnInput;
}

DataInputBox& DataInputBox::setRestartOnInput(bool restartOnInput) {
    m_restartOnInput = restartOnInput;
    return *this;
}

bool DataInputBox::activateBeforeRestart() const {
    return m_activateBeforeRestart;
}
DataInputBox& DataInputBox::setActivateBeforeRestart(bool activateBeforeRestart) {
    m_activateBeforeRestart = activateBeforeRestart;
    return *this;
}



DataInputRuntimeBox::DataInputRuntimeBox(const DataInputBox *box) :
    m_restartOnInput(box->restartOnInput()),
    m_activateBeforeRestart(box->activateBeforeRestart())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_activator.init(this, in[0], toPortNotifier(&DataInputRuntimeBox::activate));
    m_in.init(this, in[1], toPortNotifier(&DataInputRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_activator << &m_in);

    OutputPorts out = box->outputPorts();
    m_data.resize(out[0]->format().dataSize());
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_inputId = -1;
    m_dataValid = false;
    m_iinputDataValid = false;
    m_unititializedInputPort = true;
}

void DataInputRuntimeBox::registerInput()
{
    m_inputId = ThreadManager::instance()->registerInput(inputInfo()[0]);
}

void DataInputRuntimeBox::acquireInteractiveInput()
{
    if (!fetchInputPortData())
        return;
    if (ThreadManager::instance()->readInput(m_iinputData, m_inputId, false)) {
        transformData(m_data.data(), m_iinputData.data());
        m_iinputDataValid = true;
        if (m_restartOnInput && m_activateBeforeRestart)
            m_out.activateLinks();
        throw BoxBreakException(m_restartOnInput? 0: this);
    }
}

bool DataInputRuntimeBox::fetchInputPortData()
{
    if (m_dataValid)
        return true;
    if (!m_in.state().hasData())
        return false;
    Q_ASSERT(m_in.data().size() == m_data.size());
    m_in.data().copyTo(m_data.data());
    m_out.state().setValid();
    m_dataValid = true;

    // Report input data corresponding to new port data
    if (m_unititializedInputPort) {
        m_unititializedInputPort = false;
        QStringList input;
        foreach (double d, inputData(m_data.data()))
            input << QString::number(d);
        EQUARES_COUT << "input: " << owner()->decoratedName() << " " << input.join(" ") << endl;
    }
    return true;
}

bool DataInputRuntimeBox::activate(int)
{
    acquireInteractiveInput();
    return true;
}

bool DataInputRuntimeBox::processInput(int)
{
    Q_ASSERT(m_in.state().hasData());
    m_dataValid = false;
    if (fetchInputPortData()) {
        if (m_iinputDataValid)
            transformData(m_data.data(), m_iinputData.data());
        return m_out.activateLinks();
    }
    else
        return false;
}
