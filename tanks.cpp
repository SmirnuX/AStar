#include "tanks.h"
#include "add_math.h"
extern EntityStack* stack;

//-------Реализация функций класса BaseTank-------

BaseTank::BaseTank(double _x, double _y):MovingEntity(_x, _y, 0, 0, nullptr)
{
    cannon_angle = 0; //Угол поворота башни
    reload_timeout = 0;   //Время, оставшееся до возможности следующего выстрел
}

void BaseTank::Show()   //Отрисовка танка
{
    ShowTracks();
    ShowBase();
    ShowCannon();
    if (SHOW_COLLIDERS)
        FOV_collider->ShowCollider();
}

void BaseTank::ShowFOV(QPainter* pic_pntr, double _x, double _y) //Отрисовка поля зрения танка
{
    QBrush Brush(QColor(255, 0, 0, 20));
    pic_pntr->setPen(QColor(0, 0, 0, 0));
    pic_pntr->setBrush(Brush);
    pic_pntr->drawPie(_x-FOV_distance, _y-FOV_distance, 2 * FOV_distance, 2 * FOV_distance, (angle + cannon_angle - FOV_angle).GetD() * 16, FOV_angle.GetD() * 32);
}

void BaseTank::SetCannonAngle(Angle _angle) //Повернуть пушку танка
{
    //Hide();
    cannon_angle = _angle;
    Show();
}


//-------Реализация функций класса Tank-------

Tank::Tank(float _x, float _y):BaseTank(_x,_y)
{
    path = NULL;

    base_width = 80;
    base_length = 100;
    cannon_width = 15;   //Ширина пушки
    cannon_length = 100;  //Длина пушки
    FOV_distance = 400;
    FOV_angle = 30;
    acc = 0.3;          //Ускорение танка
    dec = 0.2;          //Торможение танка
    rot_speed = 2;    //Скорость поворота
    rel_time = 40;
    max_speed = 4;

    //Столкновения
    double x_s[4] = {x + base_length/2, x + base_length/2, x - base_length/2, x - base_length/2};
    double y_s[4] = {y + base_width/2, y - base_width/2, y - base_width/2, y + base_width/2};
    collision_mask = (Collider*) new PolygonCollider(x_s, y_s, 4, x, y);

    //Зрение
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
    //Перезарядка
    if (reload_timeout > 0)
        reload_timeout --;
    if (reload_timeout < 0)
        reload_timeout = 0;
    //Торможение
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

    //Зрение
    double tower_x = x + cos((angle + Angle(M_PI)).GetR()) * cannon_length * 0.3;
    double tower_y = y - sin((angle + Angle(M_PI)).GetR()) * cannon_length * 0.3;
     ((Point*)FOV_collider)->MoveTo(tower_x, tower_y);
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

void Tank::Shoot()
{
    if (reload_timeout == 0)
    {
        Entity* bullet = new Bullet(x, y, cannon_angle + angle, this);
        stack->Add(bullet);
        reload_timeout = rel_time;    //TODO - убрать магическое число
    }
}

void Tank::ShowTracks()  //Отрисовка гусениц танка
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(70, 35, 10));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    double distance = 0.5 * base_width;    //Расстояние между гусеницами
    DrawTurnedRect(&pic_pntr, x - distance * sin(angle.GetR()), y - distance * cos(angle.GetR()), angle, 1.2 * base_length, 0.2 * base_width);
    DrawTurnedRect(&pic_pntr, x + distance * sin(angle.GetR()), y + distance * cos(angle.GetR()), angle, 1.2 * base_length, 0.2 * base_width);
}

void Tank::ShowBase()  //Отрисовка корпуса танка
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(34, 139, 34));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    DrawTurnedRect(&pic_pntr, x, y, angle, base_length, base_width);
}

void Tank::ShowCannon() //Отрисовка пушки танка
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
    if (x < -50 || x >  2000 || y < -50 || x > 1200)
    {
        stack->Delete(this);
    }
}

QString Bullet::GetName()
{
    return QString::fromLocal8Bit("Снаряд");
}

EnemyTank::EnemyTank(float _x, float _y) : BaseTank(_x, _y)
{
    base_width = 100;
    base_length = 110;
    cannon_width = 22;   //Ширина пушки
    cannon_length = 150;  //Длина пушки
    FOV_distance = 500;
    FOV_angle = 20;
    max_speed = 5;

    acc = 0.2;         //Ускорение танка
    dec = 0.15;          //Торможение танка
    rot_speed = 1;    //Скорость поворота

    //Столкновения
    double x_s[4] = {x + base_length/2, x + base_length/2, x - base_length/2, x - base_length/2};
    double y_s[4] = {y + base_width/2, y - base_width/2, y - base_width/2, y + base_width/2};
    collision_mask = new PolygonCollider(x_s, y_s, 4, x, y);

    //Зрение
    double tower_x = x + cos(degtorad((angle + M_PI).GetR())) * cannon_length * 0.35;
    double tower_y = y - sin(degtorad((angle + M_PI).GetR())) * cannon_length * 0.35;
    FOV_collider = new LineCollider(tower_x, tower_y, tower_x + FOV_distance, tower_y);
}

EnemyTank::~EnemyTank()
{
    delete collision_mask;
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

    //Зрение
    double tower_x = x + cos(angle.GetR() + M_PI) * cannon_length * 0.3;
    double tower_y = y - sin(angle.GetR() + M_PI) * cannon_length * 0.3;
    FOV_collider->MoveTo(tower_x, tower_y);
    FOV_collider->SetAngle((cannon_angle + angle).GetR());

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

void EnemyTank::ShowTracks()  //Отрисовка гусениц танка
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(70, 35, 10));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    double distance = 0.5 * base_width;    //Расстояние между гусеницами
    DrawTurnedRect(&pic_pntr, x - distance * sin(angle.GetR()), y - distance * cos(angle.GetR()), angle, 1.2 * base_length, 0.2 * base_width);
    DrawTurnedRect(&pic_pntr, x + distance * sin(angle.GetR()), y + distance * cos(angle.GetR()), angle, 1.2 * base_length, 0.2 * base_width);
}

void EnemyTank::ShowBase()  //Отрисовка корпуса танка
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(255, 229, 180));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    DrawTurnedRect(&pic_pntr, x, y, angle, base_length, base_width);
}

void EnemyTank::ShowCannon()  //Отрисовка пушки танка
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
    return QString::fromLocal8Bit("Вражеский танк");
}

void Tank::RideTo(Box* obstacle) //ТЕСТ обьезд квдарата
{
    double acc = 0.3;
    double delta_angle = 2;
    double threshold = sqrt(base_width*base_width + base_length*base_length)/2 + 10;   //Запас для объезда
    double tx, ty;  //Целевая точка
    if (obstacle->GetX() < x - base_length/2 ||                             //Если препятствие уже позади
        obstacle->GetY() + obstacle->a < y - threshold ||    //Если прямоугольник ниже
        obstacle->GetY() - obstacle->a > y + threshold  )    //Если прямоугольник выше
    {
        if (angle != Angle(0))
        {
            if (angle < delta_angle || angle > 360-delta_angle)
                SetAngle(0);
            else
                Turn((angle < 180)?-delta_angle:delta_angle);
        }

        SetSpeed(speed + acc);  //Ускорение
        return;
    }
    tx = obstacle->GetX()-obstacle->a;
    if (obstacle->GetY() > y)   //Объезд по нижней кромке
        ty = obstacle->GetY() + obstacle->a + threshold;
    else    //По верхней
        ty = obstacle->GetY() - obstacle->a - threshold;

    double front_x = x + threshold + (speed + acc) * cos(angle.GetR());
    if (front_x > tx)  //Торможение
        SetSpeed(speed - acc);
    else
        SetSpeed(speed + acc);  //Ускорение


    front_x = x + threshold + (speed + acc) * cos(angle.GetR());
    double ta;  //Целевой угол
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


void Tank::BuildPath(double tx, double ty)  //Построение ПРОСТОГО пути из точки x, y к tx, ty
{
    if (path != NULL)
        delete path;
    int MAX_POINTS = 600;    //Количество точек - равно кадрам, за которое доедет до цели
    path = new Path(MAX_POINTS, tx, ty);
    double threshold = sqrt(base_width*base_width + base_length*base_length)/2 + 10;
    double path_x = x;
    double path_y = y;
    double path_angle = -angle.GetD();
    double friction = 0.1;
    if (path_angle > 180)
        path_angle -= 360;
    double path_speed = speed;
    double cosinus = cos(degtorad(path_angle));
    double sinus = sin(degtorad(path_angle));
    for (int i = 0; i < MAX_POINTS; i++)
    {
        double ideal_angle = radtodeg(atan( (ty-path_y)/(tx-path_x) ));
        if (!almostEq(angle.GetD(), ideal_angle))  //Поворот
        {
            if (fabs(anglediff(ideal_angle, path_angle)) < rot_speed)
                path_angle = ideal_angle;
            else if (anglediff(ideal_angle, path_angle) > 0)
                path_angle += rot_speed;
            else
                path_angle -= rot_speed;
            cosinus = cos(degtorad(path_angle));
            sinus = sin(degtorad(path_angle));
        }
        //Расстояния до цели в трех случаях
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

        //Торможение
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

void Tank::ShowPath()
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
    /* ПЕРЕСТРОЙКА ПУТИ
    if (!almostEq(x, path->x[path->i], 2) || !almostEq(x, path->x[path->i], 2))
        BuildPath(path->final_x, path->final_y);*/
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
    return QString::fromLocal8Bit("Танк");
}

void Tank::BuildPath(double tx, double ty, Box* obstacle)    //ТЕСТ Обьезд коробки
{
    if (path != NULL)
        delete path;
    int MAX_POINTS = 600;    //Количество точек - равно кадрам, за которое доедет до цели (10 сек)
    path = new Path(MAX_POINTS, tx, ty);
    double threshold = sqrt(base_width*base_width + base_length*base_length)/2 + 10;
    double side = 2 * obstacle->a * 0.8;    //Примерная диагональ квадрата
    double path_x = x;
    double path_y = y;
    double path_angle = -angle.GetD();
    double friction = 0.1;
    if (path_angle > 180)
        path_angle -= 360;
    double path_speed = speed;
    double cosinus = cos(degtorad(path_angle));
    double sinus = sin(degtorad(path_angle));
    double temp_x = tx;
    double temp_y = ty; //Предварительные целевые точки
    for (int i = 0; i < MAX_POINTS; i++)
    {
        //Предварительный поиск следующей точки

        //Стоит ли на пути препятствие
        //ВРЕМЕННО
        CircleCollider circ(obstacle->GetX(), obstacle->GetY(), side + threshold);
        LineCollider line(path_x, path_y, tx, ty);
        if (line.CheckCollision(&circ))
        {
            double normal_x = -(ty - path_y);   //Нормаль
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
        double ideal_angle = radtodeg(atan( (temp_y-path_y)/(temp_x-path_x) )); //Направление к целевой точке
        if (!almostEq(path_angle, ideal_angle))  //Поворот
        {
            if (fabs(anglediff(ideal_angle, path_angle)) < rot_speed)
                path_angle = ideal_angle;
            else if (anglediff(ideal_angle, path_angle) > 0)
                path_angle += rot_speed;
            else
                path_angle -= rot_speed;
            cosinus = cos(degtorad(path_angle));
            sinus = sin(degtorad(path_angle));
        }

        //Расстояния для препятствия в трех случаях
        /*
        double acc_obst_dist = distance2(path_x + (path_speed + acc - friction)*cosinus, path_y + (path_speed + acc - friction) * sinus, obstacle->GetX(), obstacle->GetY());
        double dec_obst_dist = distance2(path_x + (path_speed - dec + friction) *cosinus, path_y + (path_speed - dec + friction) * sinus, obstacle->GetX(), obstacle->GetY());
        double nop_obst_dist = distance2(path_x + (path_speed - friction)*cosinus, path_y + (path_speed - friction) * sinus, obstacle->GetX(), obstacle->GetY());
        */
        //Поиск вектора, перпендикулярного скорости езды


        //Расстояния до цели в трех случаях
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

        //Торможение
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





