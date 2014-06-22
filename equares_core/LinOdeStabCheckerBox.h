#ifndef LINODESTABCHECKERBOX_H
#define LINODESTABCHECKERBOX_H

#include "equares_core.h"
#include "equares_script.h"

class EQUARES_CORESHARED_EXPORT LinOdeStabCheckerBox : public Box
{
    Q_OBJECT
public:
    explicit LinOdeStabCheckerBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

private:
    mutable InputPort m_activator;
    mutable InputPort m_solution;
    mutable OutputPort m_initState;
    mutable OutputPort m_result;
};

class EQUARES_CORESHARED_EXPORT LinOdeStabCheckerRuntimeBox : public RuntimeBox
{
public:
    explicit LinOdeStabCheckerRuntimeBox(const LinOdeStabCheckerBox *box);

private:
    RuntimeInputPort m_activator;
    RuntimeInputPort m_solution;
    RuntimeOutputPort m_initState;
    RuntimeOutputPort m_result;

    int m_n;
    QVector<double> m_initStateData;
    double m_resultData;

    bool run(int);
};

#endif // LINODESTABCHECKERBOX_H
