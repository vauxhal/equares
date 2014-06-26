#include "PauseBox.h"

REGISTER_BOX(PauseBox, "Pause")

PauseBox::PauseBox(QObject *parent) :
    Box(parent),
    m_activator("activator", this)
{
}

InputPorts PauseBox::inputPorts() const {
    return InputPorts() << &m_activator;
}

OutputPorts PauseBox::outputPorts() const {
    return OutputPorts();
}

void PauseBox::checkPortFormat() const
{
}

bool PauseBox::propagatePortFormat() {
    return false;
}

RuntimeBox *PauseBox::newRuntimeBox() const {
    return new PauseRuntimeBox(this);
}



PauseRuntimeBox::PauseRuntimeBox(const PauseBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_activator.init(this, in[0], toPortNotifier(&PauseRuntimeBox::activate));
    setInputPorts(RuntimeInputPorts() << &m_activator);
}

bool PauseRuntimeBox::activate(int)
{
    forever {
        foreach (RuntimeBox *rtbox, runner()->rtboxes())
            rtbox->acquireInteractiveInput();
        const unsigned long InteractiveInputProcessingDelay = 100;
        ThreadManager::msleep(InteractiveInputProcessingDelay);
    }
    return true;
}
