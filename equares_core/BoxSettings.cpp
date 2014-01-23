#include "equares_core.h"
#include <functional>

template<class ElementType>
static void readPtrListSettings(const QList<ElementType*>& lst, QSettings &settings, const QString& name)
{
    int n = settings.beginReadArray(name);
    int index = 0;
    foreach (ElementType *x, lst) {
        if (index >= n)
            break;
        settings.setArrayIndex(index++);
        x->loadSettings(settings);
    }
    settings.endArray();
}

template<class ElementType, class Object, class Setter>
static void readListSettings(Object& object, Setter setter, QSettings &settings, const QString& name)
{
    int n = settings.beginReadArray(name);
    QList<ElementType> lst;
    for (int index=0; index<n; ++index) {
        settings.setArrayIndex(index);
        ElementType x;
        x.loadSettings(settings);
        lst << x;
    }
    setter(object, lst);
    settings.endArray();
}



PortHints& PortHints::loadSettings(QSettings& settings)
{
    m_entryHints.clear();

    // deBUG
    QString eee = settings.value("entries").toString();
    Q_UNUSED(eee);

    foreach(const QString& hint, settings.value("entries").toString().split(",", QString::SkipEmptyParts))
        m_entryHints << hint.trimmed();
    m_position = settings.value("position", -1.0).toDouble();
    return *this;
}

Port& Port::loadSettings(QSettings& settings)
{
    setHelpString(settings.value("help").toString());
    hints().loadSettings(settings);
    return *this;
}

BoxProperty& BoxProperty::loadSettings(QSettings& settings)
{
    name = settings.value("name").toString();
    helpString = settings.value("help").toString();
    return *this;
}

Box& Box::loadSettings(QSettings &settings)
{
    setHelpString(settings.value("help").toString());
    readPtrListSettings(inputPorts(), settings, "inputs");
    readPtrListSettings(outputPorts(), settings, "outputs");
    readListSettings<BoxProperty>(this, std::mem_fun(&Box::setBoxProperties), settings, "properties");
    return *this;
}
