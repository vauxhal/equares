// cmd.h

#ifndef CTM_EQUARES_CORE_CMD_H
#define CTM_EQUARES_CORE_CMD_H

#include <QStringList>
#include <QSharedPointer>

namespace ctm {
namespace equares {

class CommandProcessor;

typedef QStringList CommandArgs;

template< class X >
class Named
{
public:
    Named() : m_value() {}
    Named( const QString& name, const X& value) : m_name(name), m_value(value) {}
    QString name() const {
        return m_name;
    }
    X& value() { return m_value; }
    const X& value() const { return m_value; }
    operator X& () { return m_value; }
    operator const X& () const { return m_value; }
private:
    QString m_name;
    X m_value;
};

struct CommandSet
{
    typedef QSharedPointer<CommandSet> Ptr;
    typedef void ( CommandSet::*Handler )( const CommandArgs& );
    typedef Named<Handler> NamedHandler;
    virtual QList< NamedHandler > handlers() const = 0;
};

#define CMD_HANDLER( h ) static_cast<CommandSet::*Handler>( &h )

struct CommandTarget
{
    virtual QList<CommandSet::Ptr> commandSets() const = 0;
};

struct Command
{
public:
    Command() : m_cs(0), m_h(0) {}
    Command( CommandSet *cs, CommandSet::Handler h ) : m_cs(cs), m_h(h) {}
    void exec( const CommandArgs& args ) {
        Q_ASSERT( m_cs );
        Q_ASSERT( m_h );
        (m_cs->*m_h)( args );
    }
    void operator()( const CommandArgs& args ) { exec(args); }

private:
    CommandSet *m_cs;
    CommandSet::Handler m_h;
};

struct CommandProcessor
{
    void registerCommand( const QString& name, const Command& command );
    void exec( const CommandArgs& args );
};

} // end namespace equares
} // end namespace ctm

#endif // CTM_EQUARES_CORE_CMD_H
