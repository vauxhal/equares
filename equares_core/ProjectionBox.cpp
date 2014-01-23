#include "ProjectionBox.h"

REGISTER_BOX(ProjectionBox, "Projection")

ProjectionBox::ProjectionBox(QObject *parent) :
    Box(parent),
    m_in("input", this),
    m_out("output", this, PortFormat(0).setFixed())
{
}

InputPorts ProjectionBox::inputPorts() const {
    return InputPorts() << &m_in;
}

OutputPorts ProjectionBox::outputPorts() const {
    return OutputPorts() << &m_out;
}

void ProjectionBox::checkPortFormat() const
{
    if (m_out.format() != PortFormat(m_indices.size()))
        throw EquaresException("ProjectionBox: Invalid output port format");
    int inputSize = m_in.format().dataSize();
    foreach (int index, m_indices) {
        if (index < 0   ||   index >= inputSize)
            throw EquaresException("ProjectionBox: Invalid input port format or invalid projection indices");
    }
}

bool ProjectionBox::propagatePortFormat() {
    return false;
}

RuntimeBox *ProjectionBox::newRuntimeBox() const {
    return new ProjectionRuntimeBox(this);
}

const QList<int>& ProjectionBox::indices() const {
    return m_indices;
}

ProjectionBox& ProjectionBox::setIndices(const QList<int>& indices) {
    m_indices = indices;
    m_out.format().setSize(m_indices.size());
    return *this;
}



ProjectionRuntimeBox::ProjectionRuntimeBox(const ProjectionBox *box)
{
    setOwner(box);

    InputPorts in = box->inputPorts();
    m_in.init(this, in[0], toPortNotifier(&ProjectionRuntimeBox::processInput));
    setInputPorts(RuntimeInputPorts() << &m_in);

    m_indices = box->indices().toVector();
    m_data.resize(m_indices.size());

    OutputPorts out = box->outputPorts();
    m_out.init(this, out[0], PortData(m_data.size(), m_data.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_out);
}

bool ProjectionRuntimeBox::processInput()
{
    Q_ASSERT(m_in.outputPort()->state().hasData());
    double *in = m_in.data().data();
    double *out = m_out.data().data();
    for (int i=0; i<m_indices.size(); ++i)
        out[i] = in[m_indices[i]];
    m_out.state().setValid();
    return m_out.activateLinks();
}
