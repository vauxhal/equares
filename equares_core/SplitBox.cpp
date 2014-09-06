/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "SplitBox.h"
#include "box_util.h"

REGISTER_BOX(SplitBox, "Split")

SplitBox::SplitBox(QObject *parent) :
    Box(parent),
    m_in("input", this)
{
    resizePorts(2);
}

InputPorts SplitBox::inputPorts() const {
    return InputPorts() << &m_in;
}

OutputPorts SplitBox::outputPorts() const {
    OutputPorts result;
    for (int i=0; i<m_out.size(); ++i)
        result << &m_out[i];
    return result;
}

void SplitBox::checkPortFormat() const
{
    if (!m_in.format().isValid())
        throwBoxException("SplitBox: no format is specified for port 'input'");
    for (int i=0; i<m_out.size(); ++i)
        if (m_in.format() != m_out[i].format())
            throwBoxException("SplitBox: Incompatible input/output port formats");
}

bool SplitBox::propagatePortFormat()
{
    QList<Port*> ports;
    ports << &m_in;
    for (int i=0; i<m_out.size(); ++i)
        ports << &m_out[i];
    return propagateCommonFormat(ports);
}

RuntimeBox *SplitBox::newRuntimeBox() const {
    return new SplitRuntimeBox(this);
}

int SplitBox::outputPortCount() const {
    return m_out.size();
}

SplitBox& SplitBox::setOutputPortCount(int outputPortCount)
{
    const int MaxOutputPorts = 10;
    if (outputPortCount < 1   ||   outputPortCount > MaxOutputPorts)
        throwBoxException(QString("SplitBox: output port count parameter is out of range, must be between 1 and %1").arg(MaxOutputPorts));
    resizePorts(outputPortCount);
    return *this;
}

void SplitBox::resizePorts(int n)
{
    int n0 = m_out.size();
    m_out.resize(n);
    for (; n0<n; ++n0) {
        OutputPort& p = m_out[n0];
        p.setName(QString("out_%1").arg(n0+1));
        p.setOwner(this);
    }
    if (context()) {
        QScriptValue jsThis = thisObject();
        addPortProperties(jsThis, outputPorts());
        // TODO better: Remove ports if new size is less than old size
    }
}



SplitRuntimeBox::SplitRuntimeBox(const SplitBox *box)
{
    setOwner(box);
    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(&SplitRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_in);

    OutputPorts out = box->outputPorts();
    m_out.resize(out.size());
    RuntimeOutputPorts rtout;
    for (int i=0; i<out.size(); ++i) {
        m_out[i].init(this, out[i]);
        rtout << &m_out[i];
    }
    setOutputPorts(rtout);
}

bool SplitRuntimeBox::processInput(int)
{
    Q_ASSERT(m_in.state().hasData());
    const PortData& data = m_in.data();
    for (int i=0; i<m_out.size(); ++i) {
        RuntimeOutputPort& p = m_out[i];
        p.setData(data);
        p.state().setValid();
        if (!p.activateLinks())
            return false;
    }
    return true;
}
