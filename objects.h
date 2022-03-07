#ifndef OBJECTS_H
#define OBJECTS_H

#include <math.h>
#include "add_math.h"
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QColor>
#include <QPoint>



class Collider;

class Point
{
protected:
    double x;
    double y;
public:
    Point();    //Uninitialized
    Point(double _x, double _y);
    virtual ~Point() = default;

    bool operator== (const Point &b) const;    //Check for equality
    bool operator!= (const Point &b) const;

    virtual void MoveTo(double _x, double _y);
    virtual void Drag(double dx, double dy);
    virtual void Turn(Angle angle, Point& pivot);
    virtual void Turn(Angle angle); //Doesn't make sense for point, but in derived classes

    double GetX();
    double GetY();
};

double distance2(Point a, Point b);
double distance(Point a, Point b);
Point *intersect2d(double a1, double b1, double c1, double a2, double b2, double c2); //Point where lines intersect (or nullptr if there isn't one or several. Additional check is to compare c1 and c2)


class Line
{
private:
    Point* min_p;  //Left point (min x)
    Point* max_p;  //Right point (max x)
public:
    Line(Point p1, Point p2);
    Line(const Line& other);
    Line(Line&& other) = delete;
    Line();
    ~Line();
    void Set(double x1, double y1, double x2, double y2);
    void Update();  //Update line equation
    void Turn(Angle angle, Point& pivot);
    double a, b, c;
    double GetMinX() const;
    double GetMaxX() const;
    double GetMinY() const;
    double GetMaxY() const;
    operator=(const Line& other);
};

class Circle: public Point
{
protected:
    double r;
public:
    Circle(double _x, double _y, double _r);
    ~Circle();

    double GetR();
};

//=== Entities ===

class Entity: public Point   //Base entity class
{
protected:

public:
    Entity(float _x, float _y, Collider* _collider = NULL);
    virtual ~Entity();
    Collider* collision_mask;
    bool ToDelete;  //True, if this entity has to be deleted on this frame

    virtual void Show() = 0; //Draw entity

    //Functions, that calls every tact
    void EntityUpdate();        //Function, that is called every tact. This function MUST BE NOT CHANGED.
    virtual void AutoMove();    //Movement, depending of current speed and angle (isn't implemented in this class, cause static entity should not move)
    virtual void OnStep();      //Action, that is happening every tact.

    void Delete();

    //Movement
    void MoveTo(double _x, double _y);
    void Drag(double dx, double dy);

    //Getting info
    virtual QString GetName();  //Get name of an object
    virtual QString GetInfo();  //Get info of an object
    virtual void ShowOutline(); //Highlight outline of an object, if there is one - else, draw circles
};

class MovingEntity: public Entity  //Base moving entity class
{
protected:
    double speed;   //Speed of entity
    double max_speed;
    Angle angle;    //Angle of entity
public:
    MovingEntity(double _x, double _y, double _speed = 0, Angle _angle = 0, Collider* _collider = nullptr);

    //Functions, that calls every tact
    void AutoMove();    //Movement, depending of current speed and angle

    //Changing variables
    void Turn(Angle d_angle);    //Change angle by d_angle
    void SetSpeed(double _speed);
    void SetAngle(Angle _angle);    //Setting angle to _angle

    //Getting info
    virtual QString GetName();
    virtual QString GetInfo();

    //Getting variables
    double GetSpeed();
    Angle GetAngle();
};

//=== Objects ===

class Wall: public Entity   //Vertical wall
{
public:
    Wall(double _x, double _y);

    void Show();

    //Getting info
    QString GetName();
};

class Box: public Entity    //Square box
{
public:
    Box(double _x, double _y);
    double a;   //side of box
    void Show();

    //Getting info
    QString GetName();
};

class WallChain: public Entity  //Two walls connected
{
public:
    WallChain(double _x, double _y);
    void Show();

    QString GetName();
};

#endif // OBJECTS_H
