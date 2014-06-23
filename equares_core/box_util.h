#ifndef BOX_UTIL_H
#define BOX_UTIL_H

#include "equares_core.h"

bool propagateCommonFormat(Port& port1, Port& port2);
bool propagateCommonFormat(const QList<Port*>& ports);

#endif // BOX_UTIL_H
