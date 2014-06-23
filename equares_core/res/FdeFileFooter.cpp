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
EQUARES_LIB_EXPORT int paramCount(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->paramCount();
}
EQUARES_LIB_EXPORT const char *paramNames(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->paramNames();
}
EQUARES_LIB_EXPORT int varCount(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->varCount();
}
EQUARES_LIB_EXPORT const char *varNames(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->varNames();
}
EQUARES_LIB_EXPORT void prepare(void *p, const double *param) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->prepare(param);
}
EQUARES_LIB_EXPORT void nextState(void *p, double *out, const double *param, const double *state) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->nextState(out, param, state);
}
}
