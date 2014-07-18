/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "ParamArrayBox.h"

REGISTER_BOX(ParamArrayBox, "ParamArray")

ParamArrayBox::ParamArrayBox(QObject *parent) :
    Box(parent),
    m_activator("activator", this),
    m_out("output", this),
    m_flush("flush", this, PortFormat(0).setFixed()),
    m_withActivator(false)
{
}

InputPorts ParamArrayBox::inputPorts() const {
    InputPorts result;
    if (m_withActivator)
        result << &m_activator;
    return result;
}

OutputPorts ParamArrayBox::outputPorts() const {
    return OutputPorts() << &m_out << &m_flush;
}

RuntimeBox *ParamArrayBox::newRuntimeBox() const {
    return new ParamArrayRuntimeBox(this);
}

void ParamArrayBox::checkPortFormat() const
{
}

bool ParamArrayBox::propagatePortFormat()
{
    return false;
}

double *ParamArrayBox::data() const
{
    int frameSize = m_out.format().dataSize();
    if (m_data.size() % frameSize != 0) {
        int n = m_data.size() / frameSize + 1;
        m_data.resize(n * frameSize);
    }
    return m_data.data();
}

const QVector<double>& ParamArrayBox::getData() const {
    data();
    return m_data;
}

void ParamArrayBox::setData(const QVector<double>& data) {
    m_data = data;
}

Port *ParamArrayBox::getOut() const {
    return &m_out;
}

bool ParamArrayBox::withActivator() const {
    return m_withActivator;
}

ParamArrayBox& ParamArrayBox::setWithActivator(bool withActivator) {
    m_withActivator = withActivator;
    if (context()) {
        QScriptValue jsThis = thisObject();
        if (m_withActivator)
            addPortProperties(jsThis, inputPorts());
        else
            // TODO better
            jsThis.setProperty("activator", QScriptValue());
    }
    return *this;
}



ParamArrayRuntimeBox::ParamArrayRuntimeBox(const ParamArrayBox *box) :
    m_withActivator(box->withActivator())
{
    setOwner(box);

    if (m_withActivator) {
        InputPorts in = box->inputPorts();
        m_activator.init(this, in[0], toPortNotifier(&ParamArrayRuntimeBox::generate));
        setInputPorts(RuntimeInputPorts() << &m_activator);
    }

    OutputPorts out = box->outputPorts();
    m_frameSize = out[0]->format().dataSize();
    m_frameCount = m_frameSize > 0 ?   box->getData().size() / m_frameSize :   0;
    m_data = box->data();
    m_out.init(this, out[0]);
    m_flush.init(this, out[1]);
    setOutputPorts(RuntimeOutputPorts() << &m_out << &m_flush);
}

RuntimeBox::PortNotifier ParamArrayRuntimeBox::generator() const
{
    if (m_withActivator)
        return 0;
    else
        return toPortNotifier(&ParamArrayRuntimeBox::generate);
}

bool ParamArrayRuntimeBox::generate(int)
{
    double *data = m_data;
    for (int i=0; i<m_frameCount; ++i, data += m_frameSize) {
        m_out.setData(PortData(m_frameSize, data));
        m_out.state().setValid();
        if (!m_out.activateLinks())
            return false;
    }
    return m_flush.activateLinks();
}
