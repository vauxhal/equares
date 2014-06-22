#include "CrossSectionBox.h"

REGISTER_BOX(CrossSectionBox, "CrossSection")



static QScriptValue xsParamToScriptValue(QScriptEngine *e, const CrossSectionBoxParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("index", p.index);
    result.setProperty("pos", p.pos);
    QScriptValue flags = e->newArray();
    int i = 0;
    if (p.flags & CountPositiveSpeed)
        flags.setProperty(i++, "positive");
    if (p.flags & CountNegativeSpeed)
        flags.setProperty(i++, "negative");
    result.setProperty("flags", flags);
    return result;
}

static void xsParamFromScriptValue(const QScriptValue& v, CrossSectionBoxParam& result) {
    result = CrossSectionBoxParam();
    result.index = v.property("index").toInt32();
    result.pos = v.property("pos").toNumber();
    result.flags = 0;
    QScriptValue flags = v.property("flags");
    if (!flags.isArray())
        return;
    int length = flags.property("length").toInt32();
    for (int i=0; i<length; ++i) {
        QString flag = flags.property(i).toString();
        if (flag == "positive")
            result.flags |= CountPositiveSpeed;
        else if (flag == "negative")
            result.flags |= CountNegativeSpeed;
    }
}

static void xsParamScriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, xsParamToScriptValue, xsParamFromScriptValue);
}

REGISTER_SCRIPT_INIT_FUNC(xsParamScriptInit)



CrossSectionBox::CrossSectionBox(QObject *parent) :
    Box(parent),
    m_in("input", this),
    m_out("output", this)
{
}

InputPorts CrossSectionBox::inputPorts() const {
    return InputPorts() << &m_in;
}

OutputPorts CrossSectionBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void CrossSectionBox::checkPortFormat() const {
    if (m_in.format() != m_out.format())
        throwBoxException("CrossSectionBox: Incompatible port formats");
}

bool CrossSectionBox::propagatePortFormat() {
    if (m_in.format().isValid() == m_out.format().isValid())
        return false;
    if (m_in.format().isValid())
        m_out.format() = m_in.format();
    else
        m_in.format() = m_out.format();
    return true;
}

RuntimeBox *CrossSectionBox::newRuntimeBox() const {
    return new CrossSectionRuntimeBox(this);
}

CrossSectionBox::Param CrossSectionBox::param() const {
    return m_param;
}

CrossSectionBox& CrossSectionBox::setParam(const Param& param) {
    m_param = param;
    return *this;
}



CrossSectionRuntimeBox::CrossSectionRuntimeBox(const CrossSectionBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(&CrossSectionRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_in);

    m_n = in[0]->format().dataSize();
    m_buf[0].resize(m_n);
    m_buf[1].resize(m_n);
    m_buf[2].resize(m_n);
    m_d1 = m_buf[0].data();
    m_d2 = m_buf[1].data();
    m_param = box->param();
    if (m_param.index < 0   ||   m_param.index >= m_n)
        throwBoxException("CrossSectionRuntimeBox: Invalid cross-section index");
    m_canCross = false;
    enum { TwoSided = CountNegativeSpeed | CountPositiveSpeed };
    m_twoSided = ( m_param.flags & TwoSided ) == TwoSided;

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0], PortData(m_n, m_buf[2].data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);
}

bool CrossSectionRuntimeBox::processInput(int)
{
    m_in.data().copyTo(m_d1);
    Pos p = pos(m_d1);
    bool result = true;
    if (m_canCross) {
        Pos pprev = pos(m_d2);
        if (p == Neutral) {
            m_canCross = m_twoSided;
            result = cross(m_d1);
        }
        else if (pprev != p) {
            if (!m_twoSided)
                m_canCross = false;
            interp();
            result = cross(m_buf[2].data());
        }
    }
    else if (m_twoSided)
        m_canCross = true;
    else if (m_param.flags && p != Neutral)
        m_canCross =
            ((m_param.flags & CountPositiveSpeed) != 0) ==
            (p == Negative);
    qSwap(m_d1, m_d2);
    return result;
}

CrossSectionRuntimeBox::Pos CrossSectionRuntimeBox::pos(const double *d) const
{
    double x = d[m_param.index] - m_param.pos;
    return x < 0 ?   Negative :   x > 0 ?   Positive :   Neutral;
}

bool CrossSectionRuntimeBox::cross(double *d)
{
    PortData outData = m_out.data();
    if (outData.data() != d)
        outData.copyFrom(d);
    m_out.state().setValid();
    return m_out.activateLinks();
}

void CrossSectionRuntimeBox::interp()
{
    double x = m_d1[m_param.index], xprev = m_d2[m_param.index];
    Q_ASSERT(x != xprev);
    double t = (m_param.pos - xprev) / (x - xprev);
    for (int i=0; i<m_n; ++i)
        m_buf[2][i] = m_d2[i] + t*(m_d1[i] - m_d2[i]);
}
