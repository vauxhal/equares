/*
EquaRes - a tool for numerical analysis of equations.
Copyright (C) 2014 Stepan Orlov

This file is part of EquaRes.

Full text of copyright notice can be found in file copyright_notice.txt in the EquaRes root directory.
License agreement can be found in file LICENSE.md in the EquaRes root directory.
*/

#include "CxxTransformBox.h"
#include <functional>

template< class Method >
static typename Method::result_type callLibProxyMethod(const CxxTransformBox *box, const Method& method)
{
    const CxxTransformBox::TransformLibProxy *libProxy = box->transformLibProxy();
    Q_ASSERT( libProxy );
    if (libProxy->isNull())
        throw EquaresBoxException(box, "CxxTransformBox: No source is specified");
    return method(libProxy);
}



REGISTER_BOX(CxxTransformBox, "CxxTransform")

CxxTransformBox::CxxTransformBox(QObject *parent) :
    Box(parent),
    m_param("parameters", this, PortFormat(0).setFixed()),
    m_input("input", this, PortFormat(0).setFixed()),
    m_output("output", this, PortFormat(0).setFixed()),
    m_useQmake(true)
{
}

InputPorts CxxTransformBox::inputPorts() const {
    return InputPorts() << &m_param << &m_input;
}

OutputPorts CxxTransformBox::outputPorts() const {
    return OutputPorts() << &m_output;
}

void CxxTransformBox::checkPortFormat() const {
    if (m_param.format() != paramFormat())
        throwBoxException("CxxTransformBox: port 'parameters' has an invalid format");
    if (m_input.format() != inputFormat())
        throwBoxException("CxxTransformBox: port 'input' has an invalid format");
    if (m_output.format() != outputFormat())
        throwBoxException("CxxTransformBox: port 'output' has an invalid format");
}

bool CxxTransformBox::propagatePortFormat() {
    return false;
}

RuntimeBox *CxxTransformBox::newRuntimeBox() const {
    return new CxxTransformRuntimeBox(this);
}

QString CxxTransformBox::src() const {
    return m_src;
}

#define JSX_BEGIN try {

#define JSX_END } catch(const std::exception& e) { \
    context()->throwError(QString::fromUtf8(e.what())); \
    }

CxxTransformBox& CxxTransformBox::setSrc(const QString& src)
{
    JSX_BEGIN

    // Find/build & check & Load library
    m_libProxy = TransformLibProxy(
        CxxBuildHelper(this, src, "transform", ":/cxx/TransformFileFooter.cpp", m_useQmake).library(),
        this);

    // Assign source property
    m_src = src;

    // Update port formats
    m_param.format() = paramFormat();
    m_param.hints().setEntryHints(paramNames());
    m_input.format() = inputFormat();
    m_input.hints().setEntryHints(inputNames());
    m_output.format() = outputFormat();
    m_output.hints().setEntryHints(outputNames());

    return *this;

    JSX_END
    return *this;
}

QString CxxTransformBox::buildDir(const QScriptValue &boxProps) const {
    return CxxBuildHelper(this, boxProps.property("src").toString(), "ode", ":/cxx/OdeFileFooter.cpp", m_useQmake).buildDirName();
}

QString CxxTransformBox::srcExample() const {
    return readFile(":/cxx/TransformClass.cpp", this);
}

bool CxxTransformBox::useQmake() const {
    return m_useQmake;
}

CxxTransformBox& CxxTransformBox::setUseQmake(bool useQmake) {
    m_useQmake = useQmake;
    return *this;
}

const CxxTransformBox::TransformLibProxy *CxxTransformBox::transformLibProxy() const {
    return &m_libProxy;
}

#define TMP_DELEGATE_LIBPROXY_METHOD(ReturnType, method) \
    ReturnType CxxTransformBox::method() const { \
        return callLibProxyMethod(this, std::mem_fun(&TransformLibProxy::method)); \
    }

TMP_DELEGATE_LIBPROXY_METHOD(PortFormat, paramFormat)
TMP_DELEGATE_LIBPROXY_METHOD(QStringList, paramNames)
TMP_DELEGATE_LIBPROXY_METHOD(PortFormat, inputFormat)
TMP_DELEGATE_LIBPROXY_METHOD(QStringList, inputNames)
TMP_DELEGATE_LIBPROXY_METHOD(PortFormat, outputFormat)
TMP_DELEGATE_LIBPROXY_METHOD(QStringList, outputNames)

#undef TMP_DELEGATE_LIBPROXY_METHOD



CxxTransformBox::TransformLibProxy::TransformLibProxy(const QLibraryPtr& lib, const Box *box) :
    m_lib(lib),
    m_box(box)
{
    init();
}

CxxTransformBox::TransformLibProxy::TransformLibProxy(const TransformLibProxy& arg) :
    m_lib(arg.m_lib),
    m_box(arg.m_box)
{
    init();
}

CxxTransformBox::TransformLibProxy& CxxTransformBox::TransformLibProxy::operator=(const TransformLibProxy& arg)
{
    if (&arg != this) {
        m_box = arg.m_box;
        m_lib = arg.m_lib;
        init();
    }
    return *this;
}

void CxxTransformBox::TransformLibProxy::init()
{
    Q_ASSERT(m_lib->isLoaded());
#define RESOLVE_SYMBOL(name) \
    m_##name = reinterpret_cast<name##Func>(m_lib->resolve(#name)); \
    if (!m_##name) \
        throw EquaresBoxException(m_box, QString("Failed to resolve symbol '%1' in library '%2'").arg(#name, m_lib->fileName()));
    RESOLVE_SYMBOL(newInstance)
    RESOLVE_SYMBOL(deleteInstance)
    RESOLVE_SYMBOL(paramDimension)
    RESOLVE_SYMBOL(paramSizes)
    RESOLVE_SYMBOL(paramNames)
    RESOLVE_SYMBOL(inputDimension)
    RESOLVE_SYMBOL(inputSizes)
    RESOLVE_SYMBOL(inputNames)
    RESOLVE_SYMBOL(outputDimension)
    RESOLVE_SYMBOL(outputSizes)
    RESOLVE_SYMBOL(outputNames)
    RESOLVE_SYMBOL(prepare)
    RESOLVE_SYMBOL(transform)
    RESOLVE_SYMBOL(hash)
#undef RESOLVE_SYMBOL
    m_inst = m_newInstance();
}

CxxTransformBox::TransformLibProxy::~TransformLibProxy() {
    if (!m_lib.isNull())
        m_deleteInstance(m_inst);
}



CxxTransformRuntimeBox::CxxTransformRuntimeBox(const CxxTransformBox *box) :
    m_libProxy(box->transformLibProxy())
{
    if (!m_libProxy)
        throwBoxException("CxxTransformRuntimeBox: No transformation source code is currently set");

    setOwner(box);

    InputPorts in = box->inputPorts();
    m_param.init(this, in[0], toPortNotifier(&CxxTransformRuntimeBox::setParameters));
    m_input.init(this, in[1], toPortNotifier(&CxxTransformRuntimeBox::setInput));
    setInputPorts(RuntimeInputPorts() << &m_param << &m_input);

    OutputPorts out = box->outputPorts();
    m_outputData.resize(out[0]->format().dataSize());
    m_output.init(this, out[0], PortData(2, m_outputData.data()));
    setOutputPorts(RuntimeOutputPorts() << &m_output);

    m_hasParamData = in[0]->format().dataSize() == 0;
    m_paramData.resize(in[0]->format().dataSize());
}

bool CxxTransformRuntimeBox::setParameters(int)
{
    RuntimeOutputPort *paramPort = m_param.outputPort();
    Q_ASSERT(paramPort->state().hasData());
    paramPort->data().copyTo(m_paramData.data());
    m_hasParamData = true;
    m_libProxy->prepare(m_paramData.data());
    return true;
}

bool CxxTransformRuntimeBox::setInput(int)
{
    if (!m_hasParamData)
        return false;
    RuntimeOutputPort *outputPort = m_input.outputPort();
    Q_ASSERT(outputPort->state().hasData());
    m_libProxy->transform(m_output.data().data(), m_paramData.data(), outputPort->data().data());
    m_output.state().setValid();
    return m_output.activateLinks();
}
