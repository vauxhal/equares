#include <cmath>

struct rotate2d {
    int paramDimension() const {
        return 1;
    }
    const int *paramSizes() const {
        static int sizes[] = { 1 };
        return sizes;
    }
    const char *paramNames() const {
        return "phi";   // Can also return empty string or null ptr
    }
    int inputDimension() const {
        return 1;
    }
    const int *inputSizes() const {
        static int sizes[] = { 2 };
        return sizes;
    }
    const char *inputNames() const {
        return "x,y";   // Can also return empty string or null ptr
    }
    int outputDimension() const {
        return 1;
    }
    const int *outputSizes() const {
        static int sizes[] = { 2 };
        return sizes;
    }
    const char *outputNames() const {
        return "x,y";   // Can also return empty string or null ptr
    }

    // Auxiliary parameters
    double c, s;
    void prepare(const double *param) {
        c = cos(param[0]);
        s = sin(param[0]);
    }

    void transform(double *out, const double *param, const double *input) const {
        out[0] = c*input[0] - s*input[1];
        out[1] = s*input[0] + c*input[1];
    }
};
