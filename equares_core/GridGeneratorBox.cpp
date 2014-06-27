#include "GridGeneratorBox.h"
#include "box_util.h"

REGISTER_BOX(GridGeneratorBox, "GridGenerator")



template<class T> QScriptValue toScriptValue(QScriptEngine *e, const T& p);
template<class T> void fromScriptValue(const QScriptValue& v, T& result);

template<>
QScriptValue toScriptValue(QScriptEngine *e, const GridGeneratorBoxParamItem& p) {
    QScriptValue result = e->newObject();
    result.setProperty("index", p.index);
    result.setProperty("vmin", p.vmin);
    result.setProperty("vmax", p.vmax);
    result.setProperty("count", p.count);
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, GridGeneratorBoxParamItem& result) {
    result = GridGeneratorBoxParamItem();
    result.index = v.property("index").toInt32();
    result.vmin = v.property("vmin").toNumber();
    result.vmax = v.property("vmax").toNumber();
    result.count = v.property("count").toInt32();
}

template<>
QScriptValue toScriptValue(QScriptEngine *e, const GridGeneratorBoxParam& p) {
    int n = p.size();
    QScriptValue result = e->newArray(n);
    for (int i=0; i<n; ++i)
        result.setProperty(i, toScriptValue(e, p[i]));
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, GridGeneratorBoxParam& result) {
    int n = v.property("length").toInt32();
    const int MaxGridGenParamItemCount = 5;
    if (n < 0)
        n = 0;
    else if (n > MaxGridGenParamItemCount)
        n = MaxGridGenParamItemCount;
    result = GridGeneratorBoxParam(n);
    for (int i=0; i<n; ++i)
        fromScriptValue(v.property(i), result[i]);
}

static void scriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, toScriptValue<GridGeneratorBoxParamItem>, fromScriptValue<GridGeneratorBoxParamItem>);
    qScriptRegisterMetaType(e, toScriptValue<GridGeneratorBoxParam>, fromScriptValue<GridGeneratorBoxParam>);
}

REGISTER_SCRIPT_INIT_FUNC(scriptInit)



GridGeneratorBox::GridGeneratorBox(QObject *parent) :
    Box(parent),
    m_in("input", this),
    m_out("output", this),
    m_flush("flush", this, PortFormat(0).setFixed())
{
}

InputPorts GridGeneratorBox::inputPorts() const {
    return InputPorts() << &m_in;
}

OutputPorts GridGeneratorBox::outputPorts() const {
    return OutputPorts() << &m_out << &m_flush;
}

void GridGeneratorBox::checkPortFormat() const
{
    if (!m_in.format().isValid())
        throwBoxException("GridGeneratorBox: no format is specified for port 'input'");
    if (m_in.format().dimension() != 1)
        throwBoxException("GridGeneratorBox: an 1D format was expected for port 'input'");
    int inputSize = m_in.format().size(0);
    foreach (const ParamItem& item, m_param) {
        if (item.index < 0   ||   item.index >= inputSize)
            throwBoxException("GridGeneratorBox: Invalid input port format or invalid grid indices");
        if (item.count <= 0   ||   item.count >= CountLimit)
            throwBoxException(QString("GridGeneratorBox: invalid grid count %1 - should be in range [1, %2]").arg(item.count, CountLimit));
    }
    if (m_in.format() != m_out.format())
        throwBoxException("GridGeneratorBox: Incompatible input/output port formats");
}

bool GridGeneratorBox::propagatePortFormat() {
    return propagateCommonFormat(m_in, m_out);
}

RuntimeBox *GridGeneratorBox::newRuntimeBox() const {
    return new GridGeneratorRuntimeBox(this);
}

GridGeneratorBox::Param GridGeneratorBox::param() const {
    return m_param;
}

GridGeneratorBox& GridGeneratorBox::setParam(const Param& param) {
    foreach (const ParamItem& item, param) {
        if (item.count <= 0   ||   item.count >= CountLimit)
            throwBoxException(QString("GridGeneratorBox: Unable to set parameter: invalid grid count %1 - should be in range [1, %2]").arg(item.count, CountLimit));
    }
    m_param = param;
    return *this;
}



GridGeneratorRuntimeBox::GridGeneratorRuntimeBox(const GridGeneratorBox *box) :
    m_param(box->param())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(&GridGeneratorRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_in);

    OutputPorts out = box->outputPorts();
    m_data = QVector<double>(out[0]->format().dataSize(), 0);
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    m_flush.init(this, out[1]);
    m_midx.resize(m_param.size());
    setOutputPorts(RuntimeOutputPorts() << &m_out << &m_flush);
}

// Increments multi-index stored in m_midx and
// returns the last of changed indices; if m_midx has reached its final value, returns -1
int GridGeneratorRuntimeBox::incMidx() {
    for (int i=0, n=m_midx.size(); i<n; ++i) {
        if (m_midx[i] < m_param[i].count) {
            ++m_midx[i];
            for (int j=0; j<i; ++j)
                m_midx[j] = 0;
            return i;
        }
    }
    return -1;
}

bool GridGeneratorRuntimeBox::processInput(int)
{
    Q_ASSERT(m_in.state().hasData());
    if (m_data.isEmpty())
        return true;
    m_in.data().copyTo(&m_data[0]);
    m_midx.fill(0);
    forever {
        m_out.state().setValid();
        if (!m_out.activateLinks())
            return false;
        int i = incMidx();
        if (i < 0)
            break;
        for (int j=0; j<=i; ++j) {
            const ParamItem& p = m_param[j];
            m_data[p.index] = p.interp(m_midx[j]);
        }
    }
    return m_flush.activateLinks();
}
