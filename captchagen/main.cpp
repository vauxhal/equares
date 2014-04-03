#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QGuiApplication>
#define QApp QGuiApplication
#else
#include <QApplication>
#define QApp QApplication
#endif

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <QBuffer>
#include <QFontDatabase>
#include <QDir>
#include <iostream>
#include <algorithm>
#include <cstdlib>

using namespace std;

int rnd(int xmin, int xmax)
{
    Q_ASSERT(xmax >= xmin);
    return rand() % (xmax - xmin + 1) + xmin;
}

QFont randomFont(int maxHeight)
{
    static const QString ff[] = {"Serif", "Times", "Courier"};
    static const int nff = sizeof(ff)/sizeof(const QString);

    QFont result(ff[rnd(0,nff-1)]);
    result.setPixelSize(rnd(2*maxHeight/3, maxHeight));
    if (rnd(0,2) == 2)
        result.setBold(true);
    if (rnd(0,2) == 2)
        result.setItalic(true);
    return result;
}

QColor randomColor()
{
    return QColor::fromHsv(rnd(0,359), rnd(128,255), rnd(0,192));
}

QPainterPath deformedPath(const QPainterPath& path, double dx)
{
    QPainterPath result;
    result.addPath(path);
    QRectF rc = path.boundingRect();
    double f = 0.1 * max(rc.width(), rc.height());
    for (int i=0, n=result.elementCount(); i<n; ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        double
            x = e.x + rnd(0,f) + dx,
            y = e.y + rnd(0,f) + dx;
        result.setElementPositionAt(i, x, y);
    }
    return result;
}

void renderCaptchaChar(QPainter& painter, const QRect& rect, QChar c, double w)
{
    int h0 = rect.height();
    QFont fnt = randomFont(h0);
    int h = fnt.pixelSize();
    QPainterPath path;
    double y = h0 - rnd(0, h0-h) - 10;
    path.addText(0, y, fnt, c);
    path = deformedPath(path, 0.5*(w - path.boundingRect().width()));
    double a = rnd(-30, 30);
    QTransform t = painter.transform();
    painter.translate(0.5*w, 0.5*h0);
    painter.rotate(a);
    painter.translate(-0.5*w, -0.5*h0);
    painter.fillPath(path, randomColor());
    painter.setTransform(t);
}

void addShapeNoise(QPainter& painter, const QRect& rect, int alpha)
{
    int N = 100;
    int w = rect.width()-1, h = rect.height()-1;
    for (int i=0; i<N; ++i) {
        QColor color = randomColor();
        color.setAlpha(alpha);
        painter.setPen(
            QPen(
                color,
                rnd(1,5),
                static_cast<Qt::PenStyle>(rnd(Qt::SolidLine,Qt::DashDotDotLine))));
        painter.drawLine(rnd(0,w), rnd(0,h), rnd(0,w), rnd(0,h));
    }
}

void addPixelNoise(QImage *img)
{
    QRect rect = img->rect();
    int N = rect.width()*rect.height() / 25;
    int w = rect.width()-1, h = rect.height()-1;
    for (int i=0; i<N; ++i)
        img->setPixel(rnd(0,w), rnd(0,h), randomColor().rgba());
}

void renderCaptcha(QImage *img, const QRect& rect, const QString& text)
{
    int n = text.size();
    if (n < 1)
        return;
    {
        QPainter painter(img);
        double dx = rect.width() / n;
        painter.translate(rect.left(), rect.top());
        addShapeNoise(painter, rect, 10);
        for (int i=0; i<n; ++i, painter.translate(dx, 0))
            renderCaptchaChar(painter, rect, text[i], dx);
        addShapeNoise(painter, rect, 10);
    }
    addPixelNoise(img);
}

void loadFonts()
{
    static const char *fontNames[] = {
        "AverageMono.ttf",
        "AverageMonoBold.ttf",
        "AverageMonoItalic.ttf",
        "AverageMonoBoldItalic.ttf",
        "Pfennig.ttf",
        "PfennigBold.ttf",
        "PfennigItalic.ttf",
        "PfennigBoldItalic.ttf",
        "GenBasR.ttf",
        "GenBasB.ttf",
        "GenBasI.ttf",
        "GenBasBI.ttf",
        0
    };
    QDir dir(":/fonts");
    for (int i=0; fontNames[i]; ++i)
        if (QFontDatabase::addApplicationFont(dir.absoluteFilePath(fontNames[i])) == -1)
        {
            cerr << "Failed to load font '" << fontNames[i] << "'" << endl;
            exit(-1);
        }
}

int main(int argc, char *argv[])
{
    QApp a(argc, argv, false);
    QStringList args = a.arguments();
    int nSkippedArgs = 0;
    if (args.length() > 1   &&   args[1] == "-platform")
        nSkippedArgs += 2;
    if (args.length() - nSkippedArgs != 2)
    {
        cerr << "Usage: captchagen <text>" << endl;
        return -1;
    }
    loadFonts();

    QString text = args[nSkippedArgs + 1];

    QImage img(QSize(150, 35), QImage::Format_ARGB32);
    int seed = static_cast<int>(QDateTime::currentMSecsSinceEpoch());
    srand(seed);
    renderCaptcha(&img, img.rect(), text);

    QBuffer f;
    f.open(QIODevice::WriteOnly);
    img.save(&f, "PNG");
    cout << f.data().toBase64().constData();
    return 0;
}
