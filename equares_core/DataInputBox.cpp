#include "DataInputBox.h"

DataInputBox::DataInputBox(QObject *parent) :
    Box(parent),
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



DataInputRuntimeBox::DataInputRuntimeBox(const DataInputBox *box)
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
    m_inputFeedbackTime.start();
    m_inputCheckTime.start();
    m_unititializedInputPort = true;
}

void DataInputRuntimeBox::registerInput()
{
    m_inputId = ThreadManager::instance()->registerInput(inputInfo()[0]);
}

void DataInputRuntimeBox::acquireInteractiveInput()
{
    if (!fetchInputPortData())
        return;
    if (ThreadManager::instance()->readInput(m_iinputData, m_inputId, false)) {
        transformData(m_data.data(), m_iinputData.data());
        m_iinputDataValid = true;
        throw BoxBreakException(this);
    }
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
    const int DataInputFeedbackDelay = 1000;
    if (m_unititializedInputPort || m_inputFeedbackTime.elapsed() >= DataInputFeedbackDelay) {
        m_unititializedInputPort = false;
        m_inputFeedbackTime.restart();
        QStringList input;
        foreach (double d, inputData(m_data.data()))
            input << QString::number(d);
        EQUARES_COUT << "input: " << owner()->name() << " " << input.join(" ") << endl;
    }
    return true;
}

bool DataInputRuntimeBox::activate(int)
{
    const int InputCheckDelay = 100;
    if (m_inputCheckTime.elapsed() > InputCheckDelay) {
        m_inputCheckTime.restart();
        acquireInteractiveInput();
    }
    return true;
}

bool DataInputRuntimeBox::processInput(int)
{
    Q_ASSERT(m_in.state().hasData());
    m_dataValid = false;
    if (fetchInputPortData()) {
        if (m_iinputDataValid)
            transformData(m_data.data(), m_iinputData.data());
        return m_out.activateLinks();
    }
    else
        return false;
}
