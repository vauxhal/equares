#define CLASSNAME <X>

#ifdef _WIN32
#define EQUARES_LIB_EXPORT __declspec(dllexport)
#else // _WIN32
#define EQUARES_LIB_EXPORT
#endif // _WIN32

extern "C" {
EQUARES_LIB_EXPORT void* newInstance() {
    return new CLASSNAME();
}
EQUARES_LIB_EXPORT void deleteInstance(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    delete x;
}

EQUARES_LIB_EXPORT int paramDimension(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->paramDimension();
}
EQUARES_LIB_EXPORT const int *paramSizes(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->paramSizes();
}
EQUARES_LIB_EXPORT const char *paramNames(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->paramNames();
}

EQUARES_LIB_EXPORT int inputDimension(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->inputDimension();
}
EQUARES_LIB_EXPORT const int *inputSizes(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->inputSizes();
}
EQUARES_LIB_EXPORT const char *inputNames(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->inputNames();
}

EQUARES_LIB_EXPORT int outputDimension(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->outputDimension();
}
EQUARES_LIB_EXPORT const int *outputSizes(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->outputSizes();
}
EQUARES_LIB_EXPORT const char *outputNames(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->outputNames();
}

EQUARES_LIB_EXPORT void prepare(void *p, const double *param) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->prepare(param);
}
EQUARES_LIB_EXPORT void transform(void *p, double *out, const double *param, const double *input) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->transform(out, param, input);
}
}
