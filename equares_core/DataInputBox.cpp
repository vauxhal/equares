#include "DataInputBox.h"

DataInputBox::DataInputBox(QObject *parent) :
    Box(parent),
    m_sync(true),
    m_loop(true),
    m_activator("activator", this),
    m_in("input", this),
    m_out("output", this)
{
}

InputPorts DataInputBox::inputPorts() const {
    return InputPorts() << &m_activator << &m_in;
}

OutputPorts DataInputBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void DataInputBox::checkPortFormat() const
{
    if (!m_in.format().isValid())
        throwBoxException("DataInputBox: no format is specified for port 'input'");
    if (m_in.format().dimension() != 1)
        throwBoxException("DataInputBox: an 1D format was expected for port 'input'");
    if (m_in.format() != m_out.format())
        throwBoxException("DataInputBox: Incompatible input/output port formats");
}

bool DataInputBox::propagatePortFormat() {
    if (m_in.format().isValid() == m_out.format().isValid())
        return false;
    if (m_in.format().isValid())
        m_out.format() = m_in.format();
    else
        m_in.format() = m_out.format();
    return true;
}

bool DataInputBox::sync() const {
    return m_sync;
}

DataInputBox& DataInputBox::setSync(bool sync) {
    m_sync = sync;
    return *this;
}

bool DataInputBox::loop() const {
    return m_loop;
}

DataInputBox& DataInputBox::setLoop(bool loop) {
    m_loop = loop;
    return *this;
}



DataInputRuntimeBox::DataInputRuntimeBox(const DataInputBox *box) :
    m_sync(box->sync()),
    m_loop(box->loop())
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_activator.init(this, in[0], toPortNotifier(&DataInputRuntimeBox::activate));
    m_in.init(this, in[1], toPortNotifier(&DataInputRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_activator << &m_in);

    OutputPorts out = box->outputPorts();
    m_data.resize(out[0]->format().dataSize());
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);

    m_inputId = -1;
    m_dataValid = false;
    m_iinputDataValid = false;
    m_time.start();
    m_unititializedInputPort = true;
}

void DataInputRuntimeBox::registerInput()
{
    m_inputId = ThreadManager::instance()->registerInput(inputInfo()[0]);
}

bool DataInputRuntimeBox::fetchInputPortData()
{
    if (m_dataValid)
        return true;
    if (!m_in.state().hasData())
        return false;
    Q_ASSERT(m_in.data().size() == m_data.size());
    m_in.data().copyTo(m_data.data());
    m_out.state().setValid();
    m_dataValid = true;

    // Report input data corresponding to new port data
    if (!m_sync) {
        const int DataInputFeedbackDelay = 1000;
        if (m_unititializedInputPort || m_time.elapsed() >= DataInputFeedbackDelay) {
            m_unititializedInputPort = false;
            m_time.restart();
            QStringList input;
            foreach (double d, inputData(m_data.data()))
                input << QString::number(d);
            EQUARES_COUT << "input: " << owner()->name() << " " << input.join(" ") << endl;
        }
    }
    return true;
}

bool DataInputRuntimeBox::activate()
{
    ScopedInc incEc(m_ec);
    if (m_ec != 1) {
        m_iinputDataValid = false;
        throw BoxBreakException(this);
    }
    forever {
        if (!fetchInputPortData())
            return false;
        if (!m_iinputDataValid) {
            m_iinputData = ThreadManager::instance()->readInput(m_inputId, m_sync);
            m_iinputDataValid = true;
        }
        if (m_iinputData.isEmpty()) {
            if (!m_sync)
                m_iinputDataValid = false;
            return true;
        }
        transformData(m_data.data(), m_iinputData.data());
        m_out.state().setValid();
        if (m_out.activateLinks())
            m_iinputDataValid = false;
        else
            return false;
        if (!(m_loop && m_sync))
            return true;
    }
}

bool DataInputRuntimeBox::processInput()
{
    Q_ASSERT(m_in.state().hasData());
    m_dataValid = false;
    if (m_sync)
        return activate();
    else {
        if (fetchInputPortData())
            return m_out.activateLinks();
        else
            return false;
    }
}
