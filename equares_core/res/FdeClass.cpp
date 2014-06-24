struct logistics {
    int paramCount() const {
        return 1;       // c
    }
    const char *paramNames() const {
        return "c";     // Can also return empty string or null ptr
    }
    int varCount() const {
        return 1;       // q
    }
    const char *varNames() const {
        return "q";     // Can also return empty string or null ptr
    }

    // Auxiliary parameters
    double c;
    void prepare(const double *param) {
        c = param[0];
    }

    void nextState(double *out, const double *param, const double *state) const {
        out[0] = c - state[0]*state[0];
    }
};
