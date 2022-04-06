#include "cars.h"
#include "add_math.h"
extern EntityStack* stack;

//=== BaseCar class realization ===

BaseCar::BaseCar(double _x, double _y):MovingEntity(_x, _y, 0, 0, nullptr)
{
    target_ui_angle = angle;
    friction = 0.2;
    directed_friction = true;

    //Flags and timers
    is_acc = false;
    is_dec = false;
    is_turn = false;
}

void BaseCar::ResetFlags()
{
    is_acc = false;
    is_dec = false;
    is_turn = false;
}

void BaseCar::Show()   //Drawing
{
    ShowRadar();
    ShowWheels();
    ShowBase();
    ShowLights();
}

void BaseCar::ShowFOV(QPainter* pic_pntr, double _x, double _y) //Drawing FOV
{
    QBrush Brush(QColor(255, 0, 0, 10));
    pic_pntr->setPen(QColor(0, 0, 0, 0));
    pic_pntr->setBrush(Brush);
    pic_pntr->drawPie(_x-FOV_distance, _y-FOV_distance, 2 * FOV_distance, 2 * FOV_distance, (angle - FOV_angle).GetD() * 16, FOV_angle.GetD() * 32);
}

void BaseCar::Accelerate()
{
    Accelerate(acc);
}

void BaseCar::Accelerate(double _acc)
{
    if (acc <= 0)
        return;
    if (_acc < acc)
    {
        ChangeSpeed(_acc, -angle);
    }
    else
    {
        ChangeSpeed(acc, -angle);
    }
    is_acc = true;
}

void BaseCar::Deccelerate()
{
    Deccelerate(dec);
}

void BaseCar::Deccelerate(double _dec)
{
    if (_dec <= 0)
        return;
    if (_dec > dec)
    {
        _dec = dec;
    }
    if (speed - _dec < 0)
        SetSpeed(0);
    else
        ChangeSpeed(-_dec, Angle(direction_to_point(0,0,speed_x,speed_y)));

    is_dec = true;
}

void BaseCar::Rotate(Angle delta_angle)
{
    if (delta_angle.GetD() > rot_speed.GetD())
        delta_angle = rot_speed;
    if (delta_angle.GetD() < -rot_speed.GetD())
        delta_angle = -rot_speed;
    Turn(delta_angle);
    is_turn = true;
    wheels_angle = delta_angle;
}

void BaseCar::RotateTo(Angle target_angle, DIRECTION dir)   //Rotating to specified angle
{
    if (dir == NODIRECTION)
    {
        //Selecting nearest direction
        if (angle < target_angle)
            dir = COUNTERCLOCKWISE;
        else
            dir = CLOCKWISE;
    }
    if (dir == COUNTERCLOCKWISE)
    {
        if ((target_angle - angle).GetD() < rot_speed.GetD())
            RotateL(target_angle - angle);
        else
            RotateL();
    }
    else
    {
        if ((angle - target_angle).GetD() < rot_speed.GetD())
            RotateR(angle - target_angle);
        else
            RotateR();
    }
}

void BaseCar::RotateL()
{
    Turn(rot_speed);
    is_turn = true;
    wheels_angle = rot_speed;
}

void BaseCar::RotateR()
{
    Turn(-rot_speed);
    is_turn = true;
    wheels_angle = -rot_speed;
}

void BaseCar::RotateL(const Angle& delta_angle)
{
    Turn(delta_angle);
    is_turn = true;
    wheels_angle = delta_angle;
}

void BaseCar::RotateR(const Angle& delta_angle)
{
    Turn(-delta_angle);
    is_turn = true;
    wheels_angle = -delta_angle;
}


//=== Car class realization ===
Car::Car(double _x, double _y):BaseCar(_x,_y)
{
    path = nullptr;
    //FOV settings
    FOV_distance = 400;
    FOV_angle = Angle(30, DEGREES);

    //Speed settings
    acc = 0.3;
    dec = 0.2;
    rot_speed = Angle(2, DEGREES);
    max_speed = 8;
    friction = 0.1;

    //Size parameters

    base_length = 120;
    base_width = 60;
    wheels_x_delta = 50;
    wheels_y_delta = 27;
    wheels_length = 15;
    wheels_width = 8;
    lights_length = 10;
    lights_width = 6;

    //Collider
    double x_s[4] = {x + base_length/2, x + base_length/2, x - base_length/2, x - base_length/2};
    double y_s[4] = {y + base_width/2, y - base_width/2, y - base_width/2, y + base_width/2};
    collision_mask = (Collider*) new PolygonCollider(x_s, y_s, 4, x, y);

    ray_count = 9;

    //Radar
    FOV_collider = new LineCollider*[ray_count];
    double one_ang = FOV_angle.GetR() / (ray_count-1);
    FOV_points = new RadarPoint[ray_count];
    for (int i = 0; i < ray_count; i++)
    {
        FOV_points[i] = RadarPoint();
        Angle ang = -FOV_angle.GetR() / 2 + one_ang * i;
        FOV_collider[i] = new LineCollider(x, y, x + FOV_distance * sin(ang.GetR()), y + FOV_distance * cos(ang.GetR()));
    }



    radius = distance(0,0,base_width/2,base_length/2);
}

Car::~Car()
{
    delete collision_mask;
    delete FOV_collider;
    if (path != nullptr)
        delete path;
}

void Car::OnStep()
{
    //FOV

    double one_ang = FOV_angle.GetR() / (ray_count-1);
    for (int i = 0; i < ray_count; i++)
    {
        ((Point*)FOV_collider[i])->MoveTo(x, y);
        Angle ang = -FOV_angle.GetR() / 2 + one_ang * i;
        FOV_collider[i]->SetAngle(-(angle) + ang);
    }


    EntityStackItem* saved = stack->current;
    //Checking for collision with ray
    for (int i = 0; i < ray_count; i++)
        FOV_points[i].distance = -1;

    for (stack->Reset(); stack->current != NULL; stack->Next())
    {
        if (stack->current->entity == this)
            continue;
        for (int i = 0; i < ray_count; i++)
        {
            RadarPoint curr = stack->current->entity->collision_mask->Raycast((Point*) this, Angle(-FOV_angle.GetR() / 2 + one_ang * i) - angle, FOV_distance);

            if ((curr.distance < FOV_points[i].distance && curr.distance >= 0) || (curr.distance > FOV_points[i].distance && FOV_points[i].distance < 0))
            {
                FOV_points[i] = curr;
            }
        }
    }
    stack->current = saved; //Returning to previous state of stack
}

void Car::ShowRadar()
{
    QPainter pic_pntr(picture);
    double one_ang = FOV_angle.GetR() / (ray_count-1);
    for (int i = 0; i < ray_count; i++)
    {
        Angle ang = -FOV_angle.GetR() / 2 + one_ang * i - angle;
        if (FOV_points[i].distance != -1)
        {
            pic_pntr.setPen(QColor(255,0,0));
            pic_pntr.drawLine(x, y, x + FOV_points[i].distance * cos(ang.GetR()), y + FOV_points[i].distance * sin(ang.GetR()));
            pic_pntr.drawEllipse(x + FOV_points[i].distance * cos(ang.GetR()) - 2,
                                 y + FOV_points[i].distance * sin(ang.GetR()) - 2, 4, 4);
        }
        else    //No collision
        {
            pic_pntr.setPen(QColor(140,140,140,40));
            pic_pntr.drawLine(x, y, x + FOV_distance * cos(ang.GetR()), y + FOV_distance * sin(ang.GetR()));
        }
    }

    QPen pen(QColor(255,0,255));
    pen.setWidth(3);

    pic_pntr.setPen(pen);

    for (int i = 1; i < ray_count; i++)
    {
        Angle ang_pr = -FOV_angle.GetR() / 2 + one_ang * i - one_ang - angle;
        Angle ang = -FOV_angle.GetR() / 2 + one_ang * i - angle;
        if (FOV_points[i].distance != -1 && FOV_points[i-1].distance != -1)
        {

            pic_pntr.drawLine(x + FOV_points[i-1].distance * cos(ang_pr.GetR()), y + FOV_points[i-1].distance * sin(ang_pr.GetR()),
                              x + FOV_points[i].distance * cos(ang.GetR()), y + FOV_points[i].distance * sin(ang.GetR()));
            pic_pntr.drawEllipse(x + FOV_points[i].distance * cos(ang.GetR()) - 2,
                                 y + FOV_points[i].distance * sin(ang.GetR()) - 2, 4, 4);
        }
    }
}

void Car::ShowWheels()
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(60, 60, 60));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    Angle real_wheel_angle = is_turn ? Angle(angle.GetR() + wheels_angle.GetR() * 6) : angle;

    //Front wheels
    DrawTurnedRect(&pic_pntr,
                   x + wheels_x_delta * cos(-angle.GetR()) - wheels_y_delta * sin(-angle.GetR()),
                   y + wheels_x_delta * sin(-angle.GetR()) + wheels_y_delta * cos(-angle.GetR()),
                   real_wheel_angle, wheels_length, wheels_width);
    DrawTurnedRect(&pic_pntr,
                   x + wheels_x_delta * cos(-angle.GetR()) + wheels_y_delta * sin(-angle.GetR()),
                   y + wheels_x_delta * sin(-angle.GetR()) - wheels_y_delta * cos(-angle.GetR()),
                   real_wheel_angle, wheels_length, wheels_width);

    //Rear wheels
    DrawTurnedRect(&pic_pntr,
                   x - wheels_x_delta * cos(-angle.GetR()) - wheels_y_delta * sin(-angle.GetR()),
                   y - wheels_x_delta * sin(-angle.GetR()) + wheels_y_delta * cos(-angle.GetR()),
                   angle, wheels_length, wheels_width);
    DrawTurnedRect(&pic_pntr,
                   x - wheels_x_delta * cos(-angle.GetR()) + wheels_y_delta * sin(-angle.GetR()),
                   y - wheels_x_delta * sin(-angle.GetR()) - wheels_y_delta * cos(-angle.GetR()),
                   angle, wheels_length, wheels_width);
}

void Car::ShowBase()
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(160, 160, 160));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    DrawTurnedRect(&pic_pntr, x, y, angle, base_length, base_width);


    QBrush WBrush(QColor(60, 60, 60));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(WBrush);
    DrawTurnedRect(&pic_pntr, x, y, angle, base_length/2, base_width * 0.8);

    pic_pntr.setBrush(Brush);
    DrawTurnedRect(&pic_pntr, x, y, angle, base_length*0.4, base_width * 0.8);

    //Drawing windows

}

void Car::ShowLights()
{
    QPainter pic_pntr(picture);
    QBrush Brush(QColor(240, 240, 200));
    pic_pntr.setPen(QColor(0, 0, 0));
    pic_pntr.setBrush(Brush);

    double lights_y_delta = base_width/2 - lights_width/2;
    double lights_x_delta = base_length/2 - lights_length/2;
    double rear_lights_y_delta = lights_y_delta - lights_length/2;

    //Front lights
    DrawTurnedRect(&pic_pntr,
                   x + lights_x_delta * cos(-angle.GetR()) - lights_y_delta * sin(-angle.GetR()),
                   y + lights_x_delta * sin(-angle.GetR()) + lights_y_delta * cos(-angle.GetR()),
                   angle, lights_length, lights_width);
    DrawTurnedRect(&pic_pntr,
                   x + lights_x_delta * cos(-angle.GetR()) + lights_y_delta * sin(-angle.GetR()),
                   y + lights_x_delta * sin(-angle.GetR()) - lights_y_delta * cos(-angle.GetR()),
                   angle, lights_length, lights_width);

    QBrush StopBrush = (is_dec && speed > 0) ? QBrush(QColor(255,0,0)) : QBrush(QColor(100,0,0));
    pic_pntr.setBrush(StopBrush);

    //Rear stop-lights
    DrawTurnedRect(&pic_pntr,
                   x - lights_x_delta * cos(-angle.GetR()) - lights_y_delta * sin(-angle.GetR()),
                   y - lights_x_delta * sin(-angle.GetR()) + lights_y_delta * cos(-angle.GetR()),
                   angle, lights_length, lights_width);
    DrawTurnedRect(&pic_pntr,
                   x - lights_x_delta * cos(-angle.GetR()) + lights_y_delta * sin(-angle.GetR()),
                   y - lights_x_delta * sin(-angle.GetR()) - lights_y_delta * cos(-angle.GetR()),
                   angle, lights_length, lights_width);

    QBrush RearBrush = (is_dec && speed < 0) ? QBrush(QColor(255,255,255)) : QBrush(QColor(100,100,100));
    pic_pntr.setBrush(RearBrush);

    //Rear backward-lights
    DrawTurnedRect(&pic_pntr,
                   x - lights_x_delta * cos(-angle.GetR()) - rear_lights_y_delta * sin(-angle.GetR()),
                   y - lights_x_delta * sin(-angle.GetR()) + rear_lights_y_delta * cos(-angle.GetR()),
                   angle, lights_length, lights_width);
    DrawTurnedRect(&pic_pntr,
                   x - lights_x_delta * cos(-angle.GetR()) + rear_lights_y_delta * sin(-angle.GetR()),
                   y - lights_x_delta * sin(-angle.GetR()) - rear_lights_y_delta * cos(-angle.GetR()),
                   angle, lights_length, lights_width);
}

QString Car::GetName()
{
    return "Машина";
}





