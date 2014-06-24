#ifndef THRESHOLDDETECTORBOX_H
#define THRESHOLDDETECTORBOX_H

#include "equares_core.h"

enum ThresholdDetectorBoxParam {
    ThresholdLess,
    ThresholdLessOrEqual,
    ThresholdGreater,
    ThresholdGreaterOrEqual,
    ThresholdEqual,
    ThresholdNotEqual
};

Q_DECLARE_METATYPE(ThresholdDetectorBoxParam)

class EQUARES_CORESHARED_EXPORT ThresholdDetectorBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(ThresholdDetectorBoxParam param READ param WRITE setParam)
public:
    explicit ThresholdDetectorBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

    typedef ThresholdDetectorBoxParam Param;

    Param param() const;
    ThresholdDetectorBox& setParam(const Param& param);

private:
    Param m_param;
    mutable InputPort m_threshold;
    mutable InputPort m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT ThresholdDetectorRuntimeBox : public RuntimeBox
{
public:
    explicit ThresholdDetectorRuntimeBox(const ThresholdDetectorBox *box);
    void reset();

private:
    RuntimeInputPort m_threshold;
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;
    typedef double (ThresholdDetectorRuntimeBox::*ThresholdFunc)(double src) const;
    ThresholdFunc m_thresholdFunc;
    double m_outData;
    double m_thresholdData;
    bool m_thresholdDataValid;

    double thresholdLess(double src) const;
    double thresholdLessOrEqual(double src) const;
    double thresholdGreater(double src) const;
    double thresholdGreaterOrEqual(double src) const;
    double thresholdEqual(double src) const;
    double thresholdNotEqual(double src) const;
    bool setThreshold(int);
    bool processData(int);
};

#endif // THRESHOLDDETECTORBOX_H
