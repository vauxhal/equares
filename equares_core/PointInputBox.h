#ifndef POINTINPUTBOX_H
#define POINTINPUTBOX_H

#include "DataInputBox.h"

struct PointInputBoxDimTransform
{
    int index;
    double vmin;
    double vmax;
    int resolution;
    PointInputBoxDimTransform() : index(0), vmin(0), vmax(1), resolution(100) {}
    PointInputBoxDimTransform(int index, double vmin, double vmax, int resolution) :
        index(index), vmin(vmin), vmax(vmax), resolution(resolution) {}
    double transform(int i) const {
        Q_ASSERT(resolution > 0);
        double t = static_cast<double>(i) / resolution;
        return vmin + t*(vmax - vmin);
    }
    int transformBack(double x) const {
        Q_ASSERT(resolution > 0);
        double t = (x - vmin) / (vmax - vmin);
        return static_cast<int>(t*resolution);
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

class EQUARES_CORESHARED_EXPORT PointInputBox : public DataInputBox
{
    Q_OBJECT
    Q_PROPERTY(PointInputBoxTransform transform READ transform WRITE setTransform)
    Q_PROPERTY(QString refBitmap READ refBitmap WRITE setRefBitmap)
public:
    explicit PointInputBox(QObject *parent = 0);

    RuntimeBox *newRuntimeBox() const;
    void checkPortFormat() const;

    typedef PointInputBoxTransform Transform;
    typedef PointInputBoxDimTransform DimTransform;

    Transform transform() const;
    PointInputBox& setTransform(const Transform& param);
    QString refBitmap() const;
    PointInputBox& setRefBitmap(const QString& refBitmap);

private:
    Transform m_transform;
    QString m_refBitmap;
};

class EQUARES_CORESHARED_EXPORT PointInputRuntimeBox : public DataInputRuntimeBox
{
public:
    explicit PointInputRuntimeBox(const PointInputBox *box);
    InputInfoList inputInfo() const;

protected:
    void transformData(double *portData, const double *inputData) const;
    QVector<double> inputData(const double *portData) const;

private:

    typedef PointInputBoxTransform Transform;
    Transform m_transform;
    QString m_refBitmap;
};

#endif // POINTINPUTBOX_H
