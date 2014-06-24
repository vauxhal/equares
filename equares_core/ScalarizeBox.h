#ifndef SCALARIZEBOX_H
#define SCALARIZEBOX_H

#include "equares_core.h"

enum ScalarizeBoxParam {
    ScalarizeNorm1,
    ScalarizeNorm2,
    ScalarizeNormInf,
    ScalarizeMin,
    ScalarizeMax
};

Q_DECLARE_METATYPE(ScalarizeBoxParam)

class EQUARES_CORESHARED_EXPORT ScalarizeBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(ScalarizeBoxParam param READ param WRITE setParam)
public:
    explicit ScalarizeBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

    typedef ScalarizeBoxParam Param;

    Param param() const;
    ScalarizeBox& setParam(const Param& param);

private:
    Param m_param;
    mutable InputPort m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT ScalarizeRuntimeBox : public RuntimeBox
{
public:
    explicit ScalarizeRuntimeBox(const ScalarizeBox *box);

private:
    typedef ScalarizeBoxParam Param;
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;
    double m_outData;
    int m_n;
    typedef double (ScalarizeRuntimeBox::*ScalarizeFunc)(const double *src) const;
    ScalarizeFunc m_scalarize;

    double scalarizeNorm1(const double *src) const;
    double scalarizeNorm2(const double *src) const;
    double scalarizeNormInf(const double *src) const;
    double scalarizeNormMin(const double *src) const;
    double scalarizeNormMax(const double *src) const;
    bool processInput(int);
};

#endif // SCALARIZEBOX_H
