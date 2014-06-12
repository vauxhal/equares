#include "equares_core.h"
#include <QSet>

void Box::throwBoxException(const QString& what) const
{
    throw EquaresBoxException(this, what);
}

void Simulation::initPortFormat()
{
    // Clear all non-fixed formats; check that fixed formats are valid
    foreach (const Link& link, m_links) {
        if (!link.inputPort()->format().isFixed())
            link.inputPort()->format().setValid(false);
        else {
            Q_ASSERT(link.inputPort()->format().isValid());
        }
    }

    forever {
        bool progress = false;

        QSet<Box*> unresolvedBoxes;

        // Propagate all valid formats according to links
        foreach (const Link& link, m_links) {
            PortFormat
                &f1 = link.inputPort()->format(),
                &f2 = link.outputPort()->format();

            // deBUG, TODO: Remove
            QString linkstr = QString("%1:%2 -> %3:%4").arg(
                    link.outputPort()->owner()->name(),
                    link.outputPort()->name(),
                    link.inputPort()->owner()->name(),
                    link.inputPort()->name());
            Q_UNUSED(linkstr);

            if (f1.isValid()) {
                if (f2.isValid()) {
                    if (f1 != f2) {
                        throw EquaresException(QString
                            ("Incompatible port formats at link %1:%2 -> %3:%4").arg(
                                link.outputPort()->owner()->name(),
                                link.outputPort()->name(),
                                link.inputPort()->owner()->name(),
                                link.inputPort()->name()));
                    }
                }
                else {
                    f2 = f1;
                    progress = true;
                }
            }
            else if (f2.isValid()) {
                f1 = f2;
                progress = true;
            }
            else
                unresolvedBoxes << link.outputPort()->owner() << link.inputPort()->owner();
        }
        if (unresolvedBoxes.isEmpty())
            // Done
            break;

        // Propagate format inside unresolved boxes
        foreach (Box *box, unresolvedBoxes)
            progress = box->propagatePortFormat() || progress;

        if (!progress)
            // Got stuck
            throw EquaresException("Unable to resolve port format");
    }

    // Check final port formats
    foreach (Box *box, boxes())
        box->checkPortFormat();
}

bool Simulation::hasUnknownInputFormats(const Box *box)
{
    foreach (InputPort* port, box->inputPorts())
        if (!port->format().isValid())
            return true;
    return false;
}

void Simulation::setLinks(const LinkList& links)
{
    try {
        // Clear boxes' links; set simulation
        foreach (Box *box, linkBoxes(links)) {
            foreach (InputPort *port, box->inputPorts())
                port->setLink(0);
            foreach (OutputPort *port, box->outputPorts())
                port->links().clear();
            box->setSimulation(this);
        }

        // Add specified links
        foreach (const Link& link, links)
            addLink(link);

        // Resolve port formats
        initPortFormat();
    } catch(const EquaresException& e) {
        context()->throwError(e.what());
    }
}

BoxList Simulation::linkBoxes(const LinkList& links)
{
    BoxList result;
    foreach (const Link& link, links) {
        if (!result.contains(link.inputPort()->owner()))
            result << link.inputPort()->owner();
        if (!result.contains(link.outputPort()->owner()))
            result << link.outputPort()->owner();
    }
    return result;
}

void Simulation::run() const
{
    try {
        Runner *runner = new Runner;
        runner->start(this);
    } catch(const EquaresException& e) {
        context()->throwError(e.what());
    }
}



void Runner::start(const Simulation *sim)
{
    // Clear any previous runtime configuration
    m_rtboxes.clear();
    m_rtlinks.clear();

    // Create runtime configuration
    QMap<const Box*, RuntimeBox*> box2rt;
    foreach (Box *box, sim->boxes()) {
        RuntimeBox *rtbox = box->newRuntimeBox();
        rtbox->setRunner(this);
        box2rt[box] = rtbox;
        m_rtboxes << RuntimeBox::Ptr(rtbox);
    }
    foreach (const Link& link, sim->links()) {
        const Box
            *boxin = link.inputPort()->owner(),
            *boxout = link.outputPort()->owner();
        const RuntimeBox
            *rtboxin = box2rt.value(boxin),
            *rtboxout = box2rt.value(boxout);
        int iin = boxin->inputPorts().indexOf(link.inputPort()),
            iout = boxout->outputPorts().indexOf(link.outputPort());
        RuntimeInputPort *in = rtboxin->inputPorts()[iin];
        RuntimeOutputPort *out = rtboxout->outputPorts()[iout];
        m_rtlinks << RuntimeLink()
            .setInputPort(in)
            .setOutputPort(out);
        RuntimeLink *rtlink = &m_rtlinks.last();
        in->setLink(rtlink);
        out->links() << rtlink;
    }

    // Start simulation
    ThreadManager::instance()->start(this);
}

void Runner::run()
{
    // Announce output files
    OutputFileInfoList ofi;
    QTextStream& out = EQUARES_COUT;
    {
        // Gather output file information from all boxes
        foreach (const RuntimeBox::Ptr& box, m_rtboxes)
            ofi << box->outputFileInfo();

        // Check that there are no duplicate output file names
        QSet<QString> fileNames;
        foreach(const OutputFileInfo& info, ofi)
            if (fileNames.contains(info.name()))
                throw EquaresException(QObject::tr("Duplicate output file name '%1'").arg(info.name()));
            else
                fileNames << info.name();

        // Generate output file stubs
        foreach(const OutputFileInfo& info, ofi)
            info.createStubFile();

        out << "begin file announcement" << endl;
        foreach(const OutputFileInfo& info, ofi)
            out << info.toString() << endl;
        out << "end file announcement" << endl;
    }

    // Initiate process
    m_queue.clear();
    foreach (const RuntimeBox::Ptr& box, m_rtboxes)
        if (!box->generator() && box->inputPorts().empty())
            foreach (const RuntimeOutputPort *port, box->outputPorts())
                foreach (const RuntimeLink *link, port->links())
                    postPortActivation(link->inputPort()->owner(), link->inputPort()->portNotifier());
    foreach (const RuntimeBox::Ptr& box, m_rtboxes)
        if (box->generator()) {
            Q_ASSERT(box->inputPorts().empty());
            postPortActivation(box.data(), box->generator());
        }

    // Process enqueued boxes
    m_terminationRequested = 0;
    while (!m_queue.isEmpty()) {
        if (terminationRequested())
            break;
        if (!m_queue.first().activate())
            m_queue << m_queue.first();
        m_queue.removeFirst();
    }

    // Report final state of output files
    foreach(const OutputFileInfo& info, ofi)
        out << "file: " << info.name() << endl;
}

void Runner::requestTermination() {
    m_terminationRequested = 1;
}

bool Runner::terminationRequested() const {
#if QT_VERSION >= 0x050000
    int terminationRequested = m_terminationRequested.load();
#else
    int terminationRequested = m_terminationRequested;
#endif
    return terminationRequested != 0;
}

void Runner::postPortActivation(RuntimeBox *box, RuntimeBox::PortNotifier notifier) {
    Q_ASSERT(box);
    if (notifier)
        m_queue << QueueItem(box, notifier);
}



BoxFactory::BoxMap *BoxFactory::m_registry = 0;

Box *BoxFactory::newBox(const QString& name) {
    return boxCtor(name)();
}

Box::Ctor BoxFactory::boxCtor(const QString& name) {
    Box::Ctor ctor = registry().value(name);
    if (!ctor)
        throw EquaresException(QString("Unknown box type '%1'").arg(name));
    return ctor;
}

void BoxFactory::registerBoxType(const QString& name, Box::Ctor ctor) {
    BoxMap& r = registry();
    if (r.contains(name))
        throw EquaresException(QString("Box type '%1' is already registered").arg(name));
    r[name] = ctor;
}

QStringList BoxFactory::boxTypes() {
    return registry().keys();
}

BoxFactory::BoxMap& BoxFactory::registry() {
    if (!m_registry)
        m_registry = new BoxMap;
    return *m_registry;
}
