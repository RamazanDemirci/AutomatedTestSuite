#ifndef UTILS_H
#define UTILS_H

#include <QJsonArray>
#include <QVariant>

typedef struct{
    QString name;
    QString type;
    QString format;
    QString generate;
    QVariant value;
    double factor;
    int min;
    int max;
    QJsonArray range;
    QString display;
    bool status;
}tsParam;

template<typename TYPE>
bool extractData(const char* input, QByteArray format, TYPE& T)
{
    int count = sscanf(input, format, &T);

    return (count == 1);
}

static class Utils
{
public:
    static bool     almostEqual(double val1, double val2);
    static bool     isInRange(double val, double min, double max);
    static bool     isInRange(double val, int min, int max);
    static bool     isInRange(int val, int min, int max);
    static bool     isInRange(QString type, QVariant val, QVariant min, QVariant max);
    static QString  getItem(tsParam param);
    static bool     compare(tsParam param_1st, tsParam param_2nd);
    static bool     validate(tsParam param);
};

#endif // UTILS_H
