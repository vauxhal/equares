#ifndef MATHIEUBOX_H
#define MATHIEUBOX_H

#include "OdeBox.h"
#include <cmath>

struct MathieuOdeTraits {
    enum {
        ParamCount = 2,     // lambda, gamma
        VarCount = 2        // q, dq
    };
    void computeAuxParam(const double *param) {
        Q_UNUSED(param);
    }
    void rhs(double *out, const double *param, const double *state) const {
        out[0] = state[1];
        out[1] = (2*param[1]*cos(2*state[2]) - param[0]) * state[0];
    }
};

class EQUARES_CORESHARED_EXPORT MathieuBox :
    public OdeBox<MathieuOdeTraits>
{
    Q_OBJECT
public:
    explicit MathieuBox(QObject *parent = 0) : OdeBox<MathieuOdeTraits>(parent) {}
};

#endif // MATHIEUBOX_H
