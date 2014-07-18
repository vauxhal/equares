/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef SIMPLEINPUTBOX_H
#define SIMPLEINPUTBOX_H

#include "DataInputBox.h"

struct SimpleInputBoxParamItem
{
    int index;
    QString name;
    SimpleInputBoxParamItem() : index(0) {}
    SimpleInputBoxParamItem(int index, const QString& name) : index(index), name(name) {}
};

typedef QVector<SimpleInputBoxParamItem> SimpleInputBoxParam;

Q_DECLARE_METATYPE(SimpleInputBoxParamItem)
Q_DECLARE_METATYPE(SimpleInputBoxParam)

class EQUARES_CORESHARED_EXPORT SimpleInputBox : public DataInputBox
{
    Q_OBJECT
    Q_PROPERTY(SimpleInputBoxParam param READ param WRITE setParam)
public:
    explicit SimpleInputBox(QObject *parent = 0);

    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;

    typedef SimpleInputBoxParamItem ParamItem;
    typedef SimpleInputBoxParam Param;

    Param param() const;
    SimpleInputBox& setParam(const Param& param);

private:
    Param m_param;
};

class EQUARES_CORESHARED_EXPORT SimpleInputRuntimeBox : public DataInputRuntimeBox
{
public:
    explicit SimpleInputRuntimeBox(const SimpleInputBox *box);
    InputInfoList inputInfo() const;

protected:
    void transformData(double *portData, const double *inputData) const;
    QVector<double> inputData(const double *portData) const;

private:
    typedef SimpleInputBoxParamItem ParamItem;
    typedef SimpleInputBoxParam Param;

    Param m_param;
};

#endif // SIMPLEINPUTBOX_H
