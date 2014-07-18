/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef DATAINPUTBOX_H
#define DATAINPUTBOX_H

#include "equares_core.h"
#include "equares_script.h"

class EQUARES_CORESHARED_EXPORT DataInputBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(bool restartOnInput READ restartOnInput WRITE setRestartOnInput)
public:

    explicit DataInputBox(QObject *parent = 0);
    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    bool restartOnInput() const;
    DataInputBox& setRestartOnInput(bool restartOnInput);

private:
    mutable InputPort m_activator;
    mutable InputPort m_in;
    mutable OutputPort m_out;
    bool m_restartOnInput;
};

class EQUARES_CORESHARED_EXPORT DataInputRuntimeBox : public RuntimeBox
{
public:
    explicit DataInputRuntimeBox(const DataInputBox *box);
    void registerInput();
    void acquireInteractiveInput();

protected:
    virtual void transformData(double *portData, const double *inputData) const = 0;
    virtual QVector<double> inputData(const double *portData) const = 0;

private:
    RuntimeInputPort m_activator;
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;
    bool m_restartOnInput;

    QVector<double> m_data;
    bool m_dataValid;
    bool fetchInputPortData();
    QVector<double> m_iinputData;
    bool m_iinputDataValid;
    int m_inputId;
    bool m_unititializedInputPort;

    bool activate(int);
    bool processInput(int);
};

#endif // DATAINPUTBOX_H
