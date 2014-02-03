#define CLASSNAME <X>

#ifdef _WIN32
#define ODE_EXPORT __declspec(dllexport)
#else // _WIN32
#define ODE_EXPORT
#endif // _WIN32

extern "C" {
ODE_EXPORT void* newInstance() {
    return new CLASSNAME();
}
ODE_EXPORT void deleteInstance(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    delete x;
}
ODE_EXPORT int paramCount(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->paramCount();
}
ODE_EXPORT const char *paramNames(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->paramNames();
}
ODE_EXPORT int varCount(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->varCount();
}
ODE_EXPORT const char *varNames(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->varNames();
}
ODE_EXPORT void prepare(void *p, const double *param) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->prepare(param);
}
ODE_EXPORT void rhs(void *p, double *out, const double *param, const double *state) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->rhs(out, param, state);
}
}
