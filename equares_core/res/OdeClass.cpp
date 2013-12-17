struct X {
    int paramCount() const {
        return 2;   // l, g
    }
    int varCount() const {
        return 2;   // q, dq
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
