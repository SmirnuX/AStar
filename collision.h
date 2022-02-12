#ifndef COLLISION_H
#define COLLISION_H
#include <QPainter>
#include "add_math.h"
#include "objects.h"

class PointCollider;
class LineCollider;
class CircleCollider;
class PolygonCollider;

class Collider: public Point  //Collision base class
{
public:
    Angle angle;
    int collisions; //Count of collisions

    Collider(double o_x, double o_y);   //Collider with origin point in (o_x, o_y)
    virtual ~Collider();

    virtual bool CheckCollision(Collider* other) = 0;           //Collision with unknown object
    virtual bool CheckCollision(PointCollider* other) = 0;      //Collision with point
    virtual bool CheckCollision(LineCollider* other) = 0;       //Collision with line
    virtual bool CheckCollision(CircleCollider* other) = 0;     //Collision with circle
    virtual bool CheckCollision(PolygonCollider* other) = 0;    //Collision with polygon
    virtual void ShowCollider() = 0;                            //Drawing collision mask

    virtual void SetAngle(Angle _angle);

    //virtual void UpdateBoundingBox(double x, double y) = 0;
    //Polygon* BoundingBox;
    //bool CheckSimpleCollision(Collider* other);   //Check collision of bounding boxes
};

class PointCollider : public Collider
{
public:
    PointCollider(double _x, double _y);
    ~PointCollider();

    bool CheckCollision(Collider* other);           //Collision with unknown object
    bool CheckCollision(PointCollider* other);      //Collision with point
    bool CheckCollision(LineCollider* other);       //Collision with line
    bool CheckCollision(CircleCollider* other);     //Collision with circle
    bool CheckCollision(PolygonCollider* other);    //Collision with polygon
    void ShowCollider();                            //Drawing collision mask
};

class LineCollider : public Collider
{
public:
    Line* line;

    LineCollider(double x1, double y1, double x2, double y2);
    ~LineCollider();

    bool CheckCollision(Collider* other);           //Collision with unknown object
    bool CheckCollision(PointCollider* other);      //Collision with point
    bool CheckCollision(LineCollider* other);       //Collision with line
    bool CheckCollision(CircleCollider* other);     //Collision with circle
    bool CheckCollision(PolygonCollider* other);    //Collision with polygon
    void ShowCollider();

    void MoveTo(double _x, double _y);    //Move first point to (_x, _y) - second point will follow
    void Drag(double dx, double dy);
    void Turn(Angle angle, Point& pivot);
    void Turn(Angle angle); //Rotate relative to left point
    void SetAngle(Angle angle);
};

class CircleCollider : public Collider
{
public:
    Circle* circle;

    CircleCollider(double _x, double _y, double _r);
    ~CircleCollider();

    bool CheckCollision(Collider* other);           //Collision with unknown object
    bool CheckCollision(PointCollider* other);      //Collision with point
    bool CheckCollision(LineCollider* other);       //Collision with line
    bool CheckCollision(CircleCollider* other);     //Collision with circle
    bool CheckCollision(PolygonCollider* other);    //Collision with polygon
    void ShowCollider();
};

class PolygonCollider : public Collider
{
public:
    Point** points;
    Point** orig_points;
    int count;

    PolygonCollider(double* x_s, double* y_s, int num, double orig_x, double orig_y);
    ~PolygonCollider();

    bool CheckCollision(Collider* other);           //Collision with unknown object
    bool CheckCollision(PointCollider* other);      //Collision with point
    bool CheckCollision(LineCollider* other);       //Collision with line
    bool CheckCollision(CircleCollider* other);     //Collision with circle
    bool CheckCollision(PolygonCollider* other);    //Collision with polygon
    void ShowCollider();

    void MoveTo(double _x, double _y);  //Move origin to _x, _y
    void Drag(double dx, double dy);
    void Turn(Angle angle, Point& pivot);
    void Turn(Angle angle);
    void SetAngle(Angle angle);
};




#endif // COLLISION_H
