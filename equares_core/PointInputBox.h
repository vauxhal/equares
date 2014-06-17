#ifndef POINTINPUTBOX_H
#define POINTINPUTBOX_H

#include "equares_core.h"
#include "equares_script.h"

struct PointInputBoxDimTransform
{
    int index;
    double vmin;
    double vmax;
    int resolution;
    PointInputBoxDimTransform() : index(0), vmin(0), vmax(1), resolution(100) {}
    PointInputBoxDimTransform(int index, double vmin, double vmax, int resolution) :
        index(index), vmin(vmin), vmax(vmax), resolution(resolution) {}
    int transform(int i) const {
        Q_ASSERT(resolution > 0);
        double t = static_cast<double>(i) / resolution;
        return vmin + t*(vmax - vmin);
    }
};

Q_DECLARE_METATYPE(PointInputBoxDimTransform)

struct PointInputBoxTransform
{
public:
    PointInputBoxTransform() {
        m_dimTransform[1].index = 1;
    }
    PointInputBoxTransform(const PointInputBoxDimTransform& d1, const PointInputBoxDimTransform& d2) {
        m_dimTransform[0] = d1;
        m_dimTransform[1] = d2;
    }

    PointInputBoxDimTransform& operator[](int i) {
        Q_ASSERT(i==0 || i==1);
        return m_dimTransform[i];
    }
    const PointInputBoxDimTransform& operator[](int i) const {
        return (*const_cast<PointInputBoxTransform*>(this))[i];
    }

private:
    PointInputBoxDimTransform m_dimTransform[2];
};

Q_DECLARE_METATYPE(PointInputBoxTransform)

class EQUARES_CORESHARED_EXPORT PointInputBox : public Box
{
    Q_OBJECT
    Q_PROPERTY(PointInputBoxTransform transform READ transform WRITE setTransform)
    Q_PROPERTY(bool sync READ sync WRITE setSync)
    Q_PROPERTY(bool loop READ loop WRITE setLoop)
    Q_PROPERTY(QString refBitmap READ refBitmap WRITE setRefBitmap)
public:
    explicit PointInputBox(QObject *parent = 0);

    InputPorts inputPorts() const;
    OutputPorts outputPorts() const;
    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;
    bool propagatePortFormat();

    typedef PointInputBoxTransform Transform;
    typedef PointInputBoxDimTransform DimTransform;

    Transform transform() const;
    PointInputBox& setTransform(const Transform& param);
    bool sync() const;
    PointInputBox& setSync(bool sync);
    bool loop() const;
    PointInputBox& setLoop(bool loop);
    QString refBitmap() const;
    PointInputBox& setRefBitmap(const QString& refBitmap);

private:
    Transform m_transform;
    bool m_sync;
    bool m_loop;
    QString m_refBitmap;
    mutable InputPort m_activator;
    mutable InputPort m_in;
    mutable OutputPort m_out;
};

class EQUARES_CORESHARED_EXPORT PointInputRuntimeBox : public RuntimeBox
{
public:
    explicit PointInputRuntimeBox(const PointInputBox *box);
    void registerInput();

private:
    RuntimeInputPort m_activator;
    RuntimeInputPort m_in;
    RuntimeOutputPort m_out;

    PointInputBox::Transform m_transform;
    bool m_sync;
    bool m_loop;
    QString m_refBitmap;
    QVector<double> m_data;
    bool m_dataValid;
    bool fetchInputPortData();
    int m_inputId;

    typedef PointInputBoxTransform Transform;
    bool activate();
    bool processInput();
};

#endif // POINTINPUTBOX_H
