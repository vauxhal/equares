/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef RECTINPUTBOX_H
#define RECTINPUTBOX_H

#include "equares_core.h"
#include "equares_script.h"

struct RectInputBoxRange
{
public:
    RectInputBoxRange() : xmin(0), xmax(1), ymin(0), ymax(1) {}
    RectInputBoxRange(double xmin, double xmax, double ymin, double ymax) :
        xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax) {}

    double xmin;
    double xmax;
    double ymin;
    double ymax;
};

Q_DECLARE_METATYPE(RectInputBoxRange)

class EQUARES_CORESHARED_EXPORT RectInputBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(RectInputBoxRange initRect READ initRect WRITE setInitRect)
    Q_PROPERTY(bool keepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio)
    Q_PROPERTY(bool withActivator READ withActivator WRITE setWithActivator)
    Q_PROPERTY(bool restartOnInput READ restartOnInput WRITE setRestartOnInput)
    Q_PROPERTY(QString refBitmap READ refBitmap WRITE setRefBitmap)
public:
    explicit RectInputBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RectInputBoxRange initRect() const;
    RectInputBox& setInitRect(const RectInputBoxRange& initRect);
    bool keepAspectRatio() const;
    RectInputBox& setKeepAspectRatio(bool keepAspectRatio);
    bool withActivator() const;
    RectInputBox& setWithActivator(bool withActivator);
    bool restartOnInput() const;
    RectInputBox& setRestartOnInput(bool restartOnInput);
    QString refBitmap() const;
    RectInputBox& setRefBitmap(const QString& refBitmap);

private:
    mutable InputPort m_activator;
    mutable OutputPort m_out;
    RectInputBoxRange m_initRect;
    bool m_keepAspectRatio;
    bool m_withActivator;
    bool m_restartOnInput;
    QString m_refBitmap;

};

class EQUARES_CORESHARED_EXPORT RectInputRuntimeBox : public RuntimeBox
{
public:
    explicit RectInputRuntimeBox(const RectInputBox *box);
    InputInfoList inputInfo() const;
    void registerInput();
    void acquireInteractiveInput();
    PortNotifier generator() const;

private:
    RuntimeInputPort m_activator;
    RuntimeOutputPort m_out;
    RectInputBoxRange m_initRect;
    bool m_keepAspectRatio;
    bool m_withActivator;
    bool m_restartOnInput;
    QString m_refBitmap;

    int m_inputId;

    QVector<double> m_data;
    bool m_initRectSent;

    bool activate(int);
    bool sendRect(bool allowRestart);
};

#endif // RECTINPUTBOX_H
