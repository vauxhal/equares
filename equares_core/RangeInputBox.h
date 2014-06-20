#ifndef RANGEINPUTBOX_H
#define RANGEINPUTBOX_H

#include "DataInputBox.h"

struct RangeInputBoxParamItem
{
    int index;
    QString name;
    double vmin;
    double vmax;
    int resolution;
    RangeInputBoxParamItem() : index(0), vmin(0), vmax(1), resolution(100) {}
    RangeInputBoxParamItem(int index, const QString& name, double vmin, double vmax, int resolution) :
        index(index), name(name), vmin(vmin), vmax(vmax), resolution(resolution) {}
};

typedef QVector<RangeInputBoxParamItem> RangeInputBoxParam;

Q_DECLARE_METATYPE(RangeInputBoxParamItem)
Q_DECLARE_METATYPE(RangeInputBoxParam)

class EQUARES_CORESHARED_EXPORT RangeInputBox : public DataInputBox
{
    Q_OBJECT
    Q_PROPERTY(RangeInputBoxParam param READ param WRITE setParam)
public:
    explicit RangeInputBox(QObject *parent = 0);

    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;

    typedef RangeInputBoxParamItem ParamItem;
    typedef RangeInputBoxParam Param;

    Param param() const;
    RangeInputBox& setParam(const Param& param);

private:
    Param m_param;
};

class EQUARES_CORESHARED_EXPORT RangeInputRuntimeBox : public DataInputRuntimeBox
{
public:
    explicit RangeInputRuntimeBox(const RangeInputBox *box);
    InputInfoList inputInfo() const;

protected:
    void transformData(double *portData, const double *inputData) const;

private:
    typedef RangeInputBoxParamItem ParamItem;
    typedef RangeInputBoxParam Param;

    Param m_param;
};

#endif // RANGEINPUTBOX_H
