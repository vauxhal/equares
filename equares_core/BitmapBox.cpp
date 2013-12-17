#include "BitmapBox.h"
#include <QPainter>
#include <QImage>

REGISTER_BOX(BitmapBox, "Bitmap")

BitmapBox::BitmapBox(QObject *parent) :
    Box(parent),
    m_dump("input", this)
{
    m_dump.setHelpString(tr("2D input for bitmap"));
    setHelpString(tr("Outputs 2D input data to a bitmap in the PNG format"));
}

InputPorts BitmapBox::inputPorts() const {
    return InputPorts() << &m_dump;
}

OutputPorts BitmapBox::outputPorts() const {
    return OutputPorts();
}

BoxPropertyList BitmapBox::boxProperties() const {
    return BoxPropertyList() << BoxProperty("fileName", tr("The name of the output bitmap file"));
}

void BitmapBox::checkPortFormat() const
{
    if (!m_dump.format().isValid())
        throw EquaresException("BitmapBox: no format is specified for port 'dump'");
    if (m_dump.format().dimension() != 2)
        throw EquaresException("BitmapBox: a 2D format was expected for port 'dump'");
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
        throw EquaresException("BitmapRuntimeBox: no fileName is specified");
}

bool BitmapRuntimeBox::dump()
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
                    painter.fillRect(QRect(col,row,1,1), Qt::black);
            }
    }
    img.save(m_fileName);

    return true;
}
