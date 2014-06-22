#ifndef CROSSSECTIONBOX_H
#define CROSSSECTIONBOX_H

#include "equares_core.h"
#include "equares_script.h"

enum CrossSectionBoxFlags {
    CountPositiveSpeed = 0x01,
    CountNegativeSpeed = 0x02
};

struct CrossSectionBoxParam
{
    int index;
    double pos;
    int flags;
    CrossSectionBoxParam() : index(0), pos(0), flags(CountPositiveSpeed) {}
    CrossSectionBoxParam(int index, double pos, int flags) :
        index(index), pos(pos), flags(flags) {}
};

Q_DECLARE_METATYPE(CrossSectionBoxParam)

class EQUARES_CORESHARED_EXPORT CrossSectionBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(CrossSectionBoxParam param READ param WRITE setParam)
public:
    explicit CrossSectionBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RuntimeBox *newRuntimeBox() const;

    typedef CrossSectionBoxParam Param;
    Param param() const;
    CrossSectionBox& setParam(const Param& param);

private:
    mutable InputPort m_in;
    mutable OutputPort m_out;
    Param m_param;
};

class EQUARES_CORESHARED_EXPORT CrossSectionRuntimeBox : public RuntimeBox
{
public:
    explicit CrossSectionRuntimeBox(const CrossSectionBox *box);

private:
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;
    QVector<double> m_buf[3];
    int m_n;
    CrossSectionBox::Param m_param;
    double *m_d1;
    double *m_d2;
    bool m_canCross;
    bool m_twoSided;

    bool processInput(int);

    enum Pos {
        Negative,
        Neutral,
        Positive
    };
    Pos pos(const double *d) const;
    bool cross(double *d);
    void interp();
};

#endif // CROSSSECTIONBOX_H
