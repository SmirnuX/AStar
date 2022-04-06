#include "collision.h"


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

bool Collider::CheckBBCollision(Collider* other)    //Check if bounding boxed collides
{
    return ( intersect(top, bottom, other->top, other->bottom) &&
             intersect(left, right, other->left, other->right) );
}

void Collider::SetAngle(Angle _angle)
{
    angle = _angle;
}


//=== PointCollider realization ===
PointCollider::PointCollider(double _x, double _y) : Collider(_x, _y)
{
    PointCollider::updateBB();
}

PointCollider::~PointCollider()
{
    //Empty body
}

void PointCollider::updateBB()    //Update bounding box
{
    top = GetY() + EPSILON;
    bottom = GetY() - EPSILON;
    left = GetX() - EPSILON;
    right = GetX() + EPSILON;
}


bool PointCollider::CheckCollision(Collider* other)         //Collision with unknown object
{
    if (CheckBBCollision(other))
        return other->CheckCollision(this);
    else
        return false;
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

bool PointCollider::CheckCollision(ChainCollider* other)    //Collision with chain
{
    for (int i = 0; i < other->count-1; i++)
    {
        if (almostEq(   other->lines[i]->a*x + other->lines[i]->b*y + other->lines[i]->c, 0) &&
                        other->lines[i]->GetMinX() < x && x < other->lines[i]->GetMaxX())
            return true;
    }
    return false;
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

void PointCollider::ShowCollider(QPainter* pntr)  //Drawing collider
{
    QPainter* painter = pntr;
    bool ext_pntr = false;
    if (painter == nullptr)
    {
        painter = new QPainter(picture);
        if (collisions == 0)
            painter->setPen(QColor(0,150,0));
        else
            painter->setPen(QColor(255,0,0));
        collisions = 0;
        ext_pntr = true;
    }
    painter->drawEllipse(GetX() - 4, GetY() - 4, 8, 8);
    if (ext_pntr)
    {
        delete painter;
    }
}

RadarPoint PointCollider::Raycast(Point* start, Angle angle, double length)
{
    RadarPoint res; //Pointless
    res.distance = -1;
    return res;
}

obstacle PointCollider::GetOutline(double threshold)  //Get graph to ride around this object
{
    obstacle res;
    res.shape = POINT;
    res.point = new Point(GetX(), GetY());
    return res;
}


//=== LineCollider realization ===
LineCollider::LineCollider(double x1, double y1, double x2, double y2):Collider(x1 ,y1)
{
    line = new Line(Point(x1, y1), Point(x2, y2));
    LineCollider::updateBB();
}

LineCollider::~LineCollider()
{
    delete line;
}

void LineCollider::updateBB()    //Update bounding box
{
    top = max(line->GetMinY(), line->GetMaxY());
    bottom = min(line->GetMinY(), line->GetMaxY());
    left = line->GetMinX();
    right = line->GetMaxX();
}

bool LineCollider::CheckCollision(Collider* other)           //Collision with unknown object
{
    if (CheckBBCollision(other))
        return other->CheckCollision(this);
    else
        return false;
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
//    A.print();
    //Checking determinant
    double _Det = A.det();
    if (almostEq(_Det, 0))
    {
        return almostEq(line->c, other->line->c) && intersect(line->GetMinX(), line->GetMaxX(),
                                                              other->line->GetMinX(), other->line->GetMaxX());   //Parallel or equal
    }
    //Get two other determinants
    A.SetElem(-line->c, 0, 0);
    A.SetElem(-other->line->c, 1, 0);
//    A.print();
    double _det1 = A.det();
    A.SetElem(line->a, 0, 0);
    A.SetElem(other->line->a, 1, 0);
    A.SetElem(-line->c, 0, 1);
    A.SetElem(-other->line->c, 1, 1);
//    A.print();
    double _det2 = A.det();
    double res_x = _det1 / _Det;
    double res_y = _det2 / _Det;
    double this_min_y = (line->GetMinY() < line->GetMaxY()) ? line->GetMinY() : line->GetMaxY();
    double this_max_y = (line->GetMinY() > line->GetMaxY()) ? line->GetMinY() : line->GetMaxY();
    double other_min_y = (other->line->GetMinY() < other->line->GetMaxY()) ? other->line->GetMinY() : other->line->GetMaxY();
    double other_max_y = (other->line->GetMinY() > other->line->GetMaxY()) ? other->line->GetMinY() : other->line->GetMaxY();

    bool is_in_this_x = (line->GetMinX() < res_x && res_x < line->GetMaxX()) || almostEq(line->GetMinX(), res_x);
    bool is_in_this_y = (this_min_y < res_y && res_y < this_max_y) || almostEq(line->GetMinY(), res_y);
    bool is_in_other_x = (other->line->GetMinX() < res_x && res_x < other->line->GetMaxX()) || almostEq(other->line->GetMinX(), res_x);
    bool is_in_other_y = (other_min_y < res_y && res_y < other_max_y) || almostEq(other->line->GetMinY(), res_y);
    return is_in_this_x && is_in_this_y && is_in_other_x && is_in_other_y;
}

bool LineCollider::CheckCollision(ChainCollider* other)    //Collision with chain
{
    for (int i = 0; i < other->count-1; i++)
    {
        LineCollider lc = LineCollider(other->lines[i]->GetMinX(), other->lines[i]->GetMinY(),
                                       other->lines[i]->GetMaxX(), other->lines[i]->GetMaxY());
        if (CheckCollision(&lc))
            return true;
    }
    return false;
}

bool LineCollider::CheckCollision(CircleCollider* other)     //Collision with circle
{
    double dist = fabs(line->a * other->GetX() + line->b * other->GetY() + line->c) / sqrt(line->a*line->a + line->b*line->b);
    if (dist > other->circle->GetR())
        return false;

    //Getting nearest point
    double C = line->c + line->a * other->GetX() + line->b * other->GetY();
    double a = line->a * line->a + line->b * line->b;
    double b = 2 *line->b * C;
    double c = C * C - line->a * line->a * other->circle->GetR() * other->circle->GetR();
    double det = b*b - 4*a*c;
    if (det < 0)
        return false;
    double y1 = (- b - sqrt(det)) / (2*a) + other->GetY();
    double y2 = (- b + sqrt(det)) / (2*a) + other->GetY();
    double x1 = - (line->c + line->b * y1) / line->a;
    double x2 = - (line->c + line->b * y2) / line->a;
    if (line->GetMinX() <= x1 && x1 <= line->GetMaxX() &&
        min(line->GetMinY(), line->GetMaxY()) <= y1 && y1 <= max(line->GetMinY(), line->GetMaxY()))
        return true;
    if (line->GetMinX() <= x2 && x2 <= line->GetMaxX() &&
        min(line->GetMinY(), line->GetMaxY()) <= y2 && y2 <= max(line->GetMinY(), line->GetMaxY()))
        return true;
    return false;
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

void LineCollider::ShowCollider(QPainter* pntr)
{
    QPainter* painter = pntr;
    bool ext_pntr = false;
    if (painter == nullptr)
    {
        painter = new QPainter(picture);
        if (collisions == 0)
            painter->setPen(QColor(0,150,0));
        else
            painter->setPen(QColor(255,0,0));
        collisions = 0;
        ext_pntr = true;
    }
    painter->drawLine(line->GetMinX(), line->GetMinY(), line->GetMaxX(), line->GetMaxY());
    if (ext_pntr)
    {
        delete painter;
    }
}

RadarPoint LineCollider::Raycast(Point* start, Angle angle, double length)
{
    RadarPoint res; //No solution
    res.distance = -1;

    //Solution of system of linear equations : X = A^(-1)xB
    Matrix A(2, 2);
    A.SetElem(line->a, 0, 0);
    A.SetElem(line->b, 0, 1);
    Line ray(*start, Point(start->GetX() + length * cos(angle.GetR()),
                           start->GetY() + length * sin(angle.GetR())));
    A.SetElem(ray.a, 1, 0);
    A.SetElem(ray.b, 1, 1);

    //Checking determinant
    double _Det = A.det();
    if (almostEq(_Det, 0))
    {
        if (intersect(line->GetMinX(), line->GetMaxX(),
                      ray.GetMinX(), ray.GetMaxX()) && almostEq(line->c, ray.c))
        {   //Parallel or equal
            return res; //Equal - there should be something
        }
        else
            return res;
    }
    //Get two other determinants
    A.SetElem(-line->c, 0, 0);
    A.SetElem(-ray.c, 1, 0);

    double _det1 = A.det();
    A.SetElem(line->a, 0, 0);
    A.SetElem(ray.a, 1, 0);
    A.SetElem(-line->c, 0, 1);
    A.SetElem(-ray.c, 1, 1);

    double _det2 = A.det();
    double res_x = _det1 / _Det;
    double res_y = _det2 / _Det;

    res.pt = Point(res_x, res_y);
    res.distance = distance(start->GetX(), start->GetY(), res_x, res_y);
    if (res.distance > length)
        res.distance = -1;
    return res;
}

void LineCollider::MoveTo(double _x, double _y)   //Move first point to (_x, _y) - second point will follow
{
    if (almostEq(line->GetMinX(), GetX()) && almostEq(line->GetMinY(), GetY()))
    {
        //If point with min x is origin
        double dx = line->GetMaxX() - line->GetMinX();
        double dy = line->GetMaxY() - line->GetMinY();
        line->Set(_x, _y, _x + dx, _y + dy);
    }
    else
    {
        //If point with max x is origin
        double dx = line->GetMinX() - line->GetMaxX();
        double dy = line->GetMinY() - line->GetMaxY();
        line->Set(_x - dx, _y - dy, _x, _y);
    }
    Point::MoveTo(_x, _y);
    updateBB();
}

void LineCollider::Drag(double dx, double dy)
{
    line->Set(line->GetMinX() + dx, line->GetMinY() + dy, line->GetMaxX() + dx, line->GetMaxY() + dy);
    Point::Drag(dx, dy);
    updateBB();
}

void LineCollider::Turn(Angle angle, Point& pivot)
{
    line->Turn(angle, pivot);
    updateBB();
}

void LineCollider::Turn(Angle angle) //Rotate relative to origin
{
    Point pivot(GetX(), GetY());
    line->Turn(angle, pivot);
    updateBB();
}

void LineCollider::SetAngle(Angle angle)
{
    double length = distance(line->GetMinX(), line->GetMinY(), line->GetMaxX(), line->GetMaxY());
    line->Set(GetX(), GetY(),
              GetX() + length * cos(angle.GetR()), GetY() + length * sin(angle.GetR()));
    updateBB();
}

obstacle LineCollider::GetOutline(double threshold)  //Get graph to ride round this object
{
    obstacle res;
    res.shape = POLYGON;
    res.num = 4;
    res.outline = new edge[4];  //Two lines and two arcs (==)
    Angle dir = direction_to_point(line->GetMinX(), line->GetMinY(), line->GetMaxX(), line->GetMaxY());
    //Arc around first point
    res.outline[0].type = ARC_CIRCLE;
    res.outline[0].r = threshold;
    res.outline[0].cx = line->GetMinX();
    res.outline[0].cy = line->GetMinY();
    res.outline[0].aA = dir.normalL();
    res.outline[0].aB = dir.normalR();
    res.outline[0].direction = COUNTERCLOCKWISE;
    //First parallel line
    res.outline[1].type = LINEAR;
    res.outline[1].A = Point(line->GetMinX() + threshold * cos(dir.normalR().GetR()),
                             line->GetMinY() + threshold * sin(dir.normalR().GetR()));
    res.outline[1].B = Point(line->GetMaxX() + threshold * cos(dir.normalR().GetR()),
                             line->GetMaxY() + threshold * sin(dir.normalR().GetR()));
    //Arc around second point
    res.outline[2].type = ARC_CIRCLE;
    res.outline[2].r = threshold;
    res.outline[2].cx = line->GetMaxX();
    res.outline[2].cy = line->GetMaxY();
    res.outline[2].aA = dir.normalR();
    res.outline[2].aB = dir.normalL();
    res.outline[2].direction = COUNTERCLOCKWISE;
    //Second parallel line
    res.outline[3].type = LINEAR;
    res.outline[3].A = Point(line->GetMaxX() + threshold * cos(dir.normalL().GetR()),
                             line->GetMaxY() + threshold * sin(dir.normalL().GetR()));
    res.outline[3].B = Point(line->GetMinX() + threshold * cos(dir.normalL().GetR()),
                             line->GetMinY() + threshold * sin(dir.normalL().GetR()));

    return res;
}


//=== ChainCollider class realization ===
ChainCollider::ChainCollider(double* x_s, double* y_s, int num, double orig_x, double orig_y) : Collider(orig_x, orig_y)
{
    points = new Point*[num];
    lines = new Line*[num - 1];
    orig_points = new Point*[num];
    for(int i=0; i<num; i++)    //Copy
    {
        points[i] = new Point(x_s[i], y_s[i]);
        orig_points[i] = new Point(x_s[i] - x, y_s[i] - y);
        if (i != 0)
        {
            lines[i-1] = new Line(Point(x_s[i-1], y_s[i-1]), Point(x_s[i], y_s[i]));
        }
    }
    count = num;
    ChainCollider::updateBB();
}

ChainCollider::~ChainCollider()
{
    for(int i=0; i<count; i++)
    {
        delete(points[i]);
        delete(orig_points[i]);
        if (i != 0)
            delete(lines[i-1]);
    }
    delete[](orig_points);
    delete[](lines);
    delete[](points);
}

void ChainCollider::updateBB()    //Update bounding box
{
    double l = points[0]->GetX();
    double r = points[0]->GetX();
    double t = points[0]->GetY();
    double b = points[0]->GetY();
    for (int i = 1; i < count; i++)
    {
        if (points[i]->GetX() < l)
            l = points[i]->GetX();
        if (points[i]->GetX() > r)
            r = points[i]->GetX();
        if (points[i]->GetY() < b)
            b = points[i]->GetY();
        if (points[i]->GetY() > t)
            t = points[i]->GetY();
    }
    top = t;
    left = l;
    bottom = b;
    right = r;
}

void ChainCollider::update_eq()   //Update equations
{
    for(int i=1; i<count; i++)    //Copy
    {
        lines[i-1]->Set(points[i-1]->GetX(), points[i-1]->GetY(), points[i]->GetX(), points[i]->GetY());
    }
}

bool ChainCollider::CheckCollision(Collider* other)           //Collision with unknown object
{
    if (CheckBBCollision(other))
        return other->CheckCollision(this);
    else
        return false;
}

bool ChainCollider::CheckCollision(PointCollider* other)      //Collision with point
{
    return other->CheckCollision(this);
}

bool ChainCollider::CheckCollision(LineCollider* other)       //Collision with line
{
    return other->CheckCollision(this);
}

bool ChainCollider::CheckCollision(ChainCollider* other)      //Collision with chain
{
    for (int i = 0; i < other->count; i++)
    {
        LineCollider lc = LineCollider(other->lines[i]->GetMinX(), other->lines[i]->GetMinY(),
                                       other->lines[i]->GetMaxX(), other->lines[i]->GetMaxY());
        if (CheckCollision(&lc))
            return true;
    }
    return false;
}

bool ChainCollider::CheckCollision(CircleCollider* other)     //Collision with circle
{
    for (int i = 0; i < count-1; i++)
    {
        LineCollider lc = LineCollider(lines[i]->GetMinX(), lines[i]->GetMinY(),
                                       lines[i]->GetMaxX(), lines[i]->GetMaxY());
        if (lc.CheckCollision(other))
            return true;
    }
    return false;
}

bool ChainCollider::CheckCollision(PolygonCollider* other)    //Collision with polygon
{
    for (int i = 0; i < count-1; i++)
    {
        LineCollider lc = LineCollider(lines[i]->GetMinX(), lines[i]->GetMinY(),
                                       lines[i]->GetMaxX(), lines[i]->GetMaxY());
        if (lc.CheckCollision(other))
            return true;
    }
    return false;
}

void ChainCollider::ShowCollider(QPainter *pntr)
{
    QPainter* painter = pntr;
    bool ext_pntr = false;
    if (painter == nullptr)
    {
        painter = new QPainter(picture);
        if (collisions == 0)
            painter->setPen(QColor(0,150,0));
        else
            painter->setPen(QColor(255,0,0));
        collisions = 0;
        ext_pntr = true;
    }
    for (int i = 0; i < count-1; i++)
    {
        painter->drawLine(lines[i]->GetMinX(), lines[i]->GetMinY(), lines[i]->GetMaxX(), lines[i]->GetMaxY());
    }
    if (ext_pntr)
    {
        delete painter;
    }
}

RadarPoint ChainCollider::Raycast(Point* start, Angle angle, double length)
{
    RadarPoint res; //No solution
    res.distance = -1;
    for (int i = 0; i < count-1; i++)
    {
        LineCollider lc = LineCollider(lines[i]->GetMinX(), lines[i]->GetMinY(),
                                       lines[i]->GetMaxX(), lines[i]->GetMaxY());
        RadarPoint curr = lc.Raycast(start, angle, length);
        if (curr.distance < res.distance || curr.distance > res.distance && res.distance == -1)
        {
            res = curr;
        }
    }
    if (res.distance > length)
        res.distance = -1;

    return res;
}


void ChainCollider::MoveTo(double _x, double _y)  //Move origin to _x, _y
{
    double dx = _x - GetX();
    double dy = _y - GetY();
    Point::MoveTo(_x, _y);
    for(int i=0; i < count; i++)
    {
        points[i]->Drag(dx, dy);
    }
    update_eq();
    updateBB();
}

void ChainCollider::Drag(double dx, double dy)
{
    Point::Drag(dx, dy);
    for(int i=0; i < count; i++)
    {
        points[i]->Drag(dx, dy);
    }
    update_eq();
}

void ChainCollider::Turn(Angle angle, Point& pivot)
{
    for(int i=0; i < count; i++)
    {
        points[i]->Turn(angle, pivot);
    }
    update_eq();
    updateBB();
}

void ChainCollider::Turn(Angle angle)
{
    Point a(x, y);
    for(int i=0; i < count; i++)
    {
        points[i]->Turn(angle, a);
    }
    update_eq();
    updateBB();
}

void ChainCollider::SetAngle(Angle angle)
{
    Point a(x, y);
    for(int i=0; i < count; i++)
    {
        points[i]->MoveTo(orig_points[i]->GetX() + x, orig_points[i]->GetY() + y);
        points[i]->Turn(angle, a);
    }
    update_eq();
    updateBB();
}

obstacle ChainCollider::GetOutline(double threshold)  //Get graph to ride round this object
{
    obstacle res;
    res.shape = POLYGON;
    if (count == 2) //If chain consists of one segment
    {
        LineCollider lc(points[0]->GetX(), points[0]->GetY(), points[1]->GetX(), points[1]->GetY());
        return lc.GetOutline(threshold);
    }
    res.outline = new edge[count + 2 * (count-1)];  //N arcs and 2(N-1) lines
    res.num = count + 2 * (count-1);

    //First arc
    Angle dir = direction_to_point(points[0]->GetX(), points[0]->GetY(), points[1]->GetX(), points[1]->GetY());
    res.outline[0].type = ARC_CIRCLE;
    res.outline[0].r = threshold;
    res.outline[0].cx = points[0]->GetX();
    res.outline[0].cy = points[0]->GetY();
    res.outline[0].aA = dir.normalL();
    res.outline[0].aB = dir.normalR();
    res.outline[0].direction = COUNTERCLOCKWISE;

    int j = 1;  //Index of next edge

    //Create two parallel lines for first segment
    Point r1 = Point(   points[0]->GetX() + threshold * cos(dir.normalR().GetR()),
                        points[0]->GetY() + threshold * sin(dir.normalR().GetR()));
    Point r2 = Point(   points[1]->GetX() + threshold * cos(dir.normalR().GetR()),
                        points[1]->GetY() + threshold * sin(dir.normalR().GetR()));

    Point l1 = Point(   points[0]->GetX() + threshold * cos(dir.normalL().GetR()),
                        points[0]->GetY() + threshold * sin(dir.normalL().GetR()));
    Point l2 = Point(   points[1]->GetX() + threshold * cos(dir.normalL().GetR()),
                        points[1]->GetY() + threshold * sin(dir.normalL().GetR()));
    Point l3, r3, l4, r4;

    //Arcs and lines between i-1 and i vertices
    for (int i = 2; i < count; i++)   //Up to 3*(count-2)
    {   
        Angle dir2 = direction_to_point(points[i-1]->GetX(), points[i-1]->GetY(), points[i]->GetX(), points[i]->GetY());
        r3 = Point( points[i-1]->GetX() + threshold * cos(dir2.normalR().GetR()),
                    points[i-1]->GetY() + threshold * sin(dir2.normalR().GetR()));
        l3 = Point( points[i-1]->GetX() + threshold * cos(dir2.normalL().GetR()),
                    points[i-1]->GetY() + threshold * sin(dir2.normalL().GetR()));
        r4 = Point( points[i]->GetX() + threshold * cos(dir2.normalR().GetR()),
                    points[i]->GetY() + threshold * sin(dir2.normalR().GetR()));
        l4 = Point( points[i]->GetX() + threshold * cos(dir2.normalL().GetR()),
                    points[i]->GetY() + threshold * sin(dir2.normalL().GetR()));
        //Check if there is intersection on right side
        SIDE intersec = FORWARD;   //0 - there is no intersection (segments are parallel), 1 - left segments are intersected, -1 - right
        double alpha = dir.GetR();
        double beta = dir2.GetR();
        if (beta > M_PI)
        {
            beta = 2 * M_PI - beta;
        }
        alpha = 2 * M_PI - alpha;
        if (almostEq(alpha, beta))
            intersec = FORWARD;   //[Don't actually need it here, but for better readability]
        else if (alpha > beta)
            intersec = LEFT;  //Intersection on left side
        else
            intersec = RIGHT; // Intersection on right side
        Line line_1;
        Line line_2;
        Point* tmp = nullptr;
        //Adding arc (if there is any intersection)
        switch (intersec)
        {
        case FORWARD:  //Adding two edges instead of three [Can be optimized to 2 instead of 5, but due to low probability it doesnt make sense]
            res.num--;
            break;
        case LEFT:  //Adding arc and calculating intersection
            //Calculating intersection
            line_1 = Line(l1, l2);
            line_2 = Line(l3, l4);
            tmp = intersect2d(line_1.a, line_1.b, line_1.c, line_2.a, line_2.b, line_2.c);
            assert(tmp!=nullptr);
            l2 = *tmp;
            l3 = *tmp;
            //Adding arc to the right
            res.outline[j].type = ARC_CIRCLE;
            res.outline[j].r = threshold;
            res.outline[j].cx = points[i-1]->GetX();
            res.outline[j].cy = points[i-1]->GetY();
            res.outline[j].aA = dir.normalR();
            res.outline[j].aB = dir2.normalR();
            res.outline[j].direction = COUNTERCLOCKWISE;
            j++;
            break;
        case RIGHT:
            //Calculating intersection
            line_1 = Line(r1, r2);
            line_2 = Line(r3, r4);
            tmp = intersect2d(line_1.a, line_1.b, line_1.c, line_2.a, line_2.b, line_2.c);
            assert(tmp!=nullptr);
            r2 = *tmp;
            r3 = *tmp;
            //Adding arc to the left
            res.outline[j].type = ARC_CIRCLE;
            res.outline[j].r = threshold;
            res.outline[j].cx = points[i-1]->GetX();
            res.outline[j].cy = points[i-1]->GetY();
            res.outline[j].aA = dir.normalL();
            res.outline[j].aB = dir2.normalL();
            res.outline[j].direction = CLOCKWISE;
            j++;
            break;
        }
        //Adding linear edges
        res.outline[j].type = LINEAR;
        res.outline[j].A = l1;
        res.outline[j].B = l2;
        res.outline[j+1].type = LINEAR;
        res.outline[j+1].A = r1;
        res.outline[j+1].B = r2;
        j+=2;
        r1 = r3;
        l1 = l3;
        r2 = r4;
        l2 = l4;
        dir = dir2;
    }
    //Last two lines
    res.outline[j].type = LINEAR;
    res.outline[j].A = l1;
    res.outline[j].B = l2;
    res.outline[j+1].type = LINEAR;
    res.outline[j+1].A = r1;
    res.outline[j+1].B = r2;

    //Last arc
    res.outline[j+2].type = ARC_CIRCLE;
    res.outline[j+2].r = threshold;
    res.outline[j+2].cx = points[count-1]->GetX();
    res.outline[j+2].cy = points[count-1]->GetY();
    res.outline[j+2].aA = dir.normalR();
    res.outline[j+2].aB = dir.normalL();
    res.outline[j+2].direction = COUNTERCLOCKWISE;

    assert((j+2) == (res.num-1)); //Test if count is correct
    return res;
}


//=== CircleCollider class realization ===
CircleCollider::CircleCollider(double _x, double _y, double _r) : Collider(_x, _y)
{
    circle = new Circle(_x ,_y, _r);
    CircleCollider::updateBB();
}

CircleCollider::~CircleCollider()
{
    delete circle;
}

void CircleCollider::updateBB()
{
    top = GetY() + circle->GetR();
    bottom = GetY() - circle->GetR();
    right = GetX() + circle->GetR();
    left = GetX() - circle->GetR();
}

bool CircleCollider::CheckCollision(Collider* other)           //Collision with unknown object
{
    if (CheckBBCollision(other))
        return other->CheckCollision(this);
    else
        return false;
}

bool CircleCollider::CheckCollision(PointCollider* other)      //Collision with point
{
    return other->CheckCollision(this);
}

bool CircleCollider::CheckCollision(LineCollider* other)       //Collision with line
{
    return other->CheckCollision(this);
}

bool CircleCollider::CheckCollision(ChainCollider* other)    //Collision with chain
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
    PointCollider point_c(GetX(), GetY());
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

void CircleCollider::ShowCollider(QPainter* pntr)
{
    QPainter* painter = pntr;
    bool ext_pntr = false;
    if (painter == nullptr)
    {
        painter = new QPainter(picture);
        if (collisions == 0)
            painter->setPen(QColor(0,150,0));
        else
            painter->setPen(QColor(255,0,0));
        collisions = 0;
        ext_pntr = true;
    }
    painter->drawEllipse(GetX() - circle->GetR(), GetY() - circle->GetR(), 2 * circle->GetR(), 2 * circle->GetR());
    if (ext_pntr)
    {
        delete painter;
    }
}


RadarPoint CircleCollider::Raycast(Point* start, Angle angle, double length)
{
    Line ray(*start, Point(start->GetX() + length * cos(angle.GetR()),
                           start->GetY() + length * sin(angle.GetR())));
    RadarPoint res;
    res.distance = -1;
    double dist = fabs(ray.a * GetX() + ray.b * GetY() + ray.c) / sqrt(ray.a*ray.a + ray.b*ray.b);
    if (dist > circle->GetR())
        return res;

    //Getting nearest point
    double C = ray.c + ray.a * GetX() + ray.b * GetY();
    double a = ray.a * ray.a + ray.b * ray.b;
    double b = 2 * ray.b * C;
    double c = C * C - ray.a * ray.a * circle->GetR() * circle->GetR();
    double det = b*b - 4*a*c;
    if (det < 0)
        return res;
    double y1 = (- b - sqrt(det)) / (2*a) + GetY();
    double y2 = (- b + sqrt(det)) / (2*a) + GetY();
    double x1 = - (ray.c + ray.b * y1) / ray.a;
    double x2 = - (ray.c + ray.b * y2) / ray.a;

    res.distance = distance(start->GetX(), start->GetY(), x1, y1);
    if (distance(start->GetX(), start->GetY(), x2, y2) > res.distance && distance(start->GetX(), start->GetY(), x2, y2) <= length)
    {
        res.pt = Point(x2, y2);
        res.distance = distance(start->GetX(), start->GetY(), x2, y2);
    }
    else
        res.pt = Point(x1, y1);
    if (res.distance > length)
        res.distance = -1;
    return res;
}

void CircleCollider::MoveTo(double _x, double _y)
{
    Point::MoveTo(_x, _y);
    updateBB();
}

void CircleCollider::Drag(double dx, double dy)
{
    Point::Drag(dx, dy);
    updateBB();
}

void CircleCollider::Turn(Angle angle, Point& pivot)
{
    Point::Turn(angle, pivot);
    updateBB();
}

obstacle CircleCollider::GetOutline(double threshold)  //Get graph to ride round this object
{
    obstacle res;
    res.shape = CIRCLE;
    res.point = new Point(GetX(), GetY());
    res.r = this->circle->GetR() + threshold;
    return res;
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
    PolygonCollider::updateBB();
}

PolygonCollider::~PolygonCollider()
{
    for(int i=0; i<count; i++)
    {
        delete(points[i]);
        delete(orig_points[i]);
    }
    delete[](points);
    delete[](orig_points);
}

void PolygonCollider::updateBB()    //Update bounding box
{
    double l = points[0]->GetX();
    double r = points[0]->GetX();
    double t = points[0]->GetY();
    double b = points[0]->GetY();
    for (int i = 1; i < count; i++)
    {
        if (points[i]->GetX() < l)
            l = points[i]->GetX();
        if (points[i]->GetX() > r)
            r = points[i]->GetX();
        if (points[i]->GetY() < b)
            b = points[i]->GetY();
        if (points[i]->GetY() > t)
            t = points[i]->GetY();
    }
    top = t;
    left = l;
    bottom = b;
    right = r;
}

bool PolygonCollider::CheckCollision(Collider* other)           //Collision with unknown object
{
    if (CheckBBCollision(other))
        return other->CheckCollision(this);
    else
        return false;
}

bool PolygonCollider::CheckCollision(PointCollider* other)      //Collision with point
{
    return other->CheckCollision(this);
}

bool PolygonCollider::CheckCollision(LineCollider* other)       //Collision with line
{
    return other->CheckCollision(this);
}

bool PolygonCollider::CheckCollision(ChainCollider* other)    //Collision with chain
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

void PolygonCollider::ShowCollider(QPainter *pntr)
{
    QPainter* painter = pntr;
    bool ext_pntr = false;
    if (painter == nullptr)
    {
        painter = new QPainter(picture);
        if (collisions == 0)
            painter->setPen(QColor(0,150,0));
        else
            painter->setPen(QColor(255,0,0));
        collisions = 0;
        ext_pntr = true;
    }
    Point* curr, *next;
    for(int i=0; i < count; i++)
    {
        curr = points[i];
        if (i < count-1)
            next = points[i+1];
        else
            next = points[0];
        painter->drawLine(curr->GetX(), curr->GetY(), next->GetX(), next->GetY());
    }
    if (ext_pntr)
    {
        delete painter;
    }
}

RadarPoint PolygonCollider::Raycast(Point* start, Angle angle, double length)
{
    RadarPoint res; //No solution
    res.distance = -1;

    Point* curr;    //First point of segment
    Point* next;    //Second point of segment
    LineCollider line_c(0,0,1,1);

    for(int i=0; i < count; i++)   //Checking every intersection
    {
        curr = points[i];
        if (i < count-1)
            next = points[i+1];
        else
            next = points[0];
        line_c.line->Set(curr->GetX(), curr->GetY(), next->GetX(), next->GetY());

        RadarPoint curr_rp = line_c.Raycast(start, angle, length);
        if (curr_rp.distance < res.distance || curr_rp.distance > res.distance && res.distance == -1)
        {
            res = curr_rp;
        }
    }
    if (res.distance > length)
        res.distance = -1;

    return res;
}

void PolygonCollider::MoveTo(double _x, double _y)  //Move origin to _x, _y
{
    double dx = _x - GetX();
    double dy = _y - GetY();
    Point::MoveTo(_x, _y);
    for(int i=0; i < count; i++)
    {
        points[i]->Drag(dx, dy);
    }
    updateBB();
}

void PolygonCollider::Drag(double dx, double dy)
{
    Point::Drag(dx, dy);
    for(int i=0; i < count; i++)
    {
        points[i]->Drag(dx, dy);
    }
    updateBB();
}

void PolygonCollider::Turn(Angle angle, Point& pivot)
{
    for(int i=0; i < count; i++)
    {
        points[i]->Turn(angle, pivot);
    }
    updateBB();
}

void PolygonCollider::Turn(Angle angle)
{
    Point a(x, y);
    for(int i=0; i < count; i++)
    {
        points[i]->Turn(angle, a);
    }
    updateBB();
}

void PolygonCollider::SetAngle(Angle angle)
{
    Point a(x, y);
    for(int i=0; i < count; i++)
    {
        points[i]->MoveTo(orig_points[i]->GetX() + x, orig_points[i]->GetY() + y);
        points[i]->Turn(angle, a);
    }
    updateBB();
}

obstacle PolygonCollider::GetOutline(double threshold)  //Get graph to ride round this object
{
    obstacle res;
    res.shape = POLYGON;
    if (count == 2) //If polygon consisits of one segment [?]
    {
        LineCollider lc(points[0]->GetX(), points[0]->GetY(), points[1]->GetX(), points[1]->GetY());
        return lc.GetOutline(threshold);
    }
    res.outline = new edge[2 * count];  //N lines and up to N arcs [N if polygon is convex]
    res.num = 2 * count;

    //Define normal
    SIDE normal = FORWARD;
    //Calculate oriented area
    double sum = 0;
    int last = count - 1;
    for (int i = 0; i < count; i++)
    {
        sum += points[last]->GetX() * points[i]->GetY() - points[last]->GetY() * points[i]->GetX();
        last = i;
    }
    //If oriented area is greater then zero, then points are sorted clockwise, and out normal will be on left side
    if (sum < 0)
        normal = LEFT;
    else
        normal = RIGHT;

    //Preview of last line
    Angle dir = direction_to_point(points[count-1]->GetX(), points[count-1]->GetY(), points[0]->GetX(), points[0]->GetY());
    double normal_dir = (normal == LEFT)?dir.normalL().GetR():dir.normalR().GetR();

    //Create parallel line for last segment
    Point p1 = Point(   points[count-1]->GetX() + threshold * cos(normal_dir),
                        points[count-1]->GetY() + threshold * sin(normal_dir));
    Point p2 = Point(   points[0]->GetX() + threshold * cos(normal_dir),
                        points[0]->GetY() + threshold * sin(normal_dir));

    Point p3, p4;
    int j = 0;  //Index of added edge
    //Arcs and lines between i-1 and i vertices
    for (int i = 0; i < count; i++)
    {
        int next = (i == count-1) ? 0 : i+1;
        Angle dir2 = direction_to_point(points[i]->GetX(), points[i]->GetY(), points[next]->GetX(), points[next]->GetY());
        double normal_dir2 = (normal == LEFT)?dir2.normalL().GetR():dir2.normalR().GetR();

        p3 = Point( points[i]->GetX() + threshold * cos(normal_dir2),   //Getting parallel to current segment
                    points[i]->GetY() + threshold * sin(normal_dir2));
        p4 = Point( points[next]->GetX() + threshold * cos(normal_dir2),
                    points[next]->GetY() + threshold * sin(normal_dir2));


        //Check if there is intersection with previous parallel
        bool intersec = false;
        double alpha = dir.GetR();
        double beta = dir2.GetR();
        if (beta > M_PI)
        {
            beta = 2 * M_PI - beta;
        }
        alpha = 2 * M_PI - alpha;
        if (almostEq(alpha, beta))
            intersec = false;   //[Don't actually need it here, but for better readability]
        else if (alpha > beta)
            intersec = normal == LEFT;  //Intersection on left side
        else
            intersec = normal == RIGHT; // Intersection on right side
        Line line_1;
        Line line_2;
        Point* tmp = nullptr;
        //Adding arc (if there isnt any intersection)
        if (intersec)
        {
            res.num--;
            //Calculating intersection
            line_1 = Line(p1, p2);
            line_2 = Line(p3, p4);
            tmp = intersect2d(line_1.a, line_1.b, line_1.c, line_2.a, line_2.b, line_2.c);
            assert(tmp!=nullptr);
            p2 = *tmp;
            p3 = *tmp;
        }
        else
        {
            //Adding arc
            res.outline[j].type = ARC_CIRCLE;
            res.outline[j].r = threshold;
            res.outline[j].cx = points[i]->GetX();
            res.outline[j].cy = points[i]->GetY();
            res.outline[j].aA = Angle(normal_dir);
            res.outline[j].aB = Angle(normal_dir2);
            res.outline[j].direction = (normal == RIGHT)?COUNTERCLOCKWISE:CLOCKWISE;
            j++;
        }

        //Adding linear edge
        res.outline[j].type = LINEAR;
        res.outline[j].A = p3;
        res.outline[j].B = p4;
        j++;
        p1 = p3;
        p2 = p4;
        dir = dir2;
        normal_dir = normal_dir2;
    }
    assert(j == res.num);
    return res;
}





