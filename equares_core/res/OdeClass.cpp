#include <cmath>

struct pendulum {
    int paramCount() const {
        return 2;       // l, g
    }
    const char *paramNames() const {
        return "l,g";   // Can also return empty string or null ptr
    }
    int varCount() const {
        return 2;       // q, dq
    }
    const char *varNames() const {
        return "q,dq";  // Can also return empty string or null ptr
    }

    // Auxiliary parameters
    double g_l;
    void prepare(const double *param) {
        g_l = param[1]/param[0];
    }

    void rhs(double *out, const double *param, const double *state) const {
        out[0] = state[1];
        out[1] = -g_l * sin(state[0]);
    }
};
