#ifndef POINTINPUTBOX_H
#define POINTINPUTBOX_H

#include "equares_core.h"
#include "equares_script.h"

struct PointInputBoxDimParam
{
    int index;
    double vmin;
    double vmax;
    int resolution;
    PointInputBoxDimParam() : vmin(0), vmax(1), resolution(100) {}
    PointInputBoxDimParam(double vmin, double vmax, int resolution) :
        vmin(vmin), vmax(vmax), resolution(resolution) {}
    int interp(int i) const {
        Q_ASSERT(resolution > 0);
        double t = static_cast<double>(i) / resolution;
        return vmin + t*(vmax - vmin);
    }
};

Q_DECLARE_METATYPE(PointInputBoxDimParam)

struct PointInputBoxParam
{
public:
    PointInputBoxParam() {}
    PointInputBoxParam(const PointInputBoxDimParam& d1, const PointInputBoxDimParam& d2) {
        m_dimParam[0] = d1;
        m_dimParam[1] = d2;
    }

    PointInputBoxDimParam& operator[](int i) {
        Q_ASSERT(i==0 || i==1);
        return m_dimParam[i];
    }
    const PointInputBoxDimParam& operator[](int i) const {
        return (*const_cast<PointInputBoxParam*>(this))[i];
    }

private:
    PointInputBoxDimParam m_dimParam[2];
};

Q_DECLARE_METATYPE(PointInputBoxParam)

class EQUARES_CORESHARED_EXPORT PointInputBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(PointInputBoxParam param READ param WRITE setParam)
    Q_PROPERTY(bool sync READ sync WRITE setSync)
    Q_PROPERTY(QString refBitmap READ refBitmap WRITE setRefBitmap)
public:
    explicit PointInputBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    typedef PointInputBoxParam Param;
    typedef PointInputBoxDimParam DimParam;

    Param param() const;
    PointInputBox& setParam(const Param& param);
    bool sync() const;
    PointInputBox& setSync(bool sync);
    QString refBitmap() const;
    PointInputBox& setRefBitmap(const QString& refBitmap);

private:
    Param m_param;
    bool m_sync;
    QString m_refBitmap;
    mutable InputPort m_activator;
    mutable InputPort m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT PointInputRuntimeBox : public RuntimeBox
{
public:
    explicit PointInputRuntimeBox(const PointInputBox *box);

private:
    RuntimeInputPort m_activator;
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;

    PointInputBox::Param m_param;
    QString m_refBitmap;
    QVector<double> m_data;

    QVector<int> m_midx;

    typedef PointInputBoxParam Param;
    bool activate();
    bool processInput();
};

#endif // POINTINPUTBOX_H
