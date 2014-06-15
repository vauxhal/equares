#ifndef VIBRATINGPENDULUMBOX_H
#define VIBRATINGPENDULUMBOX_H

#include "OdeBox.h"
#include <cmath>

struct VibratingPendulumOdeTraits : OdeTraitsBase {
    enum {
        ParamCount = 4,     // l, g, a, omega
        VarCount = 2        // q, dq
    };

    // Auxiliary parameters
    double g_l, ao2_l;
#define L param[0]
#define G param[1]
#define A param[2]
#define OMEGA param[3]
    void computeAuxParam(const double *param) {
        g_l = G/L;
        ao2_l = A*OMEGA*OMEGA/L;
    }
    void rhs(double *out, const double *param, const double *state) const {
        Q_UNUSED(param);
        out[0] = state[1];
        out[1] = (ao2_l*sin(OMEGA*state[2]) - g_l) * sin(state[0]);
    }
#undef L
#undef G
#undef A
#undef OMEGA
};

class EQUARES_CORESHARED_EXPORT VibratingPendulumBox :
    public OdeBox<VibratingPendulumOdeTraits>
{
    Q_OBJECT
public:
    explicit VibratingPendulumBox(QObject *parent = 0) : OdeBox<VibratingPendulumOdeTraits>(parent) {}
};

#endif // VIBRATINGPENDULUMBOX_H
