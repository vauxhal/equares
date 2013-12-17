#ifndef INTERVALFILTERBOX_H
#define INTERVALFILTERBOX_H

#include "equares_core.h"
#include "equares_script.h"

struct IntervalFilterBoxParam
{
    int index;
    double offset;
    double interval;
    IntervalFilterBoxParam() : index(0), offset(0), interval(1) {}
    IntervalFilterBoxParam(int index, double offset, double interval) :
        index(index), offset(offset), interval(interval) {}
};

Q_DECLARE_METATYPE(IntervalFilterBoxParam)

class EQUARES_CORESHARED_EXPORT IntervalFilterBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(IntervalFilterBoxParam param READ param WRITE setParam)
public:
    explicit IntervalFilterBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RuntimeBox *newRuntimeBox() const;

    typedef IntervalFilterBoxParam Param;
    Param param() const;
    IntervalFilterBox& setParam(const Param& param);

private:
    mutable InputPort m_in;
    mutable OutputPort m_out;
    Param m_param;
};

class EQUARES_CORESHARED_EXPORT IntervalFilterRuntimeBox : public RuntimeBox
{
public:
    explicit IntervalFilterRuntimeBox(const IntervalFilterBox *box);

private:
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;

    QVector<double> m_buf[3];
    int m_n;
    IntervalFilterBox::Param m_param;
    double *m_d1;
    double *m_d2;
    bool m_hasPrevData;
    double m_nextPos;

    bool processInput();
    void interp(double *dst, double t);
};

#endif // INTERVALFILTERBOX_H
