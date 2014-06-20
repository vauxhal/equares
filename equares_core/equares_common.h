#ifndef EQUARES_COMMON_H
#define EQUARES_COMMON_H

#include <QString>

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

class Typed
{
public:
    Typed() {}
    explicit Typed(const QString& type) : m_type(type) {}
    QString type() const {
        return m_type;
    }
    void setType(const QString& type) {
        m_type = type;
    }
private:
    QString m_type;
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

class EntryCounter {
public:
    EntryCounter() : m_counter(0) {}
    ~EntryCounter() {
        Q_ASSERT(m_counter == 0);
    }
    operator int() const {
        return m_counter;
    }
    int inc() {
        return ++m_counter;
    }
    int dec() {
        return --m_counter;
    }
private:
    int m_counter;
};

class ScopedInc {
public:
    explicit ScopedInc(EntryCounter& ec) : m_ec(ec) {
        m_ec.inc();
    }
    ~ScopedInc() {
        m_ec.dec();
    }
private:
    EntryCounter& m_ec;
};

#endif // EQUARES_COMMON_H
