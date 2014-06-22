#include "BitmapBox.h"
#include <QPainter>
#include <QImage>

REGISTER_BOX(BitmapBox, "Bitmap")

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



BitmapRuntimeBox::BitmapRuntimeBox(const BitmapBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_dump.init(this, in[0], toPortNotifier(&BitmapRuntimeBox::dump));
    setInputPorts(RuntimeInputPorts() << &m_dump);
    m_fileName = box->fileName();
    if (m_fileName.isEmpty())
        throwBoxException("BitmapRuntimeBox: no fileName is specified");
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
    QImage img(ncols, nrows, QImage::Format_Mono);
    {
        QPainter painter(&img);
        painter.fillRect(QRect(0,0,ncols,nrows), Qt::white);

        int index = 0;
        const double *data = dumpPort->data().data();
        for (int row=0; row<nrows; ++row)
            for (int col=0; col<ncols; ++col, ++index) {
                if (data[index] != 0)
                    painter.fillRect(QRect(col,nrows-1-row,1,1), Qt::black);
            }
    }
    img.save(m_fileName);
    ThreadManager::instance()->reportProgress(ProgressInfo().setSync(true) << m_fileName);

    return true;
}
