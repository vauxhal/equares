// #define CLASSNAME <X>
#define CLASSNAME X
extern "C" {
void *newInstance() { return new CLASSNAME(); }
void deleteInstance(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    delete x;
}
int paramCount(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->paramCount();
}
int varCount(void *p) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->varCount();
}
void prepare(void *p, const double *param) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->prepare(param);
}
void rhs(void *p, double *out, const double *param, const double *state) {
    CLASSNAME* x = reinterpret_cast<CLASSNAME*>(p);
    return x->rhs(out, param, state);
}
}
