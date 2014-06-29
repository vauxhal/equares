#include "RectInputBox.h"

REGISTER_BOX(RectInputBox, "RectInput")



template<class T> QScriptValue toScriptValue(QScriptEngine *e, const T& p);
template<class T> void fromScriptValue(const QScriptValue& v, T& result);

template<>
QScriptValue toScriptValue(QScriptEngine *e, const RectInputBoxRange& p) {
    QScriptValue result = e->newObject();
    result.setProperty("xmin", p.xmin);
    result.setProperty("xmax", p.xmax);
    result.setProperty("ymin", p.ymin);
    result.setProperty("ymax", p.ymax);
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, RectInputBoxRange& result) {
    result = RectInputBoxRange();
    result.xmin = v.property("xmin").toNumber();
    result.xmax = v.property("xmax").toNumber();
    result.ymin = v.property("ymin").toNumber();
    result.ymax = v.property("ymax").toNumber();
}

static void scriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, toScriptValue<RectInputBoxRange>, fromScriptValue<RectInputBoxRange>);
}

REGISTER_SCRIPT_INIT_FUNC(scriptInit)



RectInputBox::RectInputBox(QObject *parent) :
    Box(parent),
    m_activator("activator", this),
    m_out("output", this, PortFormat(4).setFixed()),
    m_keepAspectRatio(false),
    m_withActivator(false)
{
}

InputPorts RectInputBox::inputPorts() const {
    InputPorts result;
    if (m_withActivator)
        result << &m_activator;
    return result;
}

OutputPorts RectInputBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void RectInputBox::checkPortFormat() const
{
}

bool RectInputBox::propagatePortFormat() {
    return false;
}

RuntimeBox *RectInputBox::newRuntimeBox() const {
    return new RectInputRuntimeBox(this);
}

RectInputBoxRange RectInputBox::initRect() const {
    return m_initRect;
}

RectInputBox& RectInputBox::setInitRect(const RectInputBoxRange& initRect) {
    m_initRect = initRect;
    return *this;
}

bool RectInputBox::keepAspectRatio() const {
    return m_keepAspectRatio;
}

RectInputBox& RectInputBox::setKeepAspectRatio(bool keepAspectRatio) {
    m_keepAspectRatio = keepAspectRatio;
    return *this;
}

bool RectInputBox::withActivator() const {
    return m_withActivator;
}

RectInputBox& RectInputBox::setWithActivator(bool withActivator) {
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

QString RectInputBox::refBitmap() const {
    return m_refBitmap;
}

RectInputBox& RectInputBox::setRefBitmap(const QString& refBitmap) {
    m_refBitmap = refBitmap;
    return *this;
}



RectInputRuntimeBox::RectInputRuntimeBox(const RectInputBox *box) :
    m_initRect(box->initRect()),
    m_keepAspectRatio(box->keepAspectRatio()),
    m_withActivator(box->withActivator()),
    m_refBitmap(box->refBitmap())
{
    setOwner(box);

    if (m_withActivator) {
        InputPorts in = box->inputPorts();
        m_activator.init(this, in[0], toPortNotifier(&RectInputRuntimeBox::activate));
        setInputPorts(RuntimeInputPorts() << &m_activator);
    }

    OutputPorts out = box->outputPorts();
    m_data.resize(4);
    m_data[0] = m_initRect.xmin;
    m_data[1] = m_initRect.xmax;
    m_data[2] = m_initRect.ymin;
    m_data[3] = m_initRect.ymax;
    m_out.init(this, out[0], PortData(4, m_data.data()));
    m_out.state().setValid();
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_inputId = -1;
    m_initRectSent = false;
}

InputInfoList RectInputRuntimeBox::inputInfo() const {
    return InputInfoList() << InputInfo::Ptr(new ImageRectInputInfo(owner()->name(), m_refBitmap, m_initRect, m_keepAspectRatio));
}

void RectInputRuntimeBox::registerInput()
{
    m_inputId = ThreadManager::instance()->registerInput(inputInfo()[0]);
}

void RectInputRuntimeBox::acquireInteractiveInput()
{
    if (!m_initRectSent) {
        if (sendRect())
            m_initRectSent = true;
    }
    else if (ThreadManager::instance()->readInput(m_data, m_inputId, false)) {
        Q_ASSERT(m_data.size() == 4);
        sendRect();
    }
}

RuntimeBox::PortNotifier RectInputRuntimeBox::generator() const
{
    if (m_withActivator)
        return 0;
    else
        return toPortNotifier(&RectInputRuntimeBox::activate);
}

bool RectInputRuntimeBox::activate(int)
{
    if (!m_initRectSent) {
        bool result = sendRect();
        if (result)
            m_initRectSent = true;
        return result;
    }
    acquireInteractiveInput();
    return true;
}

bool RectInputRuntimeBox::sendRect()
{
    m_out.setData(PortData(4, m_data.data()));
    m_out.state().setValid();
    return m_out.activateLinks();
}
