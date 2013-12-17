#ifndef ODECXXBOX_H
#define ODECXXBOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT OdeCxxBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QString src READ src WRITE setSrc)
public:
    explicit OdeCxxBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();
    RuntimeBox *newRuntimeBox() const;

    QString src() const;
    OdeCxxBox& setSrc(const QString& src);

    QString srcExample() const;

    int paramCount() const;
    int varCount() const;

    using Box::engine;

private:
    mutable InputPort m_param;
    mutable InputPort m_state;
    mutable OutputPort m_rhs;
    QString m_src;
};

class OdeCxxRuntimeBox : public RuntimeBox
{
public:
    explicit OdeCxxRuntimeBox(const OdeCxxBox *box);

private:
    RuntimeInputPort m_param;
    RuntimeInputPort m_state;
    RuntimeOutputPort m_rhs;

    QString m_src;

    bool m_hasParamData;
    QVector<double> m_paramData;
    QVector<double> m_stateData;
    QVector<double> m_rhsData;

    bool setParameters();
    bool setState();
};

#endif // ODECXXBOX_H
