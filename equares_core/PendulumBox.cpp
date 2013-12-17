#include "PendulumBox.h"

REGISTER_BOX(PendulumBox, "Pendulum")

PendulumBox::PendulumBox(QObject *parent) : OdeBox<PendulumOdeTraits>(parent)
{
    setHelpString(tr("Simple pendulum"));
    m_param.setHelpString(tr("Parameters of the simple pendulum"));
    m_param.entryHints().setHints(QStringList() << "l" << "g");
    m_state.entryHints().setHints(QStringList() << "q" << "dq" << "t");
    m_rhs.entryHints().setHints(QStringList() << "q" << "dq");
}
