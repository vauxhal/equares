#ifndef DOUBLEPENDULUMBOX_H
#define DOUBLEPENDULUMBOX_H

#include "OdeBox.h"
#include <cmath>

struct DoublePendulumOdeTraits : OdeTraitsBase {
    enum {
        ParamCount = 5,     // l1, m1, l2, m2, g
        VarCount = 4        // q1, q2, dq1, dq2
    };

    // Auxiliary parameters
    double L12;
    double L22;
    double L12m;
    double L22m;
    double L1L2;
    double L1mg;
    double L2mg;

#define L1 param[0]
#define M1 param[1]
#define L2 param[2]
#define M2 param[3]
#define G  param[4]
#define PHI state[0]
#define PSI state[1]
#define DPHI state[2]
#define DPSI state[3]
    void computeAuxParam(const double *param) {
        L12 = L1*L1;
        L22 = L2*L2;
        L12m = L12*M1;
        L22m = L22*M2;
        L1L2 = L1*L2;
        L1mg = L1*M1*G;
        L2mg = L2*M2*G;
    }
    void rhs(double *out, const double *param, const double *state) const {
        out[0] = state[2];
        out[1] = state[3];
        double
            sphi = sin(PHI),
            spsi = sin(PSI),
            sdif = sin(PSI-PHI),
            cdif = cos(PSI-PHI),
            A = L12m + L22m,
            B = L1L2*M2*cdif,
            C = L22m,
            c1 = L1L2*M2*sdif,
            F1 = c1*DPSI*DPSI - (L1mg+L2mg)*sphi,
            F2 = -c1*DPHI*DPHI - L2mg*spsi,
            D = A*C - B*B,
            D1 = F1*C - F2*B,
            D2 = A*F2 - B*F1;
        out[2] = D1/D;
        out[3] = D2/D;
    }
#undef L1
#undef M1
#undef L2
#undef M2
#undef G
#undef PHI
#undef PSI
#undef DPHI
#undef DPSI
};

class EQUARES_CORESHARED_EXPORT DoublePendulumBox :
    public OdeBox<DoublePendulumOdeTraits>
{
    Q_OBJECT
public:
    explicit DoublePendulumBox(QObject *parent = 0) : OdeBox<DoublePendulumOdeTraits>(parent) {}
};

#endif // DOUBLEPENDULUMBOX_H
