#ifndef INITBOXFACTORY_H
#define INITBOXFACTORY_H

#ifdef EQUARES_CORE_STATIC
void initBoxFactory();
#else // EQUARES_CORE_STATIC
inline void initBoxFactory() {}
#endif // EQUARES_CORE_STATIC

#endif // INITBOXFACTORY_H
