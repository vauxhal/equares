/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "LinOdeStabCheckerBox.h"
#include "box_util.h"
#include "math_util.h"
#include <cmath>

REGISTER_BOX(LinOdeStabCheckerBox, "LinOdeStabChecker")

LinOdeStabCheckerBox::LinOdeStabCheckerBox(QObject *parent) :
    Box(parent),
    m_activator("activator", this),
    m_solution("solution", this),
    m_initState("initState", this),
    m_result("result", this, PortFormat(1).setFixed())
{
}

InputPorts LinOdeStabCheckerBox::inputPorts() const {
    return InputPorts() << &m_activator << &m_solution;
}

OutputPorts LinOdeStabCheckerBox::outputPorts() const {
    return OutputPorts() << &m_initState << &m_result;
}

void LinOdeStabCheckerBox::checkPortFormat() const
{
    if (!m_solution.format().isValid())
        throwBoxException("LinOdeStabCheckerBox: no format is specified for port 'solution'");
    if (!m_initState.format().isValid())
        throwBoxException("LinOdeStabCheckerBox: no format is specified for port 'initState'");
    if (m_initState.format().dataSize() < 2)
        throwBoxException("LinOdeStabCheckerBox: data size for port 'initState' must be 2 or greater");
    if (m_solution.format().dimension() != 1)
        throwBoxException("LinOdeStabCheckerBox: an 1D format was expected for port 'solution'");
    if (m_solution.format() != m_initState.format())
        throwBoxException("LinOdeStabCheckerBox: different formats of ports 'initState' and 'solution'");
}

bool LinOdeStabCheckerBox::propagatePortFormat() {
    return propagateCommonFormat(m_solution, m_initState);
}

RuntimeBox *LinOdeStabCheckerBox::newRuntimeBox() const {
    return new LinOdeStabCheckerRuntimeBox(this);
}



LinOdeStabCheckerRuntimeBox::LinOdeStabCheckerRuntimeBox(const LinOdeStabCheckerBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_activator.init(this, in[0], toPortNotifier(&LinOdeStabCheckerRuntimeBox::run));
    m_solution.init(this, in[1]);
    setInputPorts(RuntimeInputPorts() << &m_activator << &m_solution);

    OutputPorts out = box->outputPorts();
    m_n = out[0]->format().dataSize();
    Q_ASSERT(m_n > 1);
    m_mmx.resize((m_n-1)*(m_n-1)+1);
#ifdef WITH_ACML
    m_eigr.resize(m_n-1);
    m_eigi.resize(m_n-1);
#endif // WITH_ACML
    m_initStateData.resize(m_n);
    m_initState.init(this, out[0], PortData(m_n, m_initStateData.data()));
    m_result.init(this, out[1], PortData(1, &m_resultData));
    setOutputPorts(RuntimeOutputPorts() << &m_initState << &m_result);
}

bool LinOdeStabCheckerRuntimeBox::run(int)
{
    int n = m_n - 1;
#ifndef WITH_ACML
    if (n != 2)
        throw EquaresException("LinOdeStabCheckerRuntimeBox: currently only 1-DOF systems are supported - consider supplying ACML_DIR variable to qmake");
#endif // !WITH_ACML

    // Find the monodromy matrix
    double *col = m_mmx.data();
    m_initStateData.fill(0);
    m_initState.state().setValid();
    for (int i=0; i<n; ++i, col+=n) {
        m_solution.state().invalidate();
        m_initStateData[i] = 1;
        if (!m_initState.activateLinks())
            return false;
        if (!m_solution.state().hasData())
            return false;
        m_solution.data().copyTo(col);
        m_initStateData[i] = 0;
    }

    // Find eigenvalues of the monodromy matrix
    const double eps = 1e-5;
#ifdef WITH_ACML
    eigenvalues(&m_eigr[0], &m_eigi[0], n, &m_mmx[0]);
    complex_norm2(&m_eigr[0], n, &m_eigr[0], &m_eigi[0]);
    double r2max = m_eigr[0];
    for (int i=1; i<n; ++i)
        if (r2max < m_eigr[i])
            r2max = m_eigr[i];
    m_resultData = r2max < 1 + 2*eps;
#else // WITH_ACML
    double
        a = m_mmx[0], b = m_mmx[1], c = m_mmx[2], d = m_mmx[3],
        D = 0.25*(a-d)*(a-d) + b*c;
    if (D < 0) {
        double rho = 0.25*(a+d)*(a+d) + fabs(D);
        m_resultData = rho < 1 + eps? 1: 0;
    }
    else {
        double x = 0.5*(a+d), sD = sqrt(D);
        m_resultData = x - sD > -1-eps   &&   x + sD < 1 + eps;
    }
#endif // WITH_ACML
    m_result.state().setValid();
    return m_result.activateLinks();
}
