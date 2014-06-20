#ifndef SIMPLEINPUTBOX_H
#define SIMPLEINPUTBOX_H

#include "equares_core.h"
#include "equares_script.h"

struct SimpleInputBoxParamItem
{
    int index;
    QString name;
    SimpleInputBoxParamItem() : index(0) {}
    SimpleInputBoxParamItem(int index, const QString& name) : index(index), name(name) {}
};

typedef QVector<SimpleInputBoxParamItem> SimpleInputBoxParam;

Q_DECLARE_METATYPE(SimpleInputBoxParamItem)
Q_DECLARE_METATYPE(SimpleInputBoxParam)

class EQUARES_CORESHARED_EXPORT SimpleInputBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(SimpleInputBoxParam param READ param WRITE setParam)
    Q_PROPERTY(bool sync READ sync WRITE setSync)
    Q_PROPERTY(bool loop READ loop WRITE setLoop)
public:
    explicit SimpleInputBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    typedef SimpleInputBoxParamItem ParamItem;
    typedef SimpleInputBoxParam Param;

    Param param() const;
    SimpleInputBox& setParam(const Param& param);
    bool sync() const;
    SimpleInputBox& setSync(bool sync);
    bool loop() const;
    SimpleInputBox& setLoop(bool loop);

private:
    Param m_param;
    bool m_sync;
    bool m_loop;
    mutable InputPort m_activator;
    mutable InputPort m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT SimpleInputRuntimeBox : public RuntimeBox
{
public:
    explicit SimpleInputRuntimeBox(const SimpleInputBox *box);
    InputInfoList inputInfo() const;
    void registerInput();

private:
    RuntimeInputPort m_activator;
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;

    typedef SimpleInputBoxParamItem ParamItem;
    typedef SimpleInputBoxParam Param;

    Param m_param;
    bool m_sync;
    bool m_loop;
    QVector<double> m_data;
    bool m_dataValid;
    bool fetchInputPortData();
    QVector<double> m_iinputData;
    bool m_iinputDataValid;
    int m_inputId;

    bool activate();
    bool processInput();
};

#endif // SIMPLEINPUTBOX_H
