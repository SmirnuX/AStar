#include "tanks.h"
#include "add_math.h"
extern EntityStack* stack;

//=== BaseTank class realization ===

BaseTank::BaseTank(double _x, double _y):MovingEntity(_x, _y, 0, 0, nullptr)
{
    cannon_angle = 0; //Tank head angle
    reload_timeout = 0;   //Timeout before next shot
}

void BaseTank::Show()   //Drawing tank
{
    ShowTracks();
    ShowBase();
    ShowCannon();
    if (SHOW_COLLIDERS)
        FOV_collider->ShowCollider();
}

void BaseTank::ShowFOV(QPainter* pic_pntr, double _x, double _y) //Drawing FOV
{
    QBrush Brush(QColor(255, 0, 0, 20));
    pic_pntr->setPen(QColor(0, 0, 0, 0));
    pic_pntr->setBrush(Brush);
    pic_pntr->drawPie(_x-FOV_distance, _y-FOV_distance, 2 * FOV_distance, 2 * FOV_distance, (angle + cannon_angle - FOV_angle).GetD() * 16, FOV_angle.GetD() * 32);
}

void BaseTank::SetCannonAngle(Angle _angle) //Rotate head of tank
{
    cannon_angle = _angle;
    Show();
}

void BaseTank::Accelerate()
{
    Accelerate(acc);
}

void BaseTank::Accelerate(double _acc)
{
    if (_acc < acc)
    {
        SetSpeed(GetSpeed() + _acc);
    }
    else
    {
        SetSpeed(GetSpeed() + _acc);
    }

}

void BaseTank::Deccelerate()
{
    Deccelerate(dec);
}

void BaseTank::Deccelerate(double _dec)
{
    if (_dec < dec)
    {
        SetSpeed(GetSpeed() - _dec);
    }
    else
    {
        SetSpeed(GetSpeed() - _dec);
    }
}

void BaseTank::Rotate(Angle delta_angle)
{
    if (delta_angle.GetD() > rot_speed.GetD())
        delta_angle = rot_speed;
    if (delta_angle.GetD() < -rot_speed.GetD())
        delta_angle = -rot_speed;
    Turn(delta_angle);
}

void BaseTank::RotateL()
{
    Turn(rot_speed);
}

void BaseTank::RotateR()
{
    Turn(-rot_speed);
}


//=== Tank class realization ===
Tank::Tank(double _x, double _y):BaseTank(_x,_y)
{
    path = nullptr;
    base_width = 80;
    base_length = 100;
    cannon_width = 15;
    cannon_length = 100;
    FOV_distance = 400;
    FOV_angle = Angle(30, DEGREES);
    acc = 0.3;
    dec = 0.2;
    rot_speed = Angle(2, DEGREES);
    rel_time = 40;
    max_speed = 4;
    friction = 0.1;

    //Collider
    double x_s[4] = {x + base_length/2, x + base_length/2, x - base_length/2, x - base_length/2};
    double y_s[4] = {y + base_width/2, y - base_width/2, y - base_width/2, y + base_width/2};
    collision_mask = (Collider*) new PolygonCollider(x_s, y_s, 4, x, y);

    //FOV
    double tower_x = x + cos((angle + M_PI).GetR()) * cannon_length * 0.3;
    double tower_y = y - sin((angle + M_PI).GetR()) * cannon_length * 0.3;
    FOV_collider = (Collider*) new LineCollider(tower_x, tower_y, tower_x + FOV_distance, tower_y);
}

Tank::~Tank()
{
    delete collision_mask;
    delete FOV_collider;
}

void Tank::OnStep()
{
    //Reloading
    if (reload_timeout > 0)
        reload_timeout --;
    if (reload_timeout < 0)
        reload_timeout = 0;
    //Friction
    if (speed > 0)
    {
        speed -= friction;
        if (speed < 0)
            speed = 0;
    }
    else if (speed < 0)
    {
        speed += friction;
        if (speed > 0)
            speed = 0;
    }

    //FOV
    double tower_x = x + cos((angle + M_PI).GetR()) * cannon_length * 0.3;
    double tower_y = y - sin((angle + M_PI).GetR()) * cannon_length * 0.3;
    ((Point*)FOV_collider)->MoveTo(tower_x, tower_y);
    FOV_collider->SetAngle(-(cannon_angle + angle));

    EntityStackItem* saved = stack->current;
    //Checking for collision with ray
    for (stack->Reset(); stack->current != NULL; stack->Next())
    {
        if (stack->current == saved)
            continue;
        if (FOV_collider->CheckCollision(stack->current->entity->collision_mask))
        {
            FOV_collider->collisions++;
            stack->current->entity->collision_mask->collisions++;
        }
    }

    stack->current = saved;
}

void Tank::Shoot()
{
    if (reload_timeout == 0)
    {
        double tower_x = x + cos((angle + M_PI).GetR()) * cannon_length * 0.3;
        double tower_y = y - sin((angle + M_PI).GetR()) * cannon_length * 0.3;
        Entity* bullet = new Bullet(tower_x, tower_y, cannon_angle + angle, this);
        stack->Add(bullet);
        reload_timeout = rel_time;
    }
}

void Tank::ShowTracks()
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(70, 35, 10));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    double distance = 0.5 * base_width;    //Distance between tracks
    DrawTurnedRect(&pic_pntr, x - distance * sin(angle.GetR()), y - distance * cos(angle.GetR()), angle, 1.2 * base_length, 0.2 * base_width);
    DrawTurnedRect(&pic_pntr, x + distance * sin(angle.GetR()), y + distance * cos(angle.GetR()), angle, 1.2 * base_length, 0.2 * base_width);
}

void Tank::ShowBase()
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(34, 139, 34));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    DrawTurnedRect(&pic_pntr, x, y, angle, base_length, base_width);
}

void Tank::ShowCannon()
{
    QPainter pic_pntr(picture);

    double tower_radius = cannon_length * 0.3;
    double tower_x = x + cos((angle + M_PI).GetR()) * cannon_length * 0.3;
    double tower_y = y - sin((angle + M_PI).GetR()) * cannon_length * 0.3;
    ShowFOV(&pic_pntr, tower_x, tower_y);

    double cannon_x = tower_x + cos((angle + cannon_angle).GetR()) * cannon_length * 0.5;
    double cannon_y = tower_y - sin((angle + cannon_angle).GetR()) * cannon_length * 0.5;
    QBrush Brush(QColor(0, 100, 0));

    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);
    pic_pntr.drawEllipse(tower_x - tower_radius, tower_y - tower_radius, 2 * tower_radius, 2 * tower_radius);
    DrawTurnedRect(&pic_pntr, cannon_x, cannon_y, angle + cannon_angle, cannon_length, cannon_width);
}




BaseBullet::BaseBullet(float _x, float _y, Entity* _parent):MovingEntity(_x, _y)
{
    parent = _parent;
    collision_mask = new CircleCollider(_x, _y, 5);
}

Bullet::Bullet(float _x, float _y, Angle _angle, Entity* _parent):BaseBullet(_x, _y, _parent)
{
    speed = 4;
    angle = _angle;
}

void Bullet::Show()
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(255, 0, 0));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    pic_pntr.drawEllipse(x - 5, y - 5, 10, 10);
}

void Bullet::OnStep()
{
    if ((x < -50) || (x > 2000) ||
        (y < -50) || (x > 1200))
    {
        Delete();
    }
}

QString Bullet::GetName()
{
    return "Снаряд";
}

EnemyTank::EnemyTank(double _x, double _y) : BaseTank(_x, _y)
{
    base_width = 100;
    base_length = 110;
    cannon_width = 22;
    cannon_length = 150;
    FOV_distance = 500;
    FOV_angle = Angle(20, DEGREES);
    max_speed = 5;

    acc = 0.2;
    dec = 0.15;
    rot_speed = Angle(1, DEGREES);

    double x_s[4] = {x + base_length/2, x + base_length/2, x - base_length/2, x - base_length/2};
    double y_s[4] = {y + base_width/2, y - base_width/2, y - base_width/2, y + base_width/2};
    collision_mask = new PolygonCollider(x_s, y_s, 4, x, y);

    double tower_x = x + cos((angle + M_PI).GetR()) * cannon_length * 0.35;
    double tower_y = y - sin((angle + M_PI).GetR()) * cannon_length * 0.35;
    FOV_collider = new LineCollider(tower_x, tower_y, tower_x + FOV_distance, tower_y);
}

EnemyTank::~EnemyTank()
{
//    delete collision_mask;
    delete FOV_collider;
}

void EnemyTank::Shoot()
{
    if (reload_timeout == 0)
    {
        Entity* bullet = new Bullet(x, y, cannon_angle + angle, this);
        stack->Add(bullet);
        reload_timeout = rel_time;
    }
}

void EnemyTank::OnStep()
{
    if (reload_timeout > 0)
        reload_timeout --;
    if (reload_timeout < 0)
        reload_timeout = 0;

    double tower_x = x + cos(angle.GetR() + M_PI) * cannon_length * 0.3;
    double tower_y = y - sin(angle.GetR() + M_PI) * cannon_length * 0.3;
    FOV_collider->MoveTo(tower_x, tower_y);
    FOV_collider->SetAngle(cannon_angle + angle);

    EntityStackItem* saved = stack->current;

    for (stack->Reset(); stack->current != NULL; stack->Next())
    {
        if (stack->current == saved)
            continue;
        if (FOV_collider->CheckCollision(stack->current->entity->collision_mask))
        {
            FOV_collider->collisions++;
            stack->current->entity->collision_mask->collisions++;
        }
    }

    stack->current = saved;
}

void EnemyTank::ShowTracks()
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(70, 35, 10));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    double distance = 0.5 * base_width;
    DrawTurnedRect(&pic_pntr, x - distance * sin(angle.GetR()), y - distance * cos(angle.GetR()), angle, 1.2 * base_length, 0.2 * base_width);
    DrawTurnedRect(&pic_pntr, x + distance * sin(angle.GetR()), y + distance * cos(angle.GetR()), angle, 1.2 * base_length, 0.2 * base_width);
}

void EnemyTank::ShowBase()
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(255, 229, 180));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    DrawTurnedRect(&pic_pntr, x, y, angle, base_length, base_width);
}

void EnemyTank::ShowCannon()
{
    QPainter pic_pntr(picture);

    double tower_radius = cannon_length * 0.3;
    double tower_x = x + cos((angle + M_PI).GetR()) * cannon_length * 0.3;
    double tower_y = y - sin((angle + M_PI).GetR()) * cannon_length * 0.3;
    ShowFOV(&pic_pntr, tower_x, tower_y);

    double cannon_x = tower_x + cos((angle + cannon_angle).GetR()) * cannon_length * 0.5;
    double cannon_y = tower_y - sin((angle + cannon_angle).GetR()) * cannon_length * 0.5;
    QBrush Brush(QColor(244, 164, 96));

    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);
    pic_pntr.drawEllipse(tower_x - tower_radius, tower_y - tower_radius, 2 * tower_radius, 2 * tower_radius);
    DrawTurnedRect(&pic_pntr, cannon_x, cannon_y, angle + cannon_angle, cannon_length, cannon_width);
}

QString EnemyTank::GetName()
{
    return "Вражеский танк";
}

QString Tank::GetName()
{
    return "Танк";
}





