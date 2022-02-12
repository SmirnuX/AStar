#include "objects.h"

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

Point::~Point()
{
    //Empty destructor
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


//=== Line class realiztion ===
Line::Line(Point p1, Point p2)
{
    min_p = new Point(p1.GetX(), p1.GetY());
    max_p = new Point(p2.GetX(), p2.GetY());
    Update();
}

Line::~Line()
{
    delete min_p;
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
    a = min_p->GetY() - max_p->GetY();
    b = max_p->GetX() - min_p->GetX();
    c = (min_p->GetX() * max_p->GetY() - min_p->GetY() * max_p->GetX());
}

void Line::Turn(Angle angle, Point &pivot)
{
    min_p->Turn(angle, pivot);
    max_p->Turn(angle, pivot);
    Update();
}

double Line::GetMinX()
{
    return min_p->GetX();
}

double Line::GetMaxX()
{
    return max_p->GetX();
}

double Line::GetMinY()
{
    return min_p->GetY();
}

double Line::GetMaxY()
{
    return max_p->GetY();
}


//=== Circle class realization ===
Circle::Circle(double _x, double _y, double _r) : Point(_x, _y)
{
    r = _r;
}

Circle::~Circle()
{

}



//=== Entity class realization ===
Entity::Entity(float _x, float _y, Collider* _collider) : Point(_x, _y)
{
    collision_mask = _collider;
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
}

void Entity::AutoMove() //Movement, depending of current speed and angle (isn't implemented in this class, cause static entity should not move)
{
    //Empty body
}

void Entity::OnStep()   //Action, that is happening every tact.
{
    //Empty body
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
    return QString::fromLocal8Bit("Статический объект");
}

QString Entity::GetInfo()
{
    QString info;   //Coords of object
    info += "x:\t" + QString::number(x) + "\ny:\t" + QString::number(y);
    return info;
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
    return QString::fromLocal8Bit("Движимый объект");
}

QString MovingEntity::GetInfo() //Get coords, speed and angle
{
    QString info;
    info += "x:\t" + QString::number(x) + "\ny:\t" + QString::number(y) + "\n";
    info += QString::fromLocal8Bit("Скорость:\t") + QString::number(speed) + "\n";
    info += QString::fromLocal8Bit("Угол:\t") + QString::number(angle.GetD());
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
    return QString::fromLocal8Bit("Стена");
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
    return QString::fromLocal8Bit("Ящик");
}

