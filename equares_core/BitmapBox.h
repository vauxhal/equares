#ifndef BITMAPBOX_H
#define BITMAPBOX_H

#include "equares_core.h"
#include "equares_script.h"

struct BitmapBoxValueRange
{
    BitmapBoxValueRange() : vmin(0), vmax(1) {}
    BitmapBoxValueRange(double vmin, double vmax) : vmin(vmin), vmax(vmax) {}
    double vmin;
    double vmax;
    double param(double value) const {
        return (value - vmin) / (vmax - vmin);
    }
};

Q_DECLARE_METATYPE(BitmapBoxValueRange)

struct BitmapBoxColorMapItem
{
    double pos;
    int color;
    BitmapBoxColorMapItem() : pos(0), color(0) {}
    BitmapBoxColorMapItem(double pos, int color) : pos(pos), color(color) {}
    bool operator<(const BitmapBoxColorMapItem& x) const {
        return pos < x.pos;
    }
};

class BitmapBoxColorMap
{
public:
    typedef QVector<BitmapBoxColorMapItem> Items;

    BitmapBoxColorMap() {
        m_items.resize(2);
        m_items[0] = BitmapBoxColorMapItem(0, 0xffffff);
        m_items[1] = BitmapBoxColorMapItem(1, 0x000000);
    }
    explicit BitmapBoxColorMap(const Items& items) :
        m_items(items)
    {
        qSort(m_items);
        double pos = m_items.isEmpty()? 1.: m_items.first().pos;
        if (pos > 0)
            m_items.push_front(BitmapBoxColorMapItem(0, 0xffffff));
        pos = m_items.last().pos;
        if (pos < 1)
            m_items.push_back(BitmapBoxColorMapItem(1, 0x000000));
        while(m_items.first().pos < 0)
            m_items.pop_front();
        while(m_items.last().pos > 1)
            m_items.pop_back();
        Q_ASSERT(m_items.size() > 1);
    }
    const Items& items() const {
        return m_items;
    }
    int color(double param) const {
        if (param <= 0)
            return m_items.first().color;
        if (param >= 1)
            return m_items.last().color;
        Items::const_iterator it = qLowerBound(m_items, BitmapBoxColorMapItem(param,0));
        Q_ASSERT(it != m_items.begin()   &&   it != m_items.end());
        const BitmapBoxColorMapItem &v2 = *it--,   &v1 = *it;
        double d = v2.pos - v1.pos;
        if (d <= 0)
            return v2.color;
        double p = (param - v1.pos) / d;
        int color = 0;
        for (int shift=0; shift<=16; shift+=8) {
            int
                c1 = (v1.color >> shift) & 0xff,
                c2 = (v2.color >> shift) & 0xff,
                c = static_cast<int>(c1 + (c2-c1)*p);
            color |= c << shift;
        }
        return color;
    }

private:
    Items m_items;
};

Q_DECLARE_METATYPE(BitmapBoxColorMap)

class EQUARES_CORESHARED_EXPORT BitmapBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
    Q_PROPERTY(BitmapBoxValueRange valueRange READ valueRange WRITE setValueRange)
    Q_PROPERTY(BitmapBoxColorMap colorMap READ colorMap WRITE setColorMap)

public:
    explicit BitmapBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RuntimeBox *newRuntimeBox() const;

    QString fileName() const;
    BitmapBox& setFileName(const QString& fileName);
    BitmapBoxValueRange valueRange() const;
    BitmapBox& setValueRange(const BitmapBoxValueRange& valueRange);
    BitmapBoxColorMap colorMap() const;
    BitmapBox& setColorMap(const BitmapBoxColorMap& colorMap);
private:
    enum { ResolutionLimit = 2000 };
    mutable InputPort m_dump;
    QString m_fileName;
    BitmapBoxValueRange m_valueRange;
    BitmapBoxColorMap m_colorMap;
};

class EQUARES_CORESHARED_EXPORT BitmapRuntimeBox : public RuntimeBox
{
public:
    explicit BitmapRuntimeBox(const BitmapBox *box);
    OutputFileInfoList outputFileInfo() const;

private:
    RuntimeInputPort m_dump;
    QString m_fileName;
    BitmapBoxValueRange m_valueRange;
    BitmapBoxColorMap m_colorMap;

    bool dump(int);
};

#endif // BITMAPBOX_H
