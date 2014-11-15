/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "EigenvaluesBox.h"
#include "box_util.h"
#include "math_util.h"

#ifdef WITH_ACML

REGISTER_BOX(EigenvaluesBox, "Eigenvalues")

EigenvaluesBox::EigenvaluesBox(QObject *parent) :
    Box(parent),
    m_matrix("matrix", this),
    m_eigReal("eig_real", this),
    m_eigImag("eig_imag", this)
{
}

InputPorts EigenvaluesBox::inputPorts() const {
    return InputPorts() << &m_matrix;
}

OutputPorts EigenvaluesBox::outputPorts() const {
    return OutputPorts() << &m_eigReal << &m_eigImag;
}

RuntimeBox *EigenvaluesBox::newRuntimeBox() const {
    return new EigenvaluesRuntimeBox(this);
}

void EigenvaluesBox::checkPortFormat() const
{
    PortFormat fm = m_matrix.format();
    if (!fm.isValid())
        throwBoxException("EigenvaluesBox: no format is specified for port 'matrix'");
    if (fm.dimension() != 2)
        throwBoxException("EigenvaluesBox: 2D data was expected at port 'matrix'");
    int n = fm.size(0);
    if (n != fm.size(1))
        throwBoxException("EigenvaluesBox: 2D square matrix was expected at port 'matrix'");
    if (n < 1)
        throwBoxException("EigenvaluesBox: nonempty data was expected at port 'matrix'");
    if (!m_eigReal.format().isValid())
        throwBoxException("EigenvaluesBox: no format is specified for port 'eig_real'");
    if (m_eigReal.format().dimension() != 1)
        throwBoxException("EigenvaluesBox: 1D data was expected at port 'eig_real'");
    if (m_eigReal.format().size(0) != n)
        throwBoxException("EigenvaluesBox: data size at port 'eig_real' does not match matrix dimension");
    if (!m_eigImag.format().isValid())
        throwBoxException("EigenvaluesBox: no format is specified for port 'eig_imag'");
    if (m_eigImag.format().dimension() != 1)
        throwBoxException("EigenvaluesBox: 1D data was expected at port 'eig_imag'");
    if (m_eigImag.format().size(0) != n)
        throwBoxException("EigenvaluesBox: data size at port 'eig_imag' does not match matrix dimension");
}

bool EigenvaluesBox::propagatePortFormat()
{
    PortFormat
        &fm = m_matrix.format(),
        &fr = m_eigReal.format(),
        &fi = m_eigImag.format();
    bool
        bm = fm.isValid(),
        br = fr.isValid(),
        bi = fi.isValid();
    if ((bm && br && bi)   ||   !(bm || br || bi))
        return false;
    bool result = false;
    if (bm) {
        if (fm.dimension() == 2   &&   fm.size(0) == fm.size(1)) {
            PortFormat fe(fm.size(0));
            if (!br) {
                fr = fe;
                result = true;
                br = true;
            }
            if (!bi) {
                fi = fe;
                result = true;
                bi = true;
            }
        }
    }
    else {
        int n = -1;
        if (br && fr.dimension() == 1)
            n = fr.size(0);
        else if (bi && fi.dimension() == 1)
            n = fi.size(0);
        if (n != -1) {
            fm = PortFormat(n, n);
            result = true;
            bm = true;
        }
    }
    if (br != bi)
        result = propagateCommonFormat(m_eigReal, m_eigImag) || result;
    return result;
}



EigenvaluesRuntimeBox::EigenvaluesRuntimeBox(const EigenvaluesBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_matrix.init(this, in[0], toPortNotifier(&EigenvaluesRuntimeBox::computeEigenvalues));
    setInputPorts(RuntimeInputPorts() << &m_matrix);

    OutputPorts out = box->outputPorts();
    m_n = in[0]->format().size(0);
    m_eigr.resize(m_n);
    m_eigi.resize(m_n);
    m_eigReal.init(this, out[0], PortData(m_n, m_eigr.data()));
    m_eigImag.init(this, out[1], PortData(m_n, m_eigi.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_eigReal << &m_eigImag);
}

bool EigenvaluesRuntimeBox::computeEigenvalues(int)
{
    eigenvalues(m_eigr.data(), m_eigi.data(), m_n, m_matrix.data().data());
    m_eigReal.state().setValid();
    m_eigImag.state().setValid();
    return m_eigReal.activateLinks() && m_eigImag.activateLinks();
}

#endif // WITH_ACML
