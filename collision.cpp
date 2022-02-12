#include "collision.h"
#include <limits>

//=== Collider class realization ===

Collider::Collider(double o_x, double o_y) : Point (o_x, o_y)   //Collider with origin point in (o_x, o_y)
{
    angle = 0;
    collisions = 0;
}

Collider::~Collider()
{
    //Empty body
}

void Collider::SetAngle(Angle _angle)
{
    angle = _angle;
}


//=== PointCollider realization ===
PointCollider::PointCollider(double _x, double _y) : Collider(_x, _y)
{
    //Empty body
}

PointCollider::~PointCollider()
{
    //Empty body
}

bool PointCollider::CheckCollision(Collider* other)         //Collision with unknown object
{
    return other->CheckCollision(this);
}

bool PointCollider::CheckCollision(PointCollider* other)    //Collision with point
{
    if (*this == *other)    //Using Point::operator==
        return true;
    else
        return false;
}

bool PointCollider::CheckCollision(LineCollider* other)     //Collision with line
{
    return almostEq(other->line->a*x + other->line->b*y + other->line->c, 0) &&
           other->line->GetMinX() < x && x < other->line->GetMaxX();
}

bool PointCollider::CheckCollision(CircleCollider* other)   //Collision with circle
{
    return (distance(x, y, other->GetX(), other->GetY()) < (other->circle->GetR() * other->circle->GetR()));
}

bool PointCollider::CheckCollision(PolygonCollider* other)  //Collision with polygon
{
    //Casting ray from point to left. If intersection with polygon edge is even
    double x_ray = GetX();   //Starting point of ray
    double y_ray = GetY();   //Height of ray

    Point* curr;    //First point of segment
    Point* next;    //Second point of segment
    bool inside = false;    //Presuming, that point is outside

    for(int i = 0; i < other->count; i++)
    {
        curr = other->points[i];
        if (i < other->count - 1)
            next = other->points[i+1];
        else
            next = other->points[0];

        if (almostEq(y_ray, curr->GetY()))  //first point is on ray
        {
            if (almostEq(y_ray, next->GetY()))  //second is too - segment is parallel to ray
            {
                if (almostEq(x_ray, curr->GetX()))  //first point is in start of ray
                    return true;
                if ( (x_ray < curr->GetX()) != (x_ray < next->GetX()) ) //points of segment are on different size of ray start
                    return true;
                //ELSE - dont count as intersection, but it should be counted on next iteration
            }
            else //second point is not on the ray
            {
                if (almostEq(x_ray, curr->GetX()))  //first point is on start of ray
                    return true;
                if (x_ray < curr->GetX())   //first point is on ray
                    inside = !inside;
                //ELSE - there is no intersection
            }
        }
        else if (y_ray < curr->GetY())  //Ray is below first point
        {
            if (almostEq(y_ray, next->GetY()))  //Second point is on ray
            {
                if (almostEq(x_ray, next->GetX()))   //Second point is on start ray
                    return true;
                if (x_ray < next->GetX()) //Second point is on ray
                    inside = !inside;
                //ELSE - there is no intersection
            }
            else if (y_ray > next->GetY())  //Segment is crossing ray
            {
                double inverse_k = (next->GetX() - curr->GetX()) / (next->GetY() - curr->GetY());
                double inverse_b = next->GetX() - inverse_k * next->GetY();
                double intersec_x = inverse_k * y_ray + inverse_b;
                if (x_ray < intersec_x)
                    inside = !inside;
            }
            //ELSE - segment is below ray, there is no intersection
        }
        else    //Ray is above first point. Almost same, as previous case
        {
            if (almostEq(y_ray, next->GetY()))
            {
                if (almostEq(x_ray, next->GetX()))
                    return true;
                if (x_ray < next->GetX())
                    inside = !inside;
            }
            else if (y_ray < next->GetY())
            {
                double inverse_k = (next->GetX() - curr->GetX()) / (next->GetY() - curr->GetY());
                double inverse_b = next->GetX() - inverse_k * next->GetY();
                double intersec_x = inverse_k * y_ray + inverse_b;
                if (x_ray < intersec_x)
                    inside = !inside;
            }
        }

    }
    return inside;
}

void PointCollider::ShowCollider()  //Drawing collider
{
    QPainter painter(picture);
    if (collisions > 0)
        painter.setPen(QColor(0,255,0));
    else
        painter.setPen(QColor(255,0,0));
    painter.drawPoint(GetX(), GetY());
}


//=== LineCollider realization ===
LineCollider::LineCollider(double x1, double y1, double x2, double y2):Collider(x1 ,y1)
{
    line = new Line(Point(x1, y1), Point(x2, y2));
}

LineCollider::~LineCollider()
{
    delete line;
}

bool LineCollider::CheckCollision(Collider* other)           //Collision with unknown object
{
    return other->CheckCollision(this);
}

bool LineCollider::CheckCollision(PointCollider* other)      //Collision with point [ALREADY IMPLEMENTED]
{
    return other->CheckCollision(this);
}

bool LineCollider::CheckCollision(LineCollider* other)       //Collision with line
{
    //Solution of system of linear equations : X = A^(-1)xB
    Matrix A(2, 2);
    A.SetElem(line->a, 0, 0);
    A.SetElem(line->b, 0, 1);
    A.SetElem(other->line->a, 1, 0);
    A.SetElem(other->line->b, 1, 1);
    //Checking determinant
    double _Det = A.det();
    if (almostEq(_Det, 0))
    {
        return almostEq(line->c, other->line->c) && intersect(line->GetMinX(), line->GetMaxX(),
                                                              other->line->GetMinX(), other->line->GetMaxX());   //Parallel or equal
    }
    Matrix C(2, 1);
    C.SetElem(-line->c, 0, 0);
    C.SetElem(-other->line->c, 1, 0);
    Matrix A_inv = A.inverse();
    Matrix res = A_inv * C;
    double res_x = res.GetElem(0, 0);
    double res_y = res.GetElem(0, 1);
    return (line->GetMinX() < res_x && res_x < line->GetMaxX() &&
            line->GetMinY() < res_y && res_y < line->GetMaxY());
}

bool LineCollider::CheckCollision(CircleCollider* other)     //Collision with circle
{
    //Search for nearest point on line to circle
    double normal_a, normal_b;
    normal_a = - line->b;
    normal_b = line->a;
    //Finding C for equation of normal
    double normal_c = -(normal_a * other->GetX() + normal_b * other->GetY());
    //Finding intersection of normal and line
    Point* nearest_pt = intersect2d(normal_a, normal_b, normal_c,
                               line->a, line->b, line->c);
    if (nearest_pt == nullptr)
    {
        throw std::runtime_error("Unexpected error. Somehow line and its normal do not intersect");
    }
    double dist = distance2(nearest_pt->GetX(), nearest_pt->GetY(), other->GetX(), other->GetY());
    return (dist < other->circle->GetR() * other->circle->GetR());
}

bool LineCollider::CheckCollision(PolygonCollider* other)            //Collision with polygon
{
    Point* curr;    //First point of segment
    Point* next;    //Second point of segment
    LineCollider line_c(0,0,1,1);

    for(int i=0; i < other->count; i++)   //Checking every intersection
    {
        curr = other->points[i];
        if (i < other->count-1)
            next = other->points[i+1];
        else
            next = other->points[0];
        line_c.line->Set(curr->GetX(), curr->GetY(), next->GetX(), next->GetY());
        if (line_c.CheckCollision(this))
            return true;
    }
    PointCollider point_c(line->GetMinX(), line->GetMinY());
    return point_c.CheckCollision(other);    //Checking, if whole line is in segment
}

void LineCollider::ShowCollider()
{
    QPainter painter(picture);
    if (collisions > 0)
        painter.setPen(QColor(0,255,0));
    else
        painter.setPen(QColor(255,0,0));
    painter.drawLine(line->GetMinX(), line->GetMinY(), line->GetMaxX(), line->GetMaxY());
    collisions = 0;
}

void LineCollider::MoveTo(double _x, double _y)   //Move first point to (_x, _y) - second point will follow
{
    double dx = line->GetMinX() - line->GetMaxX();
    double dy = line->GetMinY() - line->GetMaxY();
    line->Set(_x, _y, _x + dx, _y + dy);
}

void LineCollider::Drag(double dx, double dy)
{
    line->Set(line->GetMinX() + dx, line->GetMinY() + dy, line->GetMaxX() + dx, line->GetMaxY() + dy);
}

void LineCollider::Turn(Angle angle, Point& pivot)
{
    line->Turn(angle, pivot);
}

void LineCollider::Turn(Angle angle) //Rotate relative to left point
{
    Point pivot(line->GetMinX(), line->GetMinY());
    line->Turn(angle, pivot);
}

void LineCollider::SetAngle(Angle angle)
{
    double length = distance(line->GetMinX(), line->GetMinY(), line->GetMaxX(), line->GetMaxY());
    line->Set(line->GetMinX(), line->GetMinY(),
              line->GetMinX() + length * cos(angle.GetR()), line->GetMinY() + length * sin(angle.GetR()));
}


//=== CircleCollider class realization ===
CircleCollider::CircleCollider(double _x, double _y, double _r) : Collider(_x, _y)
{
    circle = new Circle(_x ,_y, _r);
}

CircleCollider::~CircleCollider()
{
    delete circle;
}

bool CircleCollider::CheckCollision(Collider* other)           //Collision with unknown object
{
    return other->CheckCollision(this);
}

bool CircleCollider::CheckCollision(PointCollider* other)      //Collision with point
{
    return other->CheckCollision(this);
}

bool CircleCollider::CheckCollision(LineCollider* other)       //Collision with line
{
    return other->CheckCollision(this);
}

bool CircleCollider::CheckCollision(CircleCollider* other)    //Collision with circle
{
    double dist = distance2(GetX(), GetY(), other->GetX(), other->GetY());
    return (dist < (circle->GetR() + other->circle->GetR())*(circle->GetR() + other->circle->GetR()));
}

bool CircleCollider::CheckCollision(PolygonCollider* other)            //Collision with polygon
{
    //Is center of circle inside of polygon
    PointCollider point_c(circle->GetX(), circle->GetY());
    if(point_c.CheckCollision(other))
        return true;

    //Are sides of polygon intersecting with circle
    LineCollider line_c(0,0,1,1);
    Point* curr;
    Point* next;

    for(int i=0; i < other->count; i++)
    {
        curr = other->points[i];
        if (i < other->count - 1)
            next = other->points[i+1];
        else
            next = other->points[0];
        line_c.line->Set(curr->GetX(), curr->GetY(), next->GetX(), next->GetY());
        if (line_c.CheckCollision(this))
            return true;
    }
    return false;
}

void CircleCollider::ShowCollider()
{
    QPainter painter(picture);
    if (collisions > 0)
        painter.setPen(QColor(0,255,0));
    else
        painter.setPen(QColor(255,0,0));
    painter.drawEllipse(circle->GetX() - circle->GetR(), circle->GetY() - circle->GetR(), 2 * circle->GetR(), 2 * circle->GetR());
    collisions = 0;
}


//=== PolygonCollider class realization ===
PolygonCollider::PolygonCollider(double* x_s, double* y_s, int num, double orig_x = 0, double orig_y = 0) : Collider(orig_x, orig_y)
{
    points = new Point*[num];
    orig_points = new Point*[num];
    for(int i=0; i<num; i++)
    {
        points[i] = new Point(x_s[i], y_s[i]);
        orig_points[i] = new Point(x_s[i] - x, y_s[i] - y);
    }
    count = num;
}

PolygonCollider::~PolygonCollider()
{
    for(int i=0; i<count; i++)
        delete(points[i]);
    delete[](points);
}

bool PolygonCollider::CheckCollision(Collider* other)           //Collision with unknown object
{
    return other->CheckCollision(this);
}

bool PolygonCollider::CheckCollision(PointCollider* other)      //Collision with point
{
    return other->CheckCollision(this);
}

bool PolygonCollider::CheckCollision(LineCollider* other)       //Collision with line
{
    return other->CheckCollision(this);
}

bool PolygonCollider::CheckCollision(CircleCollider* other)     //Collision with circle
{
    return other->CheckCollision(this);
}

bool PolygonCollider::CheckCollision(PolygonCollider* other)            //Collision with polygon
{
    PointCollider point_c(0,0); //Checking, if any of vertices are inside other polygon
    for(int i=0; i < count; i++)
    {
        point_c.MoveTo(points[i]->GetX(), points[i]->GetY());
        if (point_c.CheckCollision(other))
        {
            return true;
        }
    }
    //Checking, if other polygon is inside of this
    point_c.MoveTo(other->points[0]->GetX(), other->points[0]->GetY());
    if (point_c.CheckCollision(this))
    {
        return true;
    }
    //Checking all sides
    Point* curr, *next;
    LineCollider line_c(0,0,1,1);
    for(int i=0; i < count; i++)
    {
        curr = points[i];
        if (i < count-1)
            next = points[i+1];
        else
            next = points[0];
        line_c.line->Set(curr->GetX(), curr->GetY(), next->GetX(), next->GetY());
        if (line_c.CheckCollision(other))
            return true;
    }
    return false;
}

void PolygonCollider::ShowCollider()
{
    QPainter painter(picture);
    Point* curr, *next;
    if (collisions > 0)
        painter.setPen(QColor(0,255,0));
    else
        painter.setPen(QColor(255,0,0));
    for(int i=0; i < count; i++)
    {
        curr = points[i];
        if (i < count-1)
            next = points[i+1];
        else
            next = points[0];
        painter.drawLine(curr->GetX(), curr->GetY(), next->GetX(), next->GetY());
    }
    collisions = 0;
}

void PolygonCollider::MoveTo(double _x, double _y)  //Move origin to _x, _y
{
    Point::MoveTo(_x, _y);
    for(int i=0; i < count; i++)
    {
        points[i]->Drag(_x - points[i]->GetX(), _y - points[i]->GetY());
    }
}

void PolygonCollider::Drag(double dx, double dy)
{
    Point::Drag(dx, dy);
    for(int i=0; i < count; i++)
    {
        points[i]->Drag(dx, dy);
    }
}

void PolygonCollider::Turn(Angle angle, Point& pivot)
{
    for(int i=0; i < count; i++)
    {
        points[i]->Turn(angle, pivot);
    }
}

void PolygonCollider::Turn(Angle angle)
{
    Point a(x, y);
    for(int i=0; i < count; i++)
    {
        points[i]->Turn(angle, a);
    }
}

void PolygonCollider::SetAngle(Angle angle)
{
    Point a(x, y);
    for(int i=0; i < count; i++)
    {
        points[i]->MoveTo(orig_points[i]->GetX() + x, orig_points[i]->GetY() + y);
        points[i]->Turn(angle, a);
    }
}





