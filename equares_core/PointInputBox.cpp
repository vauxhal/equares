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



PointInputBox::PointInputBox(QObject *parent) : DataInputBox(parent)
{
}

void PointInputBox::checkPortFormat() const
{
    DataInputBox::checkPortFormat();
    int inputSize = inputPorts()[1]->format().size(0);
    for (int i=0; i<2; ++i) {
        if (m_transform[i].index < 0   ||   m_transform[i].index >= inputSize)
            throwBoxException("PointInputBox: Invalid input port format or invalid coordinate indices");
        if (m_transform[i].resolution <= 0)
            throwBoxException(QString("PointInputBox: Invalid grid resolution %1 - should be positive").arg(m_transform[i].resolution));
    }
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

QString PointInputBox::refBitmap() const {
    return m_refBitmap;
}

PointInputBox& PointInputBox::setRefBitmap(const QString& refBitmap) {
    m_refBitmap = refBitmap;
    return *this;
}



PointInputRuntimeBox::PointInputRuntimeBox(const PointInputBox *box) :
    DataInputRuntimeBox(box),
    m_transform(box->transform()),
    m_refBitmap(box->refBitmap())
{
}

InputInfoList PointInputRuntimeBox::inputInfo() const {
    return InputInfoList() << InputInfo::Ptr(new ImageInputInfo(owner()->name(), m_refBitmap));
}

void PointInputRuntimeBox::transformData(double *portData, const double *inputData) const
{
    for (int i=0; i<2; ++i) {
        const PointInputBoxDimTransform& t = m_transform[i];
        int x = static_cast<int>(inputData[i]);
        if (i == 1)
            x = t.resolution - x;
        portData[t.index] = t.transform(x);
    }
}

QVector<double> PointInputRuntimeBox::inputData(const double *portData) const
{
    QVector<double> result(2);
    for (int i=0; i<2; ++i) {
        const PointInputBoxDimTransform& t = m_transform[i];
        int x = t.transformBack(portData[t.index]);
        if (i == 1)
            x = t.resolution - x;
        result[i] = static_cast<double>(x);
    }
    return result;
}
