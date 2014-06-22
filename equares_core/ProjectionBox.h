#ifndef PROJECTIONBOX_H
#define PROJECTIONBOX_H

#include "equares_core.h"

class EQUARES_CORESHARED_EXPORT ProjectionBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(QList<int> indices READ indices WRITE setIndices)
public:
    explicit ProjectionBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    RuntimeBox *newRuntimeBox() const;

    const QList<int>& indices() const;
    ProjectionBox &setIndices(const QList<int>& indices);

private:
    mutable InputPort m_in;
    mutable OutputPort m_out;
    QList<int> m_indices;
};

class EQUARES_CORESHARED_EXPORT ProjectionRuntimeBox : public RuntimeBox
{
public:
    explicit ProjectionRuntimeBox(const ProjectionBox *box);

private:
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;
    QVector<int> m_indices;
    QVector<double> m_data;

    bool processInput(int);
};

#endif // PROJECTIONBOX_H
