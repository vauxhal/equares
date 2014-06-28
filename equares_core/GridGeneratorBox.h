#ifndef GRIDGENERATORBOX_H
#define GRIDGENERATORBOX_H

#include "equares_core.h"
#include "equares_script.h"

struct GridGeneratorBoxParamItem
{
    int index;      // Index of data element in the input port
    double vmin;    // Grid minimum value
    double vmax;    // Grid maximum value
    int count;      // Number of nodes in the grid
    GridGeneratorBoxParamItem() : index(0), vmin(0), vmax(1), count(101) {}
    GridGeneratorBoxParamItem(int index, double vmin, double vmax, int count) :
        index(index), vmin(vmin), vmax(vmax), count(count) {}
    double interp(int idx) const {
        Q_ASSERT(idx >= 0   &&   idx <= count);
        Q_ASSERT(count > 0);
        double t = static_cast<double>(idx) / count;
        return vmin + t*(vmax-vmin);
    }
};

typedef QVector<GridGeneratorBoxParamItem> GridGeneratorBoxParam;

Q_DECLARE_METATYPE(GridGeneratorBoxParamItem)
Q_DECLARE_METATYPE(GridGeneratorBoxParam)

class EQUARES_CORESHARED_EXPORT GridGeneratorBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(GridGeneratorBoxParam param READ param WRITE setParam)
public:
    explicit GridGeneratorBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    typedef GridGeneratorBoxParamItem ParamItem;
    typedef GridGeneratorBoxParam Param;

    Param param() const;
    GridGeneratorBox& setParam(const Param& param);

private:
    enum { CountLimit = 100000 };
    Param m_param;
    mutable InputPort m_in;
    mutable InputPort m_range;
    mutable OutputPort m_out;
    mutable OutputPort m_flush;
};

class EQUARES_CORESHARED_EXPORT GridGeneratorRuntimeBox : public RuntimeBox
{
public:
    explicit GridGeneratorRuntimeBox(const GridGeneratorBox *box);

private:
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;
    RuntimeOutputPort m_flush;

    GridGeneratorBox::Param m_param;
    QVector<double> m_data;

    QVector<int> m_midx;
    int incMidx();

    typedef GridGeneratorBoxParamItem ParamItem;
    typedef GridGeneratorBoxParam Param;
    bool processInput(int);
};

#endif // GRIDGENERATORBOX_H
