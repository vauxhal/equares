#include "PointInputBox.h"

REGISTER_BOX(PointInputBox, "PointInput")



template<class T> QScriptValue toScriptValue(QScriptEngine *e, const T& p);
template<class T> void fromScriptValue(const QScriptValue& v, T& result);

template<>
QScriptValue toScriptValue(QScriptEngine *e, const PointInputBoxDimTransform& p) {
    QScriptValue result = e->newObject();
    result.setProperty("index", p.index);
    result.setProperty("vmin", p.vmin);
    result.setProperty("vmax", p.vmax);
    result.setProperty("resolution", p.resolution);
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, PointInputBoxDimTransform& result) {
    result = PointInputBoxDimTransform();
    result.index = v.property("index").toInt32();
    result.vmin = v.property("vmin").toNumber();
    result.vmax = v.property("vmax").toNumber();
    result.resolution = v.property("resolution").toInt32();
}

template<>
QScriptValue toScriptValue(QScriptEngine *e, const PointInputBoxTransform& p) {
    QScriptValue result = e->newObject();
    result.setProperty("x", toScriptValue(e, p[0]));
    result.setProperty("y", toScriptValue(e, p[1]));
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, PointInputBoxTransform& result) {
    result = PointInputBoxTransform();
    fromScriptValue(v.property("x"), result[0]);
    fromScriptValue(v.property("y"), result[1]);
}

static void scriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, toScriptValue<PointInputBoxDimTransform>, fromScriptValue<PointInputBoxDimTransform>);
    qScriptRegisterMetaType(e, toScriptValue<PointInputBoxTransform>, fromScriptValue<PointInputBoxTransform>);
}

REGISTER_SCRIPT_INIT_FUNC(scriptInit)



PointInputBox::PointInputBox(QObject *parent) :
    Box(parent),
    m_sync(true),
    m_loop(true),
    m_activator("activator", this),
    m_in("input", this),
    m_out("output", this)
{
}

InputPorts PointInputBox::inputPorts() const {
    return InputPorts() << &m_activator << &m_in;
}

OutputPorts PointInputBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void PointInputBox::checkPortFormat() const
{
    if (!m_in.format().isValid())
        throwBoxException("PointInputBox: no format is specified for port 'input'");
    if (m_in.format().dimension() != 1)
        throwBoxException("PointInputBox: an 1D format was expected for port 'input'");
    int inputSize = m_in.format().size(0);
    for (int i=0; i<2; ++i) {
        if (m_transform[i].index < 0   ||   m_transform[i].index >= inputSize)
            throwBoxException("PointInputBox: Invalid input port format or invalid coordinate indices");
        if (m_transform[i].resolution <= 0)
            throwBoxException(QString("PointInputBox: Invalid grid resolution %1 - should be positive").arg(m_transform[i].resolution));
    }
    if (m_in.format() != m_out.format())
        throwBoxException("PointInputBox: Incompatible input/output port formats");
}

bool PointInputBox::propagatePortFormat() {
    if (m_in.format().isValid() == m_out.format().isValid())
        return false;
    if (m_in.format().isValid())
        m_out.format() = m_in.format();
    else
        m_in.format() = m_out.format();
    return true;
}

RuntimeBox *PointInputBox::newRuntimeBox() const {
    return new PointInputRuntimeBox(this);
}

PointInputBox::Transform PointInputBox::transform() const {
    return m_transform;
}

PointInputBox& PointInputBox::setTransform(const Transform& param) {
    for (int i=0; i<2; ++i) {
        if (m_transform[i].resolution <= 0)
            throwBoxException(QString("PointInputBox: Unable to set parameter: invalid grid resolution %1 - should be positive").arg(m_transform[i].resolution));
    }
    m_transform = param;
    return *this;
}

bool PointInputBox::sync() const {
    return m_sync;
}

PointInputBox& PointInputBox::setSync(bool sync) {
    m_sync = sync;
    return *this;
}

bool PointInputBox::loop() const {
    return m_loop;
}

PointInputBox& PointInputBox::setLoop(bool loop) {
    m_loop = loop;
    return *this;
}

QString PointInputBox::refBitmap() const {
    return m_refBitmap;
}

PointInputBox& PointInputBox::setRefBitmap(const QString& refBitmap) {
    m_refBitmap = refBitmap;
    return *this;
}



PointInputRuntimeBox::PointInputRuntimeBox(const PointInputBox *box) :
    m_transform(box->transform()),
    m_sync(box->sync()),
    m_loop(box->loop()),
    m_refBitmap(box->refBitmap())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_activator.init(this, in[0], toPortNotifier(&PointInputRuntimeBox::activate));
    m_in.init(this, in[1], toPortNotifier(&PointInputRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_activator << &m_in);

    OutputPorts out = box->outputPorts();
    m_data.resize(out[0]->format().dataSize());
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_inputId = -1;
    m_dataValid = false;
}

void PointInputRuntimeBox::registerInput()
{
    m_inputId = ThreadManager::instance()->registerInput(
        InputInfo::Ptr(new ImageInputInfo(owner()->name(), m_refBitmap)));
}

bool PointInputRuntimeBox::fetchInputPortData()
{
    if (m_dataValid)
        return true;
    if (!m_in.state().hasData())
        return false;
    Q_ASSERT(m_in.data().size() == m_data.size());
    m_in.data().copyTo(m_data.data());
    m_out.state().setValid();
    m_dataValid = true;
    return true;
}

bool PointInputRuntimeBox::activate()
{
    forever {
        if (!fetchInputPortData())
            return false;
        QVector<double> input = ThreadManager::instance()->readInput(m_inputId, m_sync);
        if (input.isEmpty())
            return true;
        if (!(m_loop && m_sync))
            break;
        Q_ASSERT(input.size() == 2);
        // deBUG
        EQUARES_COUT << "PointInput: " << input[0] << input[1] << endl;
        for (int i=0; i<2; ++i) {
            const PointInputBoxDimTransform& t = m_transform[i];
            Q_ASSERT(t.index >= 0   &&   t.index < m_data.size());
            m_data[t.index] = t.transform(static_cast<int>(input[i]));
            m_out.state().setValid();
        }
        if (!m_out.activateLinks())
            return false;
    }
    return true;
}

bool PointInputRuntimeBox::processInput()
{
    // TODO
    Q_ASSERT(m_in.state().hasData());
    m_dataValid = false;
    fetchInputPortData();
    return m_out.activateLinks();
}
