#ifndef MATHIEUBOX_H
#define MATHIEUBOX_H

#include "OdeBox.h"
#include <cmath>

#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif

struct MathieuOdeTraits : OdeTraitsBase {
    enum {
        ParamCount = 2,     // lambda, gamma
        VarCount = 2        // q, dq
    };

    void init(RuntimeBox *owner) {
        m_period.init(owner, owner->owner()->outputPorts()[1], PortData(1, &m_periodData));
        m_periodData = M_PI;
        m_period.state().setValid();
    }
    void computeAuxParam(const double *param) {
        Q_UNUSED(param);
    }
    void rhs(double *out, const double *param, const double *state) const {
        out[0] = state[1];
        out[1] = (2*param[1]*cos(2*state[2]) - param[0]) * state[0];
    }
    RuntimeOutputPorts additionalRuntimeOutputPorts() const {
        return RuntimeOutputPorts() << &m_period;
    }
    mutable RuntimeOutputPort m_period;
    double m_periodData;
};

class EQUARES_CORESHARED_EXPORT MathieuBox :
    public OdeBox<MathieuOdeTraits>
{
    Q_OBJECT
public:
    explicit MathieuBox(QObject *parent = 0) : OdeBox<MathieuOdeTraits>(parent),
    m_period("period", this, PortFormat(1).setFixed())
    {}

protected:
    OutputPorts additionalOutputPorts() const {
        return OutputPorts() << &m_period;
    }

private:
    mutable OutputPort m_period;
};

#endif // MATHIEUBOX_H
