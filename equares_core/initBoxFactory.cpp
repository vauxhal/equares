#include "initBoxFactory.h"
#include "BitmapBox.h"
#include "CanvasBox.h"
#include "ConstantSourceBox.h"
#include "CountedFilterBox.h"
#include "CrossSectionBox.h"
#include "DoublePendulumBox.h"
#include "DumpBox.h"
#include "IntervalFilterBox.h"
#include "MathieuBox.h"
#include "OdeJsBox.h"
#include "PendulumBox.h"
#include "ProjectionBox.h"
#include "Rk4AdjustParamBox.h"
#include "Rk4Box.h"
#include "ValveBox.h"
#include "VibratingPendulumBox.h"

#ifdef EQUARES_CORE_STATIC

static QScriptValue canvasDimParamToScriptValue(QScriptEngine *e, const CanvasBoxDimParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("vmin", p.vmin);
    result.setProperty("vmax", p.vmax);
    result.setProperty("resolution", p.resolution);
    return result;
}

static void canvasDimParamFromScriptValue(const QScriptValue& v, CanvasBoxDimParam& result) {
    result = CanvasBoxDimParam();
    result.vmin = v.property("vmin").toNumber();
    result.vmax = v.property("vmax").toNumber();
    result.resolution = v.property("resolution").toInt32();
}

static QScriptValue canvasParamToScriptValue(QScriptEngine *e, const CanvasBoxParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("x", canvasDimParamToScriptValue(e, p[0]));
    result.setProperty("y", canvasDimParamToScriptValue(e, p[1]));
    return result;
}

static void canvasParamFromScriptValue(const QScriptValue& v, CanvasBoxParam& result) {
    result = CanvasBoxParam();
    canvasDimParamFromScriptValue(v.property("x"), result[0]);
    canvasDimParamFromScriptValue(v.property("y"), result[1]);
}

static void canvasParamScriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, canvasDimParamToScriptValue, canvasDimParamFromScriptValue);
    qScriptRegisterMetaType(e, canvasParamToScriptValue, canvasParamFromScriptValue);
}



static QScriptValue xsParamToScriptValue(QScriptEngine *e, const CrossSectionBoxParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("index", p.index);
    result.setProperty("pos", p.pos);
    QScriptValue flags = e->newArray();
    int i = 0;
    if (p.flags & CountPositiveSpeed)
        flags.setProperty(i++, "positive");
    if (p.flags & CountNegativeSpeed)
        flags.setProperty(i++, "negative");
    result.setProperty("flags", flags);
    return result;
}

static void xsParamFromScriptValue(const QScriptValue& v, CrossSectionBoxParam& result) {
    result = CrossSectionBoxParam();
    result.index = v.property("index").toInt32();
    result.pos = v.property("pos").toNumber();
    result.flags = 0;
    QScriptValue flags = v.property("flags");
    if (!flags.isArray())
        return;
    int length = flags.property("length").toInt32();
    for (int i=0; i<length; ++i) {
        QString flag = flags.property(i).toString();
        if (flag == "positive")
            result.flags |= CountPositiveSpeed;
        else if (flag == "negative")
            result.flags |= CountNegativeSpeed;
    }
}

static void xsParamScriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, xsParamToScriptValue, xsParamFromScriptValue);
}



static QScriptValue ifParamToScriptValue(QScriptEngine *e, const IntervalFilterBoxParam& p) {
    QScriptValue result = e->newObject();
    result.setProperty("index", p.index);
    result.setProperty("offset", p.offset);
    result.setProperty("interval", p.interval);
    return result;
}

static void ifParamFromScriptValue(const QScriptValue& v, IntervalFilterBoxParam& result) {
    result = IntervalFilterBoxParam();
    result.index = v.property("index").toInt32();
    result.offset = v.property("offset").toNumber();
    result.interval = v.property("interval").toNumber();
}

static void ifParamScriptInit(QScriptEngine *e)
{
    qScriptRegisterMetaType(e, ifParamToScriptValue, ifParamFromScriptValue);
}



void initBoxFactory()
{
    REGISTER_BOX_STATIC(BitmapBox, "Bitmap")
    REGISTER_BOX_STATIC(CanvasBox, "Canvas")
    REGISTER_BOX_STATIC(ConstantSourceBox, "Param")
    REGISTER_BOX_STATIC(CountedFilterBox, "CountedFilter")
    REGISTER_BOX_STATIC(CrossSectionBox, "CrossSection")
    REGISTER_BOX_STATIC(DoublePendulumBox, "DoublePendulum")
    REGISTER_BOX_STATIC(DumpBox, "Dump")
    REGISTER_BOX_STATIC(IntervalFilterBox, "IntervalFilter")
    REGISTER_BOX_STATIC(MathieuBox, "Mathieu")
    REGISTER_BOX_STATIC(OdeJsBox, "JsOde")
    REGISTER_BOX_STATIC(PendulumBox, "Pendulum")
    REGISTER_BOX_STATIC(ProjectionBox, "Projection")
    REGISTER_BOX_STATIC(Rk4AdjustParamBox, "Rk4ParamAdjust")
    REGISTER_BOX_STATIC(Rk4Box, "Rk4")
    REGISTER_BOX_STATIC(ValveBox, "Valve")
    REGISTER_BOX_STATIC(VibratingPendulumBox, "VibratingPendulum")
    REGISTER_SCRIPT_INIT_FUNC_STATIC(canvasParamScriptInit)
    REGISTER_SCRIPT_INIT_FUNC_STATIC(xsParamScriptInit)
    REGISTER_SCRIPT_INIT_FUNC_STATIC(ifParamScriptInit)
}

#endif // EQUARES_CORE_STATIC
