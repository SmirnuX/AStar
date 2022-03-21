#include "objects.h"
#include "collision.h"
#include "list.h"
extern EntityStack* stack;
extern QPixmap* picture;


//--- Point class realization ---
Point::Point()
{
    //Empty body
}

Point::Point(double _x, double _y)
{
    x = _x;
    y = _y;
}

void Point::MoveTo(double _x, double _y)
{
    x = _x;
    y = _y;
}

void Point::Drag(double dx, double dy)
{
    x += dx;
    y += dy;
}

void Point::Turn(Angle angle, Point& pivot)
{
    double dx = x - pivot.GetX();
    double dy = y - pivot.GetY();

    x = pivot.GetX() + dx * cos(angle.GetR()) + dy * sin(angle.GetR());
    y = pivot.GetY() - dx * sin(angle.GetR()) + dy * cos(angle.GetR());

}

void Point::Turn(Angle angle) //Doesn't make sense for point, but in derived classes
{
    //Empty body
}

bool Point::operator== (const Point &b) const   //Равенство двух точек
{
    return (almostEq(x, b.x) && almostEq(y, b.y));
}

bool Point::operator!= (const Point &b) const
{
    return !operator==(b);
}

//Getters
double Point::GetX()
{
    return x;
}

double Point::GetY()
{
    return y;
}

double distance2(Point a, Point b)
{

    return (b.GetX()-a.GetX())*(b.GetX()-a.GetX()) + (b.GetY()-a.GetY())*(b.GetY()-a.GetY());
}

double distance(Point a, Point b)
{

    return sqrt(distance2(a, b));
}

Point* intersect2d(double a1, double b1, double c1, double a2, double b2, double c2) //Point where lines intersect (or nullptr if there isn't one or several. Additional check is to compare c1 and c2)
{
    //Solution of system of linear equations : X = A^(-1)xB
    Matrix A(2, 2);
    A.SetElem(a1, 0, 0);
    A.SetElem(b1, 0, 1);
    A.SetElem(a2, 1, 0);
    A.SetElem(b2, 1, 1);
    //Checking determinant
    double _Det = A.det();
    if (almostEq(_Det, 0))
    {
        return nullptr;
    }
    //Get two other determinants
    A.SetElem(-c1, 0, 0);
    A.SetElem(-c2, 1, 0);
    double _det1 = A.det();
    A.SetElem(a1, 0, 0);
    A.SetElem(a2, 1, 0);
    A.SetElem(-c1, 0, 1);
    A.SetElem(-c2, 1, 1);;
    double _det2 = A.det();
    double res_x = _det1 / _Det;
    double res_y = _det2 / _Det;
    return new Point(res_x, res_y);
}


//=== Line class realiztion ===
Line::Line(Point p1, Point p2)
{
    min_p = new Point(p1.GetX(), p1.GetY());
    max_p = new Point(p2.GetX(), p2.GetY());
    Update();
}

Line::Line(const Line& other)
{
    min_p = new Point(other.GetMinX(), other.GetMinY());
    max_p = new Point(other.GetMaxX(), other.GetMaxY());
    Update();
}

Line::Line()
{
    //Empty body
    min_p = new Point();
    max_p = new Point();
}

Line::~Line()
{
    if (min_p != nullptr)
        delete min_p;
    if (max_p != nullptr)
        delete max_p;
}

void Line::Set(double x1, double y1, double x2, double y2)
{
    min_p->MoveTo(x1, y1);
    max_p->MoveTo(x2, y2);
    Update();
}

void Line::Update()  //Update line equation
{
    if (max_p->GetX() < min_p->GetX())  //Swap min and max if they are not what they have to be
    {
        Point* temp = min_p;
        min_p = max_p;
        max_p = temp;
    }
    a = min_p->GetY() - max_p->GetY();  //y1-y2
    b = max_p->GetX() - min_p->GetX();  //x2-x1
    c = (min_p->GetX() * max_p->GetY() - min_p->GetY() * max_p->GetX());    //x1y2 - y1x2
}

void Line::Turn(Angle angle, Point &pivot)
{
    min_p->Turn(angle, pivot);
    max_p->Turn(angle, pivot);
    Update();
}

double Line::GetMinX() const
{
    return min_p->GetX();
}

double Line::GetMaxX() const
{
    return max_p->GetX();
}

double Line::GetMinY() const
{
    return min_p->GetY();
}

double Line::GetMaxY() const
{
    return max_p->GetY();
}

Line::operator=(const Line& other)
{
    Set(other.GetMinX(), other.GetMinY(), other.GetMaxX(), other.GetMaxY());
}

//=== Circle class realization ===
Circle::Circle(double _x, double _y, double _r) : Point(_x, _y)
{
    r = _r;
}

Circle::~Circle()
{

}

double Circle::GetR()
{
    return r;
}


//=== Entity class realization ===
Entity::Entity(float _x, float _y, Collider* _collider) : Point(_x, _y)
{
    collision_mask = _collider;
    ToDelete = false;
}

Entity::~Entity()
{
    if (collision_mask != nullptr)
        delete collision_mask;
}

//Functions, that calls every tact
void Entity::EntityUpdate()  //Function, that is called every tact. This function MUST BE NOT CHANGED. When there is need to change behaviour of object, go to OnStep()
{
    OnStep();
    AutoMove();
    if (ToDelete)
    {
        stack->Delete(this);
    }
}

void Entity::AutoMove() //Movement, depending of current speed and angle (isn't implemented in this class, cause static entity should not move)
{
    //Empty body
}

void Entity::OnStep()   //Action, that is happening every tact.
{
    //Empty body
}

void Entity::Delete()
{
    ToDelete = true;
}

void Entity::MoveTo(double _x, double _y)
{
    if (collision_mask != nullptr)
        ((Point*)collision_mask)->MoveTo(_x, _y);
    Show();
}

void Entity::Drag(double dx, double dy)
{
    if (collision_mask != nullptr)
        ((Point*)collision_mask)->Drag(dx, dy);
    x += dx;
    y += dy;
    Show();
}

QString Entity::GetName()
{
    return "Статический объект";
}

QString Entity::GetInfo()
{
    QString info;   //Coords of object
    info += "x:\t" + QString::number(x) + "\ny:\t" + QString::number(y);
    return info;
}

void Entity::ShowOutline()
{
    QPainter pntr(picture);
    QPen penn(QColor(180,230,230));
    penn.setWidth(2);
    pntr.setPen(penn);
    if (this->collision_mask == nullptr)
    {
        pntr.drawEllipse(stack->current->entity->GetX() - 20, stack->current->entity->GetY() - 20, 40, 40);
        pntr.drawEllipse(stack->current->entity->GetX() - 22, stack->current->entity->GetY() - 22, 44, 44);
        pntr.drawEllipse(stack->current->entity->GetX() - 24, stack->current->entity->GetY() - 24, 48, 48);
    }
    else
    {
        collision_mask->ShowCollider(&pntr);
    }
}

//=== MovingEntity class realization ===
MovingEntity::MovingEntity(double _x, double _y, double _speed, Angle _angle, Collider* _collider):Entity(_x, _y, _collider)
{
    speed = _speed;
    angle = _angle;
    max_speed = 1000;
}

//Functions, that calls every tact
void MovingEntity::AutoMove()    //Movement, depending of current speed and angle
{
    Drag(cos(angle.GetR()) * speed, -sin(angle.GetR()) * speed);
}

//Changing variables
void MovingEntity::Turn(Angle _angle)
{
    angle+=_angle;
    if (collision_mask != nullptr)
        collision_mask->SetAngle(angle);
}

void MovingEntity::SetSpeed(double _speed)
{
    if (_speed > max_speed)
        speed = max_speed;
    else if (_speed < -max_speed)
        speed = -max_speed;
    else
        speed = _speed;
}

void MovingEntity::SetAngle(Angle _angle)
{
    angle = _angle;
    if (collision_mask != nullptr)
        collision_mask->SetAngle(angle);
}

//Getters
double MovingEntity::GetSpeed()
{
    return speed;
}

Angle MovingEntity::GetAngle()
{
    return angle;
}

QString MovingEntity::GetName()
{
    return "Движимый объект";
}

QString MovingEntity::GetInfo() //Get coords, speed and angle
{
    QString info;
    info += "x:\t" + QString::number(x) + "\ny:\t" + QString::number(y) + "\n";
    info += "Скорость:\t" + QString::number(speed) + "\n";
    info += "Угол:\t" + QString::number(angle.GetD());
    return info;
}


//=== Various objects ===
Wall::Wall(double _x, double _y) : Entity(_x, _y)
{
    collision_mask = (Collider*) new LineCollider(_x, _y - 100, _x, _y + 100);
}

void Wall::Show()
{
    QPainter painter(picture);
    painter.setPen(QColor(0, 0, 0));
    painter.drawLine(GetX(), GetY() - 100, GetX(), GetY() + 100);
}

QString Wall::GetName()
{
    return "Стена";
}


Box::Box(double _x, double _y):Entity(_x, _y)
{
    a = 50;
    double xs[4] = {_x - a, _x + a, _x + a, _x - a};
    double ys[4] = {_y - a, _y - a, _y + a, _y + a};
    collision_mask = (Collider*) new PolygonCollider(xs, ys, 4, _x, _y);
}

void Box::Show()
{
    QPainter painter(picture);
    painter.setPen(QColor(0,0,0));
    painter.setBrush(QColor(128,128,128));
    painter.drawRect(x - a, y - a, 2 * a, 2 * a);
}

QString Box::GetName()
{
    return "Ящик";
}

WallChain::WallChain(double _x, double _y):Entity(_x, _y)
{
    double xs[3] = {_x, _x + 50, _x};
    double ys[3] = {_y - 50, _y, _y + 50};
    collision_mask = (Collider*) new ChainCollider(xs, ys, 3, _x, _y);
}

void WallChain::Show()
{
    QPainter painter(picture);
    painter.setPen(QColor(0,0,0));
    painter.setBrush(QColor(128,128,128));
    painter.drawLine(x, y-50, x+50, y);
    painter.drawLine(x+50, y, x, y+50);
}

QString WallChain::GetName()
{
    return "Галка";
}


HexBox::HexBox(double _x, double _y):Entity(_x, _y)
{
    a = 50;
    double xs[6];
    double ys[6];
    for (int i = 0; i < 6; i++)
    {
        xs[i] = _x + a * cos(M_PI/3 * i);
        ys[i] = _y + a * sin(M_PI/3 * i);
    }
    collision_mask = (Collider*) new PolygonCollider(xs, ys, 6, _x, _y);
}
void HexBox::Show()
{
    QPainter painter(picture);
    painter.setPen(QColor(0,0,0));
    painter.setBrush(QColor(90,90,90));
    QPointF pts[6];
    for (int i = 0; i < 6; i++)
    {
        pts[i] = QPointF(   x + a * cos(M_PI/3 * i),
                            y + a * sin(M_PI/3 * i));
    }
    painter.drawPolygon(pts, 6);
}

QString HexBox::GetName()
{
    return "Шестиугольник";
}


Barell::Barell(double _x, double _y, double _a):Entity(_x, _y)
{
    a = _a;
    collision_mask = (Collider*) new CircleCollider(_x, _y, _a);
}

void Barell::Show()
{
    QPainter painter(picture);
    painter.setPen(QColor(0,0,0));
    painter.setBrush(QColor(60,60,60));
    painter.drawEllipse(x - a, y - a, 2*a, 2*a);
}

QString Barell::GetName()
{
    return "Бочка";
}


