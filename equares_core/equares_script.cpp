#include "equares_script.h"

ScriptCustomInit::ScriptInitFuncList *ScriptCustomInit::m_initializers;

void ScriptCustomInit::init(QScriptEngine *e) {
    foreach (ScriptInitFunc func, initializers())
        func(e);
}

void ScriptCustomInit::registerScriptInitFunc(ScriptInitFunc func) {
    initializers() << func;
}

ScriptCustomInit::ScriptInitFuncList& ScriptCustomInit::initializers() {
    if (!m_initializers)
        m_initializers = new ScriptInitFuncList;
    return *m_initializers;
}
