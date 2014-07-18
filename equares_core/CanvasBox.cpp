/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "CanvasBox.h"

REGISTER_BOX(CanvasBox, "Canvas")



static QScriptValue canvasDimParamToScriptValue(QScriptEngine *e, const CanvasBoxDimParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("vmin", p.vmin);
    result.setProperty("vmax", p.vmax);
    result.setProperty("resolution", p.resolution);
    return result;
}

static void canvasDimParamFromScriptValue(const QScriptValue& v, CanvasBoxDimParam& result) {
    result = CanvasBoxDimParam();
    result.vmin = v.property("vmin").toNumber();
    result.vmax = v.property("vmax").toNumber();
    result.resolution = v.property("resolution").toInt32();
}

static QScriptValue canvasParamToScriptValue(QScriptEngine *e, const CanvasBoxParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("x", canvasDimParamToScriptValue(e, p[0]));
    result.setProperty("y", canvasDimParamToScriptValue(e, p[1]));
    return result;
}

static void canvasParamFromScriptValue(const QScriptValue& v, CanvasBoxParam& result) {
    result = CanvasBoxParam();
    canvasDimParamFromScriptValue(v.property("x"), result[0]);
    canvasDimParamFromScriptValue(v.property("y"), result[1]);
}

static void canvasParamScriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, canvasDimParamToScriptValue, canvasDimParamFromScriptValue);
    qScriptRegisterMetaType(e, canvasParamToScriptValue, canvasParamFromScriptValue);
}

REGISTER_SCRIPT_INIT_FUNC(canvasParamScriptInit)



CanvasBox::CanvasBox(QObject *parent) :
    Box(parent),
    m_refreshInterval(0),
    m_timeCheckCount(100),
    m_clearOnRestart(false),
    m_withInputValue(false),
    m_in("input", this),
    m_flush("flush", this),
    m_clear("clear", this),
    m_range("range", this, PortFormat(4).setFixed()),
    m_out("output", this, PortFormat(m_param[0].resolution, m_param[1].resolution).setFixed())
{
}

InputPorts CanvasBox::inputPorts() const {
    return InputPorts() << &m_in << &m_flush << &m_clear << &m_range;
}

OutputPorts CanvasBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void CanvasBox::checkPortFormat() const
{
    if (m_in.format() != PortFormat(m_withInputValue? 3: 2))
        throwBoxException("CanvasBox: Invalid input port format");
    if (m_out.format() != PortFormat(m_param[0].resolution, m_param[1].resolution))
        throwBoxException("CanvasBox: Invalid output port format");
}

bool CanvasBox::propagatePortFormat() {
    return false;
}

RuntimeBox *CanvasBox::newRuntimeBox() const {
    return new CanvasRuntimeBox(this);
}

CanvasBox::Param CanvasBox::param() const {
    return m_param;
}

CanvasBox& CanvasBox::setParam(const Param& param) {
    if (param[0].resolution <= 0   ||   param[0].resolution > ResolutionLimit ||
        param[1].resolution <= 0   ||   param[1].resolution > ResolutionLimit)
        throwBoxException(QString("CanvasBox: Unable to set parameter: invalid resolution - should be in range [1,%1]").arg(ResolutionLimit));
    m_param = param;
    m_out.format().setSize(0, param[0].resolution);
    m_out.format().setSize(1, param[1].resolution);
    return *this;
}

int CanvasBox::refreshInterval() const {
    return m_refreshInterval;
}

CanvasBox& CanvasBox::setRefreshInterval(int refreshInterval) {
    m_refreshInterval = refreshInterval;
    return *this;
}

int CanvasBox::timeCheckCount() const {
    return m_timeCheckCount;
}
CanvasBox& CanvasBox::setTimeCheckCount(int timeCheckCount) {
    m_timeCheckCount = timeCheckCount;
    return *this;
}

bool CanvasBox::clearOnRestart() const {
    return m_clearOnRestart;
}

CanvasBox& CanvasBox::setClearOnRestart(bool clearOnRestart) {
    m_clearOnRestart = clearOnRestart;
    return *this;
}

bool CanvasBox::withInputValue() const {
    return m_withInputValue;
}

CanvasBox& CanvasBox::setWithInputValue(bool withInputValue) {
    m_withInputValue = withInputValue;
    return *this;
}



CanvasRuntimeBox::CanvasRuntimeBox(const CanvasBox *box) :
    m_param(box->param()),
    m_refreshInterval(box->refreshInterval()),
    m_timeCheckCount(box->timeCheckCount()),
    m_clearOnRestart(box->clearOnRestart())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(
        box->withInputValue() ?
            &CanvasRuntimeBox::processInputWithValue :
            &CanvasRuntimeBox::processInputNoValue));
    m_flush.init(this, in[1], PortNotifier(&CanvasRuntimeBox::flush));
    m_clear.init(this, in[2], PortNotifier(&CanvasRuntimeBox::clear));
    m_range.init(this, in[3], PortNotifier(&CanvasRuntimeBox::setRange));
    m_rangeValid = in[3]->link() == 0;
    setInputPorts(RuntimeInputPorts() << &m_in << &m_flush << &m_clear << &m_range);

    OutputPorts out = box->outputPorts();
    m_data = QVector<double>(out[0]->format().dataSize(), 0);
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_timeCheckCounter = 0;
    m_time.start();
}

void CanvasRuntimeBox::reset() {
    clear(0);
}

void CanvasRuntimeBox::restart() {
    if (m_clearOnRestart)
        clear(0);
}

bool CanvasRuntimeBox::processInputNoValue(int)
{
    if (!m_rangeValid)
        return false;
    Q_ASSERT(m_in.state().hasData());
    int i = m_param.index(m_in.data().data());
    if (i >= 0) {
        double *data = m_out.data().data();
        data[i] = 1;
        if (++m_timeCheckCounter >= m_timeCheckCount) {
            if (m_refreshInterval > 0   &&   m_time.elapsed() >= m_refreshInterval) {
                flush(0);
                m_time.restart();
            }
            m_timeCheckCounter = 0;
        }
    }
    return true;
}

bool CanvasRuntimeBox::processInputWithValue(int)
{
    if (!m_rangeValid)
        return false;
    Q_ASSERT(m_in.state().hasData());
    const double *dataIn = m_in.data().data();
    int i = m_param.index(dataIn);
    if (i >= 0) {
        double *dataOut = m_out.data().data();
        dataOut[i] = dataIn[2];
        if (++m_timeCheckCounter >= m_timeCheckCount) {
            if (m_refreshInterval > 0   &&   m_time.elapsed() >= m_refreshInterval) {
                flush(0);
                m_time.restart();
            }
            m_timeCheckCounter = 0;
        }
    }
    return true;
}

bool CanvasRuntimeBox::flush(int)
{
    m_out.state().setValid();
    return m_out.activateLinks();
}

bool CanvasRuntimeBox::clear(int)
{
    m_data.fill(0);
    return true;
}

bool CanvasRuntimeBox::setRange(int)
{
    Q_ASSERT(m_range.state().hasData());
    PortData rd = m_range.data();
    Q_ASSERT((rd.size() & 1) == 0);
    int n = rd.size() >> 1;
    if (n > 2)
        n = 2;
    const double *d = rd.data();
    for (int i=0; i<n; ++i, d+=2) {
        m_param[i].vmin = d[0];
        m_param[i].vmax = d[1];
    }
    m_rangeValid = true;
    clear(0);
    return true;
}
