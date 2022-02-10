#include "game.h"

extern QPixmap* picture;
//-------Реализация функций класса Point-------

Point::Point(float _x, float _y)
{
    x = _x;
    y = _y;
}

//Передвижение точки
void Point::MoveTo(float _x, float _y)  //Передвинуть в указанную позицию
{
    x = _x;
    y = _y;
}

void Point::Drag(float dx, float dy)  //Передвинуть на dx пикселей по горизонтали и на dy по вертикали
{
    x += dx;
    y += dy;
}

bool Point::operator== (const Point &b) const   //Равенство двух точек
{
    return (almostEq(x, b.x) && almostEq(y, b.y));
}

bool Point::operator!= (const Point &b) const
{
    return !operator==(b);
}

void Point::Turn(double angle, Point& pivot)    //Повернуть точку относительно точки pivot
{
    //ОПТИМИЗИРОВАТЬ
    double dx = x - pivot.GetX();
    double dy = y - pivot.GetY();

    x = pivot.GetX() + dx * cos(angle) + dy * sin(angle);
    y = pivot.GetY() - dx * sin(angle) + dy * cos(angle);

}

//Получение координат
float Point::GetX()
{
    return x;
}

float Point::GetY()
{
    return y;
}

//-------Реализация функций класса Entity-------
Entity::Entity(float _x, float _y, Collider* _collider) : Point(_x, _y)
{
    collision_mask = _collider;
}

//Функции, выполняющиеся каждый такт
void Entity::EntityUpdate()  //Функция, вызываемая каждый такт. В отличии от OnStep, не должна изменяться в дочерних классах
{
    OnStep();   //Каждый такт сущность выполняет лишь OnStep
    AutoMove(); //BUG -
}

void Entity::AutoMove() //Пустое AutoMove
{

}

void Entity::OnStep()
{

}

void Entity::MoveTo(float _x, float _y)  //Передвинуть в указанную позицию
{
    //Hide();
    if (collision_mask != NULL)
        collision_mask->MoveTo(_x, _y);
    x = _x;
    y = _y;
    Show();
}

void Entity::Drag(float dx, float dy)  //Передвинуть на dx пикселей по горизонтали и на dy по вертикали
{
    //Hide();
    if (collision_mask != NULL)
        collision_mask->Drag(dx, dy);
    x += dx;
    y += dy;
    Show();
}

QString Entity::GetName()  //Получить название обьекта
{
    return QString::fromLocal8Bit("Статический объект");
}

QString Entity::GetInfo()  //Получить информацию об обьекте
{
    QString info;
    info += "x:\t" + QString::number(x) + "\ny:\t" + QString::number(y);
    return info;
}


//-------Реализация функций класса MovingEntity-------

MovingEntity::MovingEntity(float _x, float _y, double _speed, float _angle, Collider* _collider):Entity(_x, _y, _collider)
{
    speed = _speed;
    angle = _angle;
    max_speed = 1000;
}

//Функции, выполняющиеся каждый такт
void MovingEntity::AutoMove()    //Движение согласно скорости и углу объекта
{
    Drag(cos(degtorad(angle)) * speed, -sin(degtorad(angle)) * speed);
}

//Изменение свойств объекта
void MovingEntity::Turn(float _angle)   //Поворот сущности
{
    //Hide();
    angle+=_angle;
    while(angle<0)  //Корректировка углов
        angle+=360;
    while(angle>=360)
        angle-=360;
    if (collision_mask != NULL)
        collision_mask->SetAngle(angle);
    //Show();
}

void MovingEntity::SetSpeed(double _speed) //Установка скорости
{
    if (_speed > max_speed)
        speed = max_speed;
    else if (_speed < -max_speed)
        speed = -max_speed;
    else
        speed = _speed;
}

void MovingEntity::SetAngle(float _angle)    //Установка угла
{
    angle = _angle;
    while(angle<0)  //Корректировка углов
        angle+=360;
    while(angle>=360)
        angle-=360;
    if (collision_mask != NULL)
        collision_mask->SetAngle(angle);
}

//Получение значений
double MovingEntity::GetSpeed()
{
    return speed;
}

float MovingEntity::GetAngle()
{
    return angle;
}

QString MovingEntity::GetName()  //Получить название обьекта
{
    return QString::fromLocal8Bit("Движимый объект");
}

QString MovingEntity::GetInfo()  //Получить информацию об обьекте
{
    QString info;
    info += "x:\t" + QString::number(x) + "\ny:\t" + QString::number(y) + "\n";
    info += QString::fromLocal8Bit("Скорость:\t") + QString::number(speed) + "\n";
    info += QString::fromLocal8Bit("Угол:\t") + QString::number(angle);
    return info;
}


Wall::Wall(float _x, float _y) : Entity(_x, _y)
{
    collision_mask = new LineCollider(_x, _y - 100, _x, _y + 100);
}

Wall::~Wall()
{
    delete collision_mask;
}

void Wall::Show()
{
    QPainter painter(picture);
    painter.setPen(QColor(0, 0, 0));
    painter.drawLine(GetX(), GetY() - 100, GetX(), GetY() + 100);
}

void Wall::Hide()
{
    QPainter painter(picture);
    painter.setPen(QColor(255, 255, 255));
    painter.drawLine(GetX(), GetY() - 100, GetX(), GetY() + 100);
}

QString Wall::GetName()  //Получить название обьекта
{
    return QString::fromLocal8Bit("Стена");
}


Box::Box(float _x, float _y):Entity(_x, _y)
{
    a = 50;
    double xs[4] = {_x - a, _x + a, _x + a, _x - a};
    double ys[4] = {_y - a, _y - a, _y + a, _y + a};
    collision_mask = new PolygonCollider(xs, ys, 4, _x, _y);
}

Box::~Box()
{
    delete collision_mask;
}

void Box::Show()
{
    QPainter painter(picture);
    painter.setPen(QColor(0,0,0));
    painter.setBrush(QColor(128,128,128));
    painter.drawRect(x - a, y - a, 2 * a, 2 * a);
}

void Box::Hide()
{

}

QString Box::GetName()  //Получить название обьекта
{
    return QString::fromLocal8Bit("Ящик");
}

