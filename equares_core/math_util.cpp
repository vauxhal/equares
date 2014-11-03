#include "math_util.h"

#ifdef WITH_ACML

#include <acml.h>
#include "EquaresException.h"

void eigenvalues(double *eig_real, double *eig_imag, int n, double *matrix)
{
    int info = 0;
    dgeev(
        'N',        // left eigenvectors of A are not computed
        'N',        // right eigenvectors of A are not computed
        n,          // order of the matrix A
        matrix,     // matrix A (overwritten)
        n,          // leading dimension of the array A
        eig_real,   // real parts of computed eigenvalues
        eig_imag,   // imaginary parts of computed eigenvalues
        0,          // left eigenvectors, VL
        1,          // leading dimension of the array VL
        0,          // right eigenvectors, VR
        1,          // leading dimension of the array VR
        &info);
    if (info != 0 )
        throw EquaresException(QString("eigenvalues() failed: dgeev() returned %1").arg(info));
}

void complex_norm2(double *norm2, int n, const double *real_parts, const double *image_parts)
{
    for (int i=0; i<n; ++i)
        norm2[i] = sqr(real_parts[i]) + sqr(image_parts[i]);
}

#endif // WITH_ACML
