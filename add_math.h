#ifndef ADD_MATH_H
#define ADD_MATH_H

#define EPSILON 0.001

#include <math.h>
#include "collision.h"
//Р’СЃРїРѕРјРѕРіР°С‚РµР»СЊРЅС‹Рµ РјР°С‚РµРјР°С‚РёС‡РµСЃРєРёРµ С„СѓРЅРєС†РёРё

//=== РўР РР“РћРќРћРњР•РўР РРЇ ===
double degtorad(double angle);  //РџРµСЂРµРІРѕРґ РёР· РіСЂР°РґСѓСЃРѕРІ РІ СЂР°РґРёР°РЅС‹
double radtodeg(double angle);  //РџРµСЂРµРІРѕРґ РёР· СЂР°РґРёР°РЅ РІ РіСЂР°РґСѓСЃС‹

double direction_to_point(double from_x, double from_y, double to_x, double to_y);  //РЈРіРѕР» РјРµР¶РґСѓ РґРІСѓРјСЏ С‚РѕС‡РєР°РјРё
double anglediff(double a, double b);    //Р Р°Р·РЅРѕСЃС‚СЊ РґРІСѓС… СѓРіР»РѕРІ РІ С„РѕСЂРјР°С‚Рµ РѕС‚ - 180 РґРѕ 180

double safe_acos(double ang);   //Р‘РµР·РѕРїР°СЃРЅР°СЏ С„СѓРЅРєС†РёСЏ Р°СЂРєРєРѕСЃРёРЅСѓСЃР°
double safe_asin(double ang);


//=== РџРµСЂРµСЃРµС‡РµРЅРёСЏ ===
Point* circle_tangent(double cx, double cy, double cr, double a, double b, double c);    //РџРѕРёСЃРє РїРµСЂРµСЃРµС‡РµРЅРёСЏ РїСЂСЏРјРѕР№ Рё РѕРєСЂСѓР¶РЅРѕСЃС‚Рё
double solve_square1(double a, double b, double c); //Р РµС€РµРЅРёРµ РєРІР°РґСЂР°С‚РЅРѕРіРѕ СѓСЂР°РІРЅРµРЅРёСЏ

//=== РџСЂРѕС‡РёРµ РјР°С‚РµРјР°С‚РёС‡РµСЃРєРёРµ С„СѓРЅРєС†РёРё ===
double distance2(double ax, double ay, double bx, double by);  //Р”РёСЃС‚Р°РЅС†РёСЏ РјРµР¶РґСѓ С‚РѕС‡РєР°РјРё РІ РєРІР°РґСЂР°С‚Рµ
double distance2(Point a, Point b); //Р”РёСЃС‚Р°РЅС†РёСЏ РјРµР¶РґСѓ С‚РѕС‡РєР°РјРё РІ РєРІР°РґСЂР°С‚Рµ

double distance(double ax, double ay, double bx, double by);  //Р”РёСЃС‚Р°РЅС†РёСЏ РјРµР¶РґСѓ С‚РѕС‡РєР°РјРё
double distance(Point a, Point b); //Р”РёСЃС‚Р°РЅС†РёСЏ РјРµР¶РґСѓ С‚РѕС‡РєР°РјРё

double sign(double a);  //Р—РЅР°Рє С‡РёСЃР»Р°
bool almostEq(double a, double b, double eps = EPSILON);  //Р Р°РІРЅС‹ Р»Рё РґСЂСѓРі РґСЂСѓРіСѓ С‡РёСЃР»Р° a Рё b СЃ РїРѕРіСЂРµС€РЅРѕСЃС‚СЊСЋ EPSILON

void DrawTurnedRect(QPainter* painter, double x, double y, double angle, double width, double height);  //РћС‚СЂРёСЃРѕРІРєР° РїРѕРІРµСЂРЅСѓС‚РѕРіРѕ РїСЂСЏРјРѕСѓРіРѕР»СЊРЅРёРєР°

class Angle //РљР»Р°СЃСЃ СѓРіР»Р°
{
private:
    double angle;   //РЈРіРѕР» РІ СЂР°РґРёР°РЅР°С… РѕС‚ 0 РґРѕ 2PI
public:

    Angle();            //РџСѓСЃС‚РѕР№ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ
    Angle(double a);    //РџСЂРѕСЃС‚РѕР№ РєРѕРЅСЃС‚СЂСѓРєС‚РѕСЂ

    void CorrectAngle();

    double GetR();  //РџРѕР»СѓС‡РёС‚СЊ СѓРіРѕР» РІ СЂР°РґРёР°РЅР°С…
    double GetD();  //РџРѕР»СѓС‡РёС‚СЊ СѓРіРѕР» РІ РіСЂР°РґСѓСЃР°С… (РѕС‚ 0 РґРѕ 360)

    Angle normalL();   //Р›РµРІС‹Р№ РїРµСЂРїРµРЅРґРёРєСѓР»СЏСЂ
    Angle normalR();   //РџСЂР°РІС‹Р№ РїРµСЂРїРµРЅРґРёРєСѓР»СЏСЂ

    //РџРµСЂРµРіСЂСѓР·РєР° РѕРїРµСЂР°С‚РѕСЂРѕРІ
        //РЎР»РѕР¶РµРЅРёРµ
    Angle& operator+=(Angle& a);
    Angle& operator+=(double a);
        //Р’С‹С‡РёС‚Р°РЅРёРµ
    Angle& operator-=(Angle& a);
    Angle& operator-=(double a);
        //РџСЂРёСЃРІР°РёРІР°РЅРёРµ
    Angle& operator=(Angle& right);
    Angle& operator=(double right);
};

////РЎР»РѕР¶РµРЅРёРµ Рё РІС‹С‡РёС‚Р°РЅРёРµ
Angle operator+(Angle& left, Angle& right);
Angle operator-(Angle& left, Angle& right);

////РЎСЂР°РІРЅРµРЅРёРµ
bool operator==(Angle& left, Angle& right);
bool operator!=(Angle& left, Angle& right);
bool operator<(Angle& left, Angle& right);
bool operator>(Angle& left, Angle& right);




#endif // ADD_MATH_H
