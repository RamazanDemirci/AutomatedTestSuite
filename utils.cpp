#include "utils.h"
#include <QVariant>
#include <QtMath>
#include <QDebug>

bool Utils::almostEqual(double val1, double val2)
{
    return (std::fabs(val1 - val2) < std::fabs(std::min(val1, val2)) * std::numeric_limits<double>::epsilon());
}

bool Utils::isInRange(double val, double min, double max)
{
    return (val > min) || almostEqual(val, min)
        && (val < max) || almostEqual(val, max);
}

bool Utils::isInRange(double val, int min, int max)
{
    return (val > min) || almostEqual(val, min)
        && (val < max) || almostEqual(val, max);
}

bool Utils::isInRange(int val, int min, int max)
{
    return (val >= min) && (val <= max);
}

bool Utils::isInRange(QString type, QVariant val, QVariant min, QVariant max)
{
    if(type == "int")
        return isInRange(val.toInt(), min.toInt(), max.toInt());
    else if(type == "double")
        return isInRange(val.toDouble(), min.toDouble(), max.toDouble());
    return false;
}

QString Utils::getItem(tsParam param)
{
    if(param.value.type() == QVariant::Type::Int)
    {
        if(param.value < param.range.count())
        {
            int index = param.value.toInt();
            QVariant item = param.range.at(index).toVariant();

            if(item.type() == QVariant::Type::Int)
            {
                return QString::number(item.toInt());
                //return item.toByteArray();
            }
            else if(item.type() == QVariant::Type::Double)
            {
                return QString::number(item.toDouble());
                //return item.toByteArray();
            }
            else if(item.type() == QVariant::Type::String)
            {
                return item.toString().toLocal8Bit();
            }
            else
            {
                return item.toByteArray();
            }
        }
        else
        {
            return param.value.toByteArray();
        }
    }
    else
    {
        return param.value.toByteArray();
    }
}

bool Utils::validate(tsParam param)
{
    bool bValid;
    if(param.type == "int"){
        bValid = Utils::isInRange(param.value.toInt(), param.min, param.max);
    }
    else if(param.type == "char"){
        bValid = param.range.contains(QJsonValue(param.value.toString()));
    }
    else if(param.type == "double"){
        bValid = Utils::isInRange(param.value.toDouble(), param.min, param.max);
    }
    else if(param.type == "string"){
        bValid = param.range.contains(QJsonValue(param.value.toString()));
    }
    return bValid;
}

bool Utils::compare(tsParam param_1st, tsParam param_2nd)
{
    bool bValid = false;

    if(param_1st.type == "int")
    {
        if(param_2nd.type == "int")
        {
            int s_tmp = param_1st.value.toInt() * param_1st.factor;
            int r_tmp = param_2nd.value.toInt() * param_2nd.factor;
            bValid = (s_tmp == r_tmp) ? true : false;
        }
        else if(param_2nd.type == "double")
        {
            double s_tmp = param_1st.value.toDouble() * param_2nd.factor;
            double r_tmp = param_2nd.value.toDouble();

            if(r_tmp != 0)
                bValid = ( (fabs(s_tmp - r_tmp) / r_tmp) < param_2nd.factor) ? true : false;
        }
        else if(param_2nd.type == "string")
        {
            if(param_2nd.generate == "range")
            {
                int rangeIndex = param_1st.value.toInt();
                QString s_tmp = param_2nd.range.at(rangeIndex).toString();
                QString r_tmp = param_2nd.value.toString();
                bValid = (s_tmp == r_tmp) ? true : false;
            }
            else
            {
                bValid = false;
                qDebug() << "incompatible Type, param1st:int, param2nd:string; but genarate is not range";
            }
        }
    }
    else if(param_1st.type == "char")
    {
        QChar s_tmp = param_1st.value.toChar();
        if(param_2nd.type == "char")
        {
            QChar r_tmp = param_2nd.value.toChar();
            bValid = (s_tmp == r_tmp) ? true : false;
        }
    }
    else if(param_1st.type == "double")
    {
        if(param_2nd.type == "double")
        {
            double s_tmp = param_1st.value.toDouble();
            double r_tmp = param_2nd.value.toDouble();
            if(r_tmp != 0)
                bValid = ( (fabs(s_tmp - r_tmp) / r_tmp) < param_2nd.factor) ? true : false;
        }
        else if(param_2nd.type == "int")
        {
            double s_tmp = param_1st.value.toDouble();
            double r_tmp = param_2nd.value.toDouble() * param_2nd.factor;

            if(r_tmp != 0)
                bValid = ( (fabs(s_tmp - r_tmp) / r_tmp) < param_2nd.factor) ? true : false;
        }
    }
    else if(param_1st.type == "string")
    {
        if(param_2nd.type == "string")
        {
            QString s_tmp = param_1st.value.toString();
            QString r_tmp = param_2nd.value.toString();
            bValid = (s_tmp == r_tmp) ? true : false;
        }
    }
    return bValid;
}
