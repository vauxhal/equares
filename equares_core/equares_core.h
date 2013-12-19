#ifndef EQUARES_CORE_H
#define EQUARES_CORE_H

#include <QObject>
#include <QScriptable>
#include <QScriptContext>
#include <QVector>
#include <QList>
#include <QStringList>
#include <QSharedPointer>
#include <QMultiMap>

#include "equares_core_global.h"
#include "EquaresException.h"

template< class Owner >
class OwnedBy
{
public:
    typedef Owner owner_type;

    OwnedBy() : m_owner(0) {}
    explicit OwnedBy(Owner *owner) : m_owner(owner) {}
    Owner *owner() const {
        return m_owner;
    }
    void setOwner(Owner *owner) {
        m_owner = owner;
    }
private:
    Owner *m_owner;
};

class Named
{
public:
    Named() {}
    explicit Named(const QString& name) : m_name(name) {}
    QString name() const {
        return m_name;
    }
    void setName(const QString& name) {
        m_name = name;
    }
private:
    QString m_name;
};

template< class Lnk >
class LinkedToOne
{
public:
    typedef Lnk link_type;

    LinkedToOne() : m_link(0) {}
    explicit LinkedToOne(Lnk *link) : m_link(link) {}
    Lnk *link() const {
        return m_link;
    }
    void setLink(Lnk *link) {
        m_link = link;
    }
    bool isConnected() const {
        return m_link != 0;
    }
private:
    Lnk *m_link;
};

template< class Lnk >
class LinkedToMany
{
public:
    typedef Lnk link_type;
    typedef QList<Lnk*> Links;

    LinkedToMany() {}
    explicit LinkedToMany(const Links& links) : m_links(links) {}
    const Links& links() const {
        return m_links;
    }
    Links& links() {
        return m_links;
    }
    bool isConnected() const {
        return !m_links.isEmpty();
    }
private:
    Links m_links;
};

template< class InPrt, class OutPrt >
class LinkTemplate
{
public:
    LinkTemplate() : m_inputPort(0), m_outputPort(0) {}
    LinkTemplate(OutPrt *out, InPrt *in) : m_inputPort(in), m_outputPort(out) {}
    InPrt *inputPort() const {
        return m_inputPort;
    }
    LinkTemplate& setInputPort(InPrt *inputPort) {
        m_inputPort = inputPort;
        return *this;
    }
    OutPrt *outputPort() const {
        return m_outputPort;
    }
    LinkTemplate& setOutputPort(OutPrt *outputPort) {
        m_outputPort = outputPort;
        return *this;
    }
    bool operator==(const LinkTemplate<InPrt, OutPrt>& that) const {
        return
            m_inputPort == that.m_inputPort &&
            m_outputPort == that.m_outputPort;
    }
    bool operator!=(const LinkTemplate<InPrt, OutPrt>& that) const {
        return !(*this == that);
    }

private:
    InPrt *m_inputPort;
    OutPrt *m_outputPort;
};

#define PropClass(ClassName, PropType, PassedPropType, propGetter, propSetter) \
    class ClassName { \
    public: \
        explicit ClassName(PassedPropType value) : m_value(value) {} \
        ClassName() : m_value() {} \
        PassedPropType propGetter() const { \
            return m_value; \
        } \
        void propSetter(PassedPropType value) { \
            m_value = value; \
        } \
    private: \
        PropType m_value; \
    };

#define PropRefClass(ClassName, PropType, propRefAccessor) \
    class ClassName { \
    public: \
        ClassName() : m_value() {} \
        explicit ClassName(const PropType& value) : m_value(value) {} \
        PropType& propRefAccessor() { \
            return m_value; \
        } \
        const PropType& propRefAccessor() const { \
            return m_value; \
        } \
    private: \
        PropType m_value; \
    };

inline void setFlag(int& flags, int flag) {
    flags |= flag;
}
inline void clearFlag(int& flags, int flag) {
    flags &= ~flag;
}
inline void setFlag(int& flags, int flag, bool set) {
    if (set)
        setFlag(flags, flag);
    else
        clearFlag(flags, flag);
}

inline bool hasFlag(int flags, int flag) {
    return (flags & flag) != 0;
}

#define DENY_INSTANCE_COPYING(ClassName) \
    private: \
        ClassName(const ClassName&); \
        ClassName& operator=(const ClassName&);


class Box;
class Port;
class InputPort;
class OutputPort;
typedef QList<InputPort*> InputPorts;
typedef QList<OutputPort*> OutputPorts;
typedef LinkTemplate<InputPort, OutputPort> Link;
typedef QList<const Link*> ConstLinkList;

class RuntimeBox;
class RuntimePort;
class RuntimeInputPort;
class RuntimeOutputPort;
typedef QList<RuntimeInputPort*> RuntimeInputPorts;
typedef QList<RuntimeOutputPort*> RuntimeOutputPorts;
typedef LinkTemplate<RuntimeInputPort, RuntimeOutputPort> RuntimeLink;

class Simulation;
class Runner;

class PortFormat
{
public:
    PortFormat() : m_flags(0) {}
    explicit PortFormat(const QVector<int>& size) : m_size(size), m_flags(PortFormatValid) {}
    explicit PortFormat(int length) : m_size(1, length), m_flags(PortFormatValid) {}
    PortFormat(int length1, int length2) : m_size(2), m_flags(PortFormatValid) {
        m_size[0] = length1;
        m_size[1] = length2;
    }
    int dimension() const { return m_size.size(); }
    const QVector<int>& size() const { return m_size; }
    int size(int index) const {
        Q_ASSERT(index >= 0   &&   index < m_size.size());
        return m_size[index];
    }
    bool isValid() const {
        return hasFlag(m_flags, PortFormatValid);
    }
    PortFormat& setValid(bool valid = true) {
        setFlag(m_flags, PortFormatValid, valid);
        return *this;
    }
    bool isFixed() const {
        return hasFlag(m_flags, PortFormatFixed);
    }
    PortFormat& setFixed(bool fixed = true) {
        setFlag(m_flags, PortFormatFixed, fixed);
        return *this;
    }

    PortFormat& setDimension(int dimension) {
        m_size = QVector<int>(dimension, 0);
        setValid(false);
        return *this;
    }
    PortFormat& setSize(const QVector<int>& size) {
        m_size = size;
        setValid();
        return *this;
    }
    PortFormat& setSize(int length) {
        m_size = QVector<int>(1, length);
        setValid();
        return *this;
    }
    PortFormat& setSize(int index, int length) {
        Q_ASSERT(index >= 0   &&   index < m_size.size());
        m_size[index] = length;
        return *this;
    }
    bool isEempty() const {
        return m_size.isEmpty();
    }
    int dataSize() const {
        int result = 1;
        foreach(int s, m_size)
            result *= s;
        return result;
    }
    int flattenIndex(const int *multiIndex) const {
        int result = 0,   m = 1;
        for(int i=m_size.size()-1; i>=0; m*=m_size[i], --i) {
            Q_ASSERT(multiIndex[i] >= 0   &&   multiIndex[i] < m_size[i]);
            result += m*multiIndex[i];
        }
        Q_ASSERT(result >= 0   &&   result < dataSize());
        return result;
    }
    bool operator==(const PortFormat& that) const {
        if (isValid() != that.isValid())
            return false;
        return !isValid()   ||   m_size == that.m_size;
    }
    bool operator!=(const PortFormat& that) const {
        return !(*this == that);
    }
private:
    enum PortFormatFlags {
        PortFormatValid = 0x01,
        PortFormatFixed = 0x02
    };

    QVector<int> m_size;
    int m_flags;
};

PropRefClass(WithPortFormat, PortFormat, format)

PropClass(WithSimulation, Simulation*, Simulation*, simulation, setSimulation)

PropClass(WithHelpString, QString, const QString&, helpString, setHelpString)

class PortEntryHints
{
public:
    bool hasHints() const {
        return !m_hints.isEmpty();
    }
    QStringList hints() const {
        return m_hints;
    }
    PortEntryHints& setHints(const QStringList& hints) {
        m_hints = hints;
        return *this;
    }

private:
    QStringList m_hints;
};

PropRefClass(WithPortEntryHints, PortEntryHints, entryHints)

struct BoxProperty
{
    explicit BoxProperty(const QString& name, const QString& helpString) :
        name(name), helpString(helpString) {}
    QString name;
    QString helpString;
};
typedef QList<BoxProperty> BoxPropertyList;

class Box :
    public QObject,
    protected QScriptable,
    public Named,
    public WithSimulation,
    public WithHelpString
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
public:
    typedef QSharedPointer<Box> Ptr;

    typedef Box* (*Ctor)();

    explicit Box(QObject *parent = 0) : QObject(parent) {}
    virtual ~Box() {}

    virtual InputPorts inputPorts() const = 0;
    virtual OutputPorts outputPorts() const = 0;
    virtual BoxPropertyList boxProperties() const {
        return BoxPropertyList();
    }
    virtual void checkPortFormat() const = 0;
    virtual bool propagatePortFormat() = 0;

    virtual RuntimeBox *newRuntimeBox() const = 0;
};

class Port :
    public OwnedBy<Box>,
    public WithPortFormat,
    public Named,
    public WithHelpString,
    public WithPortEntryHints
{
public:
    Port() {}
    virtual ~Port() {}
    Port(const QString& name, Box *owner, const PortFormat& format = PortFormat()) :
        OwnedBy<Box>(owner), WithPortFormat(format), Named(name)
    {}
};

class InputPort :
    public Port,
    public LinkedToOne<const Link>
{
public:
    InputPort() {}
    InputPort(const QString& name, Box *owner, const PortFormat& format = PortFormat()) :
        Port(name, owner, format) {}
};

class OutputPort :
    public Port,
    public LinkedToMany<const Link>
{
public:
    OutputPort() {}
    OutputPort(const QString& name, Box *owner, const PortFormat& format = PortFormat()) :
        Port(name, owner, format) {}
};




class PortState
{
public:
    PortState() : m_flags(0) {}
    bool isOpen() const { return (m_flags & PortClosed) == 0; }
    bool isClosed() const { return (m_flags & PortClosed) != 0; }
    bool hasData() const { return (m_flags & PortDataValid) != 0; }
    PortState& open() {
        clearFlag(m_flags, PortClosed);
        return *this;
    }
    PortState& close() {
        setFlag(m_flags, PortClosed);
        return *this;
    }
    PortState& setValid() {
        setFlag(m_flags, PortDataValid);
        return *this;
    }
    PortState& invalidate() {
        clearFlag(m_flags, PortDataValid);
        return *this;
    }
private:
    enum PortStateFlags {
        PortDataValid = 0x01,
        PortClosed = 0x02
    };
    int m_flags;
};

class PortData
{
public:
    PortData() : m_size(0), m_data(0) {}
    PortData(int size, double *data) : m_size(size), m_data(data) {}
    int size() const { return m_size; }
    double *data() const {
        return m_data;
    }
    operator double*() const {
        return m_data;
    }
    void copyFrom(const double *data) const {
        for (int i=0; i<m_size; ++i)
            m_data[i] = data[i];
    }
    void copyTo(double *data) const {
        for (int i=0; i<m_size; ++i)
            data[i] = m_data[i];
    }

private:
    int m_size;
    double *m_data;
};

PropClass(WithPort, const Port*, const Port*, port, setPort)
PropRefClass(WithPortState, PortState, state)
PropClass(WithPortData, PortData, const PortData&, data, setData)

PropClass(WithRuntimeInputPorts, RuntimeInputPorts, const RuntimeInputPorts&, inputPorts, setInputPorts)
PropClass(WithRuntimeOutputPorts, RuntimeOutputPorts, const RuntimeOutputPorts&, outputPorts, setOutputPorts)
PropClass(WithRunner, Runner*, Runner*, runner, setRunner)
PropClass(WithBusyFlag, bool, bool, isBusy, setBusy)

class RuntimeBox :
    public OwnedBy<const Box>,
    public WithRuntimeInputPorts,
    public WithRuntimeOutputPorts,
    public WithRunner
{
public:
    typedef QSharedPointer<RuntimeBox> Ptr;
    virtual ~RuntimeBox () {}
    typedef bool (RuntimeBox::*PortNotifier)();

protected:
    template<class ThisClass>
    static PortNotifier toPortNotifier(bool (ThisClass::*notifier)()) {
        return static_cast<PortNotifier>(notifier);
    }

};

class RuntimePort :
    public OwnedBy<RuntimeBox>,
    public WithPort
{
public:
    void init(RuntimeBox *owner, Port *port) {
        setOwner(owner);
        setPort(port);
    }
};

PropClass(WithPortNotifier, RuntimeBox::PortNotifier, RuntimeBox::PortNotifier, portNotifier, setPortNotifier)

class RuntimeInputPort :
    public RuntimePort,
    public LinkedToOne<const RuntimeLink>,
    public WithPortNotifier
{
public:
    void init(RuntimeBox *owner, Port *port, RuntimeBox::PortNotifier portNotifier = 0) {
        RuntimePort::init(owner, port);
        setPortNotifier(portNotifier);
    }

    bool hasPortNotifier() const {
        return portNotifier() != 0;
    }
    inline bool activate() const;

    inline RuntimeOutputPort *outputPort() {
        return link()->outputPort();
    }
    inline const RuntimeOutputPort *outputPort() const {
        return link()->outputPort();
    }

    inline const PortData& data() const;
    inline RuntimeInputPort& setData(const PortData& data);

    inline PortState& state();
    inline const PortState& state() const;
};

class RuntimeOutputPort :
    public RuntimePort,
    public LinkedToMany<const RuntimeLink>,
    public WithPortState,
    public WithPortData
{
public:
    void init(RuntimeBox *owner, Port *port, const PortData& data = PortData()) {
        RuntimePort::init(owner, port);
        setData(data);
    }

    bool hasActiveLinks() const {
        foreach (const RuntimeLink *link, links())
            if(link->inputPort()->hasPortNotifier())
                return true;
        return false;
    }
    bool activateLinks() const {
        bool result = true;
        foreach (const RuntimeLink *link, links())
            result = link->inputPort()->activate() && result;
        return result;
    }
};

inline const PortData& RuntimeInputPort::data() const {
    return link()->outputPort()->data();
}

inline RuntimeInputPort& RuntimeInputPort::setData(const PortData& data) {
    link()->outputPort()->setData(data);
    return *this;
}

inline PortState& RuntimeInputPort::state() {
    return link()->outputPort()->state();
}

inline const PortState& RuntimeInputPort::state() const {
    return link()->outputPort()->state();
}

typedef QList< Box* > BoxList;
typedef QList< Link > LinkList;

class EQUARES_CORESHARED_EXPORT Simulation :
    public QObject,
    protected QScriptable
{
    Q_OBJECT
public:
    explicit Simulation(QObject *parent = 0) : QObject(parent) {}
    BoxList boxes() const {
        return linkBoxes(m_links);
    }
    const LinkList& links() const {
        return m_links;
    }

    const Link *findLink(OutputPort *outputPort, InputPort *inputPort) const
    {
        Q_ASSERT(outputPort);
        Q_ASSERT(inputPort);
        const Link *link = inputPort->link();
        if (link   &&   link->outputPort() == outputPort)
            return link;
        return 0;
    }
    bool isConnected(OutputPort *outputPort, InputPort *inputPort) const
    {
        return findLink(outputPort, inputPort) != 0;
    }
    Simulation& addLink(const Link& link)
    {
        Q_ASSERT(!link.inputPort()->isConnected());
        m_links << link;
        const Link *l = &m_links.last();
        link.inputPort()->setLink(l);
        OutputPort *o = link.outputPort();
        o->links() << l;
        return *this;
    }
    Simulation& removeLink(const Link& link)
    {
        Q_ASSERT(isConnected(link.outputPort(), link.inputPort()));
        int i = m_links.indexOf(link);
        Q_ASSERT(i != -1);
        const Link *l = &m_links[i];
        ConstLinkList& portLinks = l->outputPort()->links();
        int ip = portLinks.indexOf(l);
        Q_ASSERT(ip!= -1);
        portLinks.removeAt(ip);
        Q_ASSERT(l->inputPort()->isConnected());
        l->inputPort()->setLink(0);
        m_links.removeAt(i);
        return *this;
    }

    void initPortFormat();

    using QScriptable::engine;

public slots:
    void setLinks(const LinkList& links);
    void run() const;

private:
    LinkList m_links;

    static bool hasUnknownInputFormats(const Box *box);
    static BoxList linkBoxes(const LinkList& links);
};

class EQUARES_CORESHARED_EXPORT Runner
{
public:
    void run(const Simulation *sim);

    void postPortActivation(const RuntimeInputPort *port);
    void quit();

private:
    QList< RuntimeBox::Ptr > m_rtboxes;
    QList< RuntimeLink > m_rtlinks;

    QList<const RuntimeInputPort *> m_queue;
    bool m_quitRequested;
};

inline bool RuntimeInputPort::activate() const {
    RuntimeBox::PortNotifier notifier = portNotifier();
    if (!notifier)
        return true;
    return (owner()->*notifier)();
}

class EQUARES_CORESHARED_EXPORT BoxFactory
{
public:
    static Box *newBox(const QString& name);
    static Box::Ctor boxCtor(const QString& name);
    static void registerBoxType(const QString& name, Box::Ctor ctor);
    static QStringList boxTypes();
private:
    typedef QMap<QString, Box::Ctor> BoxMap;
    static BoxMap *m_registry;
    static BoxMap& registry();
};

template< class BoxType >
class BoxRegistrator {
public:
    BoxRegistrator(const QString& name) {
        BoxFactory::registerBoxType(name, &BoxRegistrator<BoxType>::newBox);
    }
private:
    static Box *newBox() { return new BoxType(); }
};

#ifdef EQUARES_CORE_STATIC
#define REGISTER_BOX(ClassName, UserName)
#define REGISTER_BOX_STATIC(ClassName, UserName) \
    static BoxRegistrator<ClassName> boxRegistratorFor##ClassName(UserName);
#else // EQUARES_CORE_STATIC
#define REGISTER_BOX(ClassName, UserName) \
    static BoxRegistrator<ClassName> boxRegistratorFor##ClassName(UserName);
#define REGISTER_BOX_STATIC(ClassName, UserName)
#endif // EQUARES_CORE_STATIC

#endif // EQUARES_CORE_H
