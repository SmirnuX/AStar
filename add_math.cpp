#include "add_math.h"

double degtorad(double angle)  //РџРµСЂРµРІРѕРґ РёР· РіСЂР°РґСѓСЃРѕРІ РІ СЂР°РґРёР°РЅС‹
{
    return angle * M_PI / 180.0;
}

double radtodeg(double angle)  //РџРµСЂРµРІРѕРґ РёР· СЂР°РґРёР°РЅ РІ РіСЂР°РґСѓСЃС‹
{
    return angle * 180 / M_PI;
}

double direction_to_point(double from_x, double from_y, double to_x, double to_y)
{
    double dir = safe_acos( (to_x - from_x) / sqrt(distance2(from_x, from_y, to_x, to_y)));
    if (to_y < from_y)
        dir = 2* M_PI - dir;
    return dir;
}

double anglediff(double a, double b)    //Р Р°Р·РЅРѕСЃС‚СЊ РґРІСѓС… СѓРіР»РѕРІ РІ С„РѕСЂРјР°С‚Рµ РѕС‚ - 180 РґРѕ 180
{
    //РќРѕСЂРјР°Р»РёР·Р°С†РёСЏ
    if (a < 0)
        a += 360;
    if (a >= 360)
        a -= 360;
    if (b < 0)
        b += 360;
    if (b >= 360)
        b -= 360;
    double da = a - b;
    if (da < 0)
        da += 360;
    if (da >= 360)
        da -= 360;
    if (da > 180)
        da = da - 360;
    return da;
}

double distance2(double ax, double ay, double bx, double by)  //Р”РёСЃС‚Р°РЅС†РёСЏ РјРµР¶РґСѓ С‚РѕС‡РєР°РјРё РІ РєРІР°РґСЂР°С‚Рµ
{
    return (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
}

double distance2(Point a, Point b)  //Р”РёСЃС‚Р°РЅС†РёСЏ РјРµР¶РґСѓ С‚РѕС‡РєР°РјРё РІ РєРІР°РґСЂР°С‚Рµ
{

    return (b.GetX()-a.GetX())*(b.GetX()-a.GetX()) + (b.GetY()-a.GetY())*(b.GetY()-a.GetY());
}

double distance(double ax, double ay, double bx, double by)  //Р”РёСЃС‚Р°РЅС†РёСЏ РјРµР¶РґСѓ С‚РѕС‡РєР°РјРё
{
    return sqrt(distance2(ax, ay, bx, by));
}

double distance(Point a, Point b)  //Р”РёСЃС‚Р°РЅС†РёСЏ РјРµР¶РґСѓ С‚РѕС‡РєР°РјРё
{

    return sqrt(distance2(a, b));
}

double sign(double a)
{
    if (a >= 0)
        return 1;
    else
        return -1;
}

bool almostEq(double a, double b, double eps)  //Р Р°РІРЅС‹ Р»Рё РґСЂСѓРі РґСЂСѓРіСѓ С‡РёСЃР»Р° a Рё b СЃ РїРѕРіСЂРµС€РЅРѕСЃС‚СЊСЋ EPSILON
{
    if (a > b - eps && a < b + eps)
        return true;
    else
        return false;
}

void DrawTurnedRect(QPainter* painter, double x, double y, double angle, double width, double height)   //РћС‚СЂРёСЃРѕРІРєР° РїРѕРІРµСЂРЅСѓС‚РѕРіРѕ РїСЂСЏРјРѕСѓРіРѕР»СЊРЅРёРєР° СЃ С†РµРЅС‚СЂРѕРј РІ С‚РѕС‡РєРµ (x;y), РїРѕРІРѕСЂРѕС‚РѕРј angle Рё СЂР°Р·РјРµСЂР°РјРё width x height
{
    QPoint points[4];
    double diag_angle = atan(height/width);   //РЈРіРѕР» РјРµР¶РґСѓ РґР»РёРЅРЅРѕР№ СЃС‚РѕСЂРѕРЅРѕР№ Рё РґРёР°РіРѕРЅР°Р»СЊСЋ
    double diag = sqrt(width*width + height*height)/2;  //РџРѕР»РѕРІРёРЅР° РґР»РёРЅС‹ РґРёР°РіРѕРЅР°Р»Рё
    double sinus = sin(diag_angle + degtorad(angle));
    double cosinus = cos(diag_angle + degtorad(angle));
    double r_sinus = sin(M_PI - diag_angle + degtorad(angle));
    double r_cosinus = cos(M_PI - diag_angle + degtorad(angle));
    points[0] = QPoint(x + cosinus * diag, y - sinus * diag); //Р’РµСЂС…РЅСЏСЏ РїСЂР°РІР°СЏ
    points[1] = QPoint(x + r_cosinus * diag, y - r_sinus * diag); //Р’РµСЂС…РЅСЏСЏ Р»РµРІР°СЏ
    points[2] = QPoint(x - cosinus * diag, y + sinus * diag); //РќРёР¶РЅСЏСЏ Р»РµРІР°СЏ
    points[3] = QPoint(x - r_cosinus * diag, y + r_sinus * diag); //РќРёР¶РЅСЏСЏ РїСЂР°РІР°СЏ
    painter->drawConvexPolygon(points, 4);
}

//РљР»Р°СЃСЃ СѓРіР»Р°
Angle::Angle()            //Пустой конструктор
{

}

Angle::Angle(double a)    //РџСЂРѕСЃС‚РѕР№ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
{
    angle = a;
    CorrectAngle();
}

void Angle::CorrectAngle()   //Коррекция угла, чтобы он находился в промежутке между 0 и 2PI
{
    while (angle < 0 || angle >= 2*M_PI)
    {
        if (angle < 0)
            angle += 2*M_PI;
        else if (angle >= 2*M_PI)
            angle -= 2*M_PI;
    }
}

double Angle::GetR()  //РџРѕР»СѓС‡РёС‚СЊ СѓРіРѕР» РІ СЂР°РґРёР°РЅР°С…
{
    return angle;
}

double Angle::GetD()  //РџРѕР»СѓС‡РёС‚СЊ СѓРіРѕР» РІ РіСЂР°РґСѓСЃР°С… (РѕС‚ 0 РґРѕ 360)
{
    return radtodeg(angle);
}

Angle Angle::normalL()   //Левый перпендикуляр
{
    Angle a = Angle(GetR() + M_PI/4);
    return a;
}

Angle Angle::normalR()   //Правый перпендикуляр
{
    Angle a = Angle(GetR() - M_PI/4);
    return a;
}

//РџРµСЂРµРіСЂСѓР·РєР° РѕРїРµСЂР°С‚РѕСЂРѕРІ
    //Прибавление
Angle& Angle::operator+=(Angle& a)
{
    angle += a.GetR();
    CorrectAngle();
    return *this;
}

Angle& Angle::operator+=(double a)
{
    angle += a;
    CorrectAngle();
    return *this;
}

    //Убавление
Angle& Angle::operator-=(Angle& a)
{
    *this += -a.GetR();
    return *this;
}

Angle& Angle::operator-=(double a)
{
    *this += -a;
    return *this;
}

    //РџСЂРёСЃРІР°РёРІР°РЅРёРµ
Angle& Angle::operator=(Angle& right)
{
    angle = right.GetR();
    CorrectAngle();
    return *this;
}

Angle& Angle::operator=(double right)
{
    angle = right;
    CorrectAngle();
    return *this;
}

//РЎР»РѕР¶РµРЅРёРµ Рё РІС‹С‡РёС‚Р°РЅРёРµ
Angle operator+(Angle& left, Angle& right)
{
    Angle temp = left.GetR();
    temp += right;
    return temp;
}

Angle operator-(Angle& left, Angle& right)
{
    Angle temp = left.GetR();
    temp -= right;
    return temp;
}

//РЎСЂР°РІРЅРµРЅРёРµ
bool operator==(Angle& left, Angle& right)   //РџРѕРіСЂРµС€РЅРѕСЃС‚СЊ - 0.01
{
    return almostEq(left.GetR(), right.GetR());
}

bool operator!=(Angle& left, Angle& right)
{
    return !(left.GetR() == right.GetR());
}

bool operator<(Angle& left, Angle& right)    //Находится ли левый операнд правее правого?
{
    return ((left-right).GetR() > M_PI);
}

bool operator>(Angle& left, Angle& right)
{
    return ((left-right).GetR() < M_PI);
}


