#include "BitmapBox.h"
#include <QPainter>
#include <QImage>
#include <QFileInfo>
#include <QDir>

REGISTER_BOX(BitmapBox, "Bitmap")

template<class T> QScriptValue toScriptValue(QScriptEngine *e, const T& p);
template<class T> void fromScriptValue(const QScriptValue& v, T& result);

template<>
QScriptValue toScriptValue(QScriptEngine *e, const BitmapBoxValueRange& p) {
    QScriptValue result = e->newObject();
    result.setProperty("vmin", p.vmin);
    result.setProperty("vmax", p.vmax);
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, BitmapBoxValueRange& result) {
    result = BitmapBoxValueRange();
    result.vmin = v.property("vmin").toNumber();
    result.vmax = v.property("vmax").toNumber();
}

template<>
QScriptValue toScriptValue(QScriptEngine *e, const BitmapBoxColorMap& p) {
    const BitmapBoxColorMap::Items& items = p.items();
    int n = items.size();
    QScriptValue result = e->newArray(n);
    for (int i=0; i<n; ++i) {
        QScriptValue item = e->newObject();
        item.setProperty("pos", items[i].pos);
        item.setProperty("color", items[i].color);
        result.setProperty(i, item);
    }
    return result;
}

template<>
void fromScriptValue(const QScriptValue& v, BitmapBoxColorMap& result) {
    int n = v.property("length").toInt32();
    const int MaxColorMapItemCount = 25;
    if (n < 0)
        n = 0;
    else if (n > MaxColorMapItemCount)
        n = MaxColorMapItemCount;
    BitmapBoxColorMap::Items items(n);
    for (int i=0; i<n; ++i) {
        QScriptValue item = v.property(i);
        items[i] = BitmapBoxColorMapItem(item.property("pos").toNumber(), item.property("color").toInt32());
    }
    result = BitmapBoxColorMap(items);
}

static void scriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, toScriptValue<BitmapBoxValueRange>, fromScriptValue<BitmapBoxValueRange>);
    qScriptRegisterMetaType(e, toScriptValue<BitmapBoxColorMap>, fromScriptValue<BitmapBoxColorMap>);
}

REGISTER_SCRIPT_INIT_FUNC(scriptInit)



BitmapBox::BitmapBox(QObject *parent) :
    Box(parent),
    m_dump("input", this)
{
}

InputPorts BitmapBox::inputPorts() const {
    return InputPorts() << &m_dump;
}

OutputPorts BitmapBox::outputPorts() const {
    return OutputPorts();
}

void BitmapBox::checkPortFormat() const
{
    if (!m_dump.format().isValid())
        throwBoxException("BitmapBox: no format is specified for port 'input'");
    if (m_dump.format().dimension() != 2)
        throwBoxException("BitmapBox: a 2D format was expected for port 'input'");
    if (m_dump.format().size(0) <= 0 ||
        m_dump.format().size(1) <= 0)
        throwBoxException(QString("BitmapBox: resolution invalid format (non-positive size) for port 'input'").arg(ResolutionLimit));
    if (m_dump.format().size(0) > ResolutionLimit ||
        m_dump.format().size(1) > ResolutionLimit)
        throwBoxException(QString("BitmapBox: resolution limit (%1) is exceeded for port 'input'").arg(ResolutionLimit));
}

bool BitmapBox::propagatePortFormat() {
    return false;
}

RuntimeBox *BitmapBox::newRuntimeBox() const {
    return new BitmapRuntimeBox(this);
}

QString BitmapBox::fileName() const {
    return m_fileName;
}

BitmapBox &BitmapBox::setFileName(const QString& fileName) {
    m_fileName = fileName;
    return *this;
}

BitmapBoxValueRange BitmapBox::valueRange() const {
    return m_valueRange;
}

BitmapBox& BitmapBox::setValueRange(const BitmapBoxValueRange& valueRange) {
    m_valueRange = valueRange;
    return *this;
}

BitmapBoxColorMap BitmapBox::colorMap() const {
    return m_colorMap;
}

BitmapBox& BitmapBox::setColorMap(const BitmapBoxColorMap& colorMap) {
    m_colorMap = colorMap;
    return *this;
}



BitmapRuntimeBox::BitmapRuntimeBox(const BitmapBox *box) :
    m_fileName(box->fileName()),
    m_valueRange(box->valueRange()),
    m_colorMap(box->colorMap())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_dump.init(this, in[0], toPortNotifier(&BitmapRuntimeBox::dump));
    setInputPorts(RuntimeInputPorts() << &m_dump);
    if (m_fileName.isEmpty())
        throwBoxException("BitmapRuntimeBox: no fileName is specified");
    if (QFileInfo(m_fileName).absolutePath() != QDir::current().absolutePath())
        throwBoxException(QString("DumpRuntimeBox: Output file name is not valid (parameter fileName)"));
}

OutputFileInfoList BitmapRuntimeBox::outputFileInfo() const
{
    QVector<int> size = m_dump.outputPort()->port()->format().size();
    return OutputFileInfoList() << OutputFileInfo::image(m_fileName, size[0], size[1]);
}

bool BitmapRuntimeBox::dump(int)
{
    RuntimeOutputPort
        *dumpPort = m_dump.outputPort();
    Q_ASSERT(dumpPort->state().hasData());
    QVector<int> size = dumpPort->port()->format().size();
    Q_ASSERT(size.size() == 2);

    int nrows = size[1],   ncols = size[0];
    //QImage img(ncols, nrows, QImage::Format_Mono);
    QImage img(ncols, nrows, QImage::Format_ARGB32);
    {
        QPainter painter(&img);
        painter.fillRect(QRect(0,0,ncols,nrows), Qt::white);

        int index = 0;
        const double *data = dumpPort->data().data();
        for (int row=0; row<nrows; ++row)
            for (int col=0; col<ncols; ++col, ++index) {
                int color = m_colorMap.color(m_valueRange.param(data[index]));
                painter.fillRect(QRect(col,nrows-1-row,1,1), color);
            }
    }
    img.save(m_fileName);
    ThreadManager::instance()->reportProgress(ProgressInfo().setSync(true) << m_fileName);

    return true;
}
