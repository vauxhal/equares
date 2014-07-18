/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#ifndef PENDULUMBOX_H
#define PENDULUMBOX_H

#include "OdeBox.h"
#include <cmath>

struct PendulumOdeTraits : OdeTraitsBase {
    enum {
        ParamCount = 2,     // l, g
        VarCount = 2        // q, dq
    };

    // Auxiliary parameters
    double g_l;

    void computeAuxParam(const double *param) {
        g_l = param[1]/param[0];
    }
    void rhs(double *out, const double *param, const double *state) const {
        Q_UNUSED(param);
        out[0] = state[1];
        out[1] = -g_l * sin(state[0]);
    }
};

class EQUARES_CORESHARED_EXPORT PendulumBox :
    public OdeBox<PendulumOdeTraits>
{
    Q_OBJECT
public:
    explicit PendulumBox(QObject *parent = 0) : OdeBox<PendulumOdeTraits>(parent) {}
};

#endif // PENDULUMBOX_H
