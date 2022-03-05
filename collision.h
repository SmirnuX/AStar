#ifndef COLLISION_H
#define COLLISION_H

#include "objects.h"
#include "graph_struct.h"
#include <limits>

extern QPixmap* picture;

class PointCollider;
class LineCollider;
class ChainCollider;
class CircleCollider;
class PolygonCollider;

class Collider: public Point  //Collision base class
{
public:
    Angle angle;
    int collisions; //Count of collisions

    Collider(double o_x, double o_y);   //Collider with origin point in (o_x, o_y)
    virtual ~Collider();

    double left, right, top, bottom;  //X's of bounding box
    bool CheckBBCollision(Collider* other);  //Check if bounding boxed collides

    virtual void updateBB() = 0;    //Update bounding box
    virtual bool CheckCollision(Collider* other) = 0;           //Collision with unknown object
    virtual bool CheckCollision(PointCollider* other) = 0;      //Collision with point
    virtual bool CheckCollision(LineCollider* other) = 0;       //Collision with line
    virtual bool CheckCollision(ChainCollider* other) = 0;      //Collision with chain
    virtual bool CheckCollision(CircleCollider* other) = 0;     //Collision with circle
    virtual bool CheckCollision(PolygonCollider* other) = 0;    //Collision with polygon
    virtual void ShowCollider(QPainter *pntr = nullptr) = 0;    //Drawing collision mask

    virtual void SetAngle(Angle _angle);

    virtual obstacle GetOutline(double treshold) = 0;  //Get graph to ride round this object

};

class PointCollider : public Collider
{
public:
    PointCollider(double _x, double _y);
    ~PointCollider();

    void updateBB();    //Update bounding box

    bool CheckCollision(Collider* other);           //Collision with unknown object
    bool CheckCollision(PointCollider* other);      //Collision with point
    bool CheckCollision(LineCollider* other);       //Collision with line
    bool CheckCollision(ChainCollider* other);      //Collision with chain
    bool CheckCollision(CircleCollider* other);     //Collision with circle
    bool CheckCollision(PolygonCollider* other);    //Collision with polygon
    void ShowCollider(QPainter *pntr = nullptr);    //Drawing collision mask

    obstacle GetOutline(double treshold);  //Get graph to ride round this object
};

class LineCollider : public Collider
{
public:
    Line* line;

    LineCollider(double x1, double y1, double x2, double y2);   //First point is set as origin
    ~LineCollider();

    void updateBB();    //Update bounding box

    bool CheckCollision(Collider* other);           //Collision with unknown object
    bool CheckCollision(PointCollider* other);      //Collision with point
    bool CheckCollision(LineCollider* other);       //Collision with line
    bool CheckCollision(ChainCollider* other);      //Collision with chain
    bool CheckCollision(CircleCollider* other);     //Collision with circle
    bool CheckCollision(PolygonCollider* other);    //Collision with polygon
    void ShowCollider(QPainter *pntr = nullptr);

    void MoveTo(double _x, double _y);    //Move origin point to (_x, _y) - points will follow
    void Drag(double dx, double dy);
    void Turn(Angle angle, Point& pivot);
    void Turn(Angle angle); //Rotate relative to left point
    void SetAngle(Angle angle);

    obstacle GetOutline(double treshold);  //Get graph to ride round this object
};

class ChainCollider : public Collider
{
public:
    Point** points;
    Line** lines;
    Point** orig_points;
    int count;

    ChainCollider(double* x_s, double* y_s, int num, double orig_x, double orig_y);
    ~ChainCollider();

    void updateBB();    //Update bounding box
    void update_eq();   //Update equations

    bool CheckCollision(Collider* other);           //Collision with unknown object
    bool CheckCollision(PointCollider* other);      //Collision with point
    bool CheckCollision(LineCollider* other);       //Collision with line
    bool CheckCollision(ChainCollider* other);      //Collision with chain
    bool CheckCollision(CircleCollider* other);     //Collision with circle
    bool CheckCollision(PolygonCollider* other);    //Collision with polygon
    void ShowCollider(QPainter *pntr = nullptr);

    void MoveTo(double _x, double _y);    //Move origin point to (_x, _y) - points will follow
    void Drag(double dx, double dy);
    void Turn(Angle angle, Point& pivot);
    void Turn(Angle angle); //Rotate relative to left point
    void SetAngle(Angle angle);

    obstacle GetOutline(double treshold);  //Get graph to ride round this object
};

class CircleCollider : public Collider
{
public:
    Circle* circle;

    CircleCollider(double _x, double _y, double _r);
    ~CircleCollider();

    void updateBB();    //Update bounding box

    bool CheckCollision(Collider* other);           //Collision with unknown object
    bool CheckCollision(PointCollider* other);      //Collision with point
    bool CheckCollision(LineCollider* other);       //Collision with line
    bool CheckCollision(ChainCollider* other);      //Collision with chain
    bool CheckCollision(CircleCollider* other);     //Collision with circle
    bool CheckCollision(PolygonCollider* other);    //Collision with polygon
    void ShowCollider(QPainter *pntr = nullptr);

    void MoveTo(double _x, double _y);
    void Drag(double dx, double dy);
    void Turn(Angle angle, Point& pivot);

    obstacle GetOutline(double treshold);  //Get graph to ride round this object
};

class PolygonCollider : public Collider
{
public:
    Point** points;
    Point** orig_points;
    int count;

    PolygonCollider(double* x_s, double* y_s, int num, double orig_x, double orig_y);
    ~PolygonCollider();

    void updateBB();    //Update bounding box

    bool CheckCollision(Collider* other);           //Collision with unknown object
    bool CheckCollision(PointCollider* other);      //Collision with point
    bool CheckCollision(LineCollider* other);       //Collision with line
    bool CheckCollision(ChainCollider* other);          //Collision with chain
    bool CheckCollision(CircleCollider* other);     //Collision with circle
    bool CheckCollision(PolygonCollider* other);    //Collision with polygon
    void ShowCollider(QPainter *pntr = nullptr);

    void MoveTo(double _x, double _y);  //Move origin to _x, _y
    void Drag(double dx, double dy);
    void Turn(Angle angle, Point& pivot);
    void Turn(Angle angle);
    void SetAngle(Angle angle);

    obstacle GetOutline(double treshold);  //Get graph to ride round this object
};




#endif // COLLISION_H
