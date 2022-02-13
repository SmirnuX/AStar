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
    path = NULL;

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
    double friction = 0.1;
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
        Entity* bullet = new Bullet(x, y, cannon_angle + angle, this);
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

void Tank::RideTo(Box* obstacle) //Test
{
    double acc = 0.3;
    double delta_angle = 2;
    double threshold = sqrt(base_width*base_width + base_length*base_length)/2 + 10;
    double tx, ty;  //Целевая точка
    if (obstacle->GetX() < x - base_length/2 ||
        obstacle->GetY() + obstacle->a < y - threshold ||
        obstacle->GetY() - obstacle->a > y + threshold  )
    {
        if (angle != Angle(0))
        {
            if ((angle < delta_angle) || (angle > 360-delta_angle))
                SetAngle(0);
            else
                Turn((angle < 180)?-delta_angle:delta_angle);
        }

        SetSpeed(speed + acc);
        return;
    }
    tx = obstacle->GetX()-obstacle->a;
    if (obstacle->GetY() > y)
        ty = obstacle->GetY() + obstacle->a + threshold;
    else
        ty = obstacle->GetY() - obstacle->a - threshold;

    double front_x = x + threshold + (speed + acc) * cos(angle.GetR());
    if (front_x > tx)
        SetSpeed(speed - acc);
    else
        SetSpeed(speed + acc);


    front_x = x + threshold + (speed + acc) * cos(angle.GetR());
    double ta;
    if (fabs(tx - front_x) < threshold)
        ta = (ty-y) > 0 ? 90 : -90;
    else
        ta = radtodeg( atan((ty-y)/(tx-front_x)));
    if (angle != ta)
    {
        double temp_a = angle.GetD();
        if (temp_a > 180)
            temp_a -= 360;
        if (fabs(ta - temp_a) < delta_angle)
            SetAngle(ta);
        else
            Turn(((ta - temp_a) > 0)?delta_angle:-delta_angle);
    }

}


void Tank::BuildPath(double tx, double ty)  //Simple path
{
    if (path != NULL)
        delete path;
    int MAX_POINTS = 600;
    path = new Path(MAX_POINTS, tx, ty);
    double threshold = sqrt(base_width*base_width + base_length*base_length)/2 + 10;
    double path_x = x;
    double path_y = y;
    Angle path_angle = -angle;
    double friction = 0.1;
    if (path_angle > 180)
        path_angle -= 360;
    double path_speed = speed;
    double cosinus = cos(path_angle.GetR());
    double sinus = sin(path_angle.GetR());
    for (int i = 0; i < MAX_POINTS; i++)
    {
        double ideal_angle = radtodeg(atan( (ty-path_y)/(tx-path_x) ));
        if (!almostEq(angle.GetD(), ideal_angle))
        {
            if (fabs(anglediff(ideal_angle, path_angle)) < rot_speed)
                path_angle = ideal_angle;
            else if (anglediff(ideal_angle, path_angle) > 0)
                path_angle += rot_speed;
            else
                path_angle -= rot_speed;
            cosinus = cos(path_angle.GetR());
            sinus = sin(path_angle.GetR());
        }
        double acc_dist = distance2(path_x + (path_speed + acc - friction)*cosinus, path_y + (path_speed + acc - friction)* sinus, tx, ty);
        double dec_dist = distance2(path_x + (path_speed - dec + friction)*cosinus, path_y + (path_speed - dec + friction) * sinus, tx, ty);
        double nop_dist = distance2(path_x + (path_speed - friction)*cosinus, path_y + (path_speed - friction)* sinus, tx, ty);

        if (acc_dist < dec_dist && acc_dist < nop_dist)
            path_speed += acc;
        else if (dec_dist < nop_dist)
            path_speed -= dec;
        if (path_speed > max_speed)
            path_speed = max_speed;
        else if (path_speed < -max_speed)
            path_speed = -max_speed;
        path->a[i] = path_angle;
        path->s[i] = path_speed;

        if (path_speed > 0)
        {
            path_speed -= friction;
            if (path_speed < 0)
                path_speed = 0;
        }
        else if (path_speed < 0)
        {
            path_speed += friction;
            if (path_speed > 0)
                path_speed = 0;
        }

        path_x += path_speed*cosinus;
        path_y += path_speed*sinus;
        path->x[i] = path_x;
        path->y[i] = path_y;
    }

}

void Tank::ShowPath()   //Drawing path
{
    if (path == NULL)
        return;
    QPainter pntr(picture);
    QPen penn;
    penn.setWidth(3);
    bool color = false;
    for (int i = 0; i < path->num - 1; i++)
    {
        if (color)
        {
            double br = 255 * (fabs(path->s[i]) / max_speed);
            penn.setBrush(QBrush(QColor(br, 255-br, 0)));
            pntr.setPen(penn);
        }
        pntr.drawLine(path->x[i], path->y[i], path->x[i+1], path->y[i+1]);
    }
}

void Tank::FollowPath()
{
    if (path == NULL)
        return;
    if (path->s[path->i] > speed)
        Accelerate(path->s[path->i] - speed);
    else
        Deccelerate(speed - path->s[path->i]);
    if (fabs(anglediff(- path->a[path->i], angle.GetD())) < rot_speed)
        SetAngle(- path->a[path->i]);
    else if (anglediff(- path->a[path->i], angle.GetD()) > 0)
        Turn(rot_speed);
    else
        Turn(-rot_speed);
    path->i++;
    if (path->i == path->num || (almostEq(x, path->final_x, 2) && almostEq(y, path->final_y, 2)))
    {
        delete path;
        path = NULL;
    }
}

QString Tank::GetName()
{
    return "Танк";
}

void Tank::BuildPath(double tx, double ty, Box* obstacle)
{
    if (path != NULL)
        delete path;
    int MAX_POINTS = 600;
    path = new Path(MAX_POINTS, tx, ty);
    double threshold = sqrt(base_width*base_width + base_length*base_length)/2 + 10;
    double side = 2 * obstacle->a * 0.8;
    double path_x = x;
    double path_y = y;
    Angle path_angle = -angle;
    double friction = 0.1;
    if (path_angle > 180)
        path_angle -= 360;
    double path_speed = speed;
    double cosinus = cos(path_angle.GetR());
    double sinus = sin(path_angle.GetR());
    double temp_x = tx;
    double temp_y = ty;
    for (int i = 0; i < MAX_POINTS; i++)
    {
        CircleCollider circ(obstacle->GetX(), obstacle->GetY(), side + threshold);
        LineCollider line(path_x, path_y, tx, ty);
        if (line.CheckCollision(&circ))
        {
            double normal_x = -(ty - path_y);
            double normal_y = tx - path_x;
            double dist = sqrt(distance2(tx, ty, path_x, path_y));
            normal_x /= dist;
            normal_y /= dist;
            normal_x *= (side + threshold);
            normal_y *= (side + threshold);
            temp_x = obstacle->GetX() + normal_x;
            temp_y = obstacle->GetY() + normal_y;
        }
        else
        {
            temp_x = tx;
            temp_y = ty;
        }
        double ideal_angle = radtodeg(atan( (temp_y-path_y)/(temp_x-path_x) ));
        if (!almostEq(path_angle.GetD(), ideal_angle))
        {
            if (fabs(anglediff(ideal_angle, path_angle)) < rot_speed)
                path_angle = ideal_angle;
            else if (anglediff(ideal_angle, path_angle) > 0)
                path_angle += rot_speed;
            else
                path_angle -= rot_speed;
            cosinus = cos(path_angle.GetR());
            sinus = sin(path_angle.GetR());
        }
        double acc_dist = distance2(path_x + (path_speed + acc - friction)*cosinus, path_y + (path_speed + acc - friction)* sinus, temp_x, temp_y);
        double dec_dist = distance2(path_x + (path_speed - dec + friction)*cosinus, path_y + (path_speed - dec + friction) * sinus, temp_x, temp_y);
        double nop_dist = distance2(path_x + (path_speed - friction)*cosinus, path_y + (path_speed - friction)* sinus, temp_x, temp_y);

        if (acc_dist < dec_dist && acc_dist < nop_dist)
            path_speed += acc;
        else if (dec_dist < nop_dist)
            path_speed -= dec;
        if (path_speed > max_speed)
            path_speed = max_speed;
        else if (path_speed < -max_speed)
            path_speed = -max_speed;
        path->a[i] = path_angle;
        path->s[i] = path_speed;

        if (path_speed > 0)
        {
            path_speed -= friction;
            if (path_speed < 0)
                path_speed = 0;
        }
        else if (path_speed < 0)
        {
            path_speed += friction;
            if (path_speed > 0)
                path_speed = 0;
        }

        path_x += path_speed*cosinus;
        path_y += path_speed*sinus;
        path->x[i] = path_x;
        path->y[i] = path_y;
    }
}





