#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#ifdef WITH_ACML

template< class X > X sqr(const X& x) {
    return x*x;
}

void eigenvalues(double *eig_real, double *eig_imag, int n, double *matrix);
void complex_norm2(double *norm2, int n, const double *real_parts, const double *image_parts);

#endif // WITH_ACML

#endif // MATH_UTIL_H
