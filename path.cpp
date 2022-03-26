#include "cars.h"
extern EntityStack* stack;

std::vector<edge> get_path_from_graph(graph *gr)  //Getting path from graph
{
    vertex* start = gr->vertices[gr->start];
    vertex* end = gr->vertices[gr->end];
    vertex* curr = start;
    vertex* prev = nullptr;

    std::vector<edge> result;
    while (curr != end)
    {
        for (uint i = 0; i < gr->edges.size(); i++)
        {
            vertex* adj;    //Adjacent vertex
            DIRECTION dir = COUNTERCLOCKWISE;   //Direction of movement - aA is before aB
            if (gr->edges[i].pA == curr)
            {
                adj = gr->edges[i].pB;
            }
            else if (gr->edges[i].pB == curr)
            {
                dir = CLOCKWISE; //Reverse direction
                adj = gr->edges[i].pA;
            }
            else
                continue;
            if (adj == prev)    //If there is cycle - skip
                continue;
            if (gr->edges[i].chosen)    //If this edge is marked as path
            {
                edge temp_edge;
                temp_edge.A.MoveTo(curr->point->GetX(), curr->point->GetY());
                temp_edge.B.MoveTo(adj->point->GetX(), adj->point->GetY());
                if (gr->edges[i].type == LINEAR)
                {
                    temp_edge.type = LINEAR;
                }
                else
                {
                    if (gr->edges[i].pA == curr)    //Start and end angles
                    {
                        temp_edge.aA = gr->edges[i].pA->angle;
                        temp_edge.aB = gr->edges[i].pB->angle;
                    }
                    else
                    {
                        temp_edge.aB = gr->edges[i].pA->angle;
                        temp_edge.aA = gr->edges[i].pB->angle;
                    }
                    temp_edge.type = ARC_CIRCLE;
                    temp_edge.direction = dir;
                    temp_edge.r = gr->edges[i].r;
                    temp_edge.cx = gr->edges[i].cx;
                    temp_edge.cy = gr->edges[i].cy;
                }
                temp_edge.length = gr->edges[i].length;
                prev = curr;
                curr = adj;
                result.push_back(temp_edge);
                break;
            }
        }
    }
    return result;
}

void Car::graph_to_path(graph* gr, uint target) //Getting path with physical properties
{
    if (gr == nullptr)
        return;
    if (!gr->IsWay())   //If there is no way found
        return;
    if (path != nullptr)   //Deleting old path
        delete path;

    //1. Searching for path in graph
    std::vector<edge> temp_path = get_path_from_graph(gr);

    //2. Creating path
    uint segment_count = temp_path.size();   //Number of segments
    path = new Path(segment_count, gr->vertices[target]->point->GetX(), gr->vertices[target]->point->GetY());
    path->startx = x;
    path->starty = y;

    //3. Calculating max speed for every segment
    double* temp_speeds = new double[segment_count];
    temp_speeds[segment_count-1] = 0;    //Object have to stop in last point
    for (int i = segment_count-2; i >= 0; i--)
    {
        double length = temp_path[i].length;
        //Calculating max speed for this segment
        double max_spd;
        if (i == 0) //Start speed
            max_spd = temp_speeds[1];
        else
        {
            if (temp_path[i].type == ARC_CIRCLE)
            {
                max_spd = 2 * temp_path[i].r * sin(rot_speed.GetR()/2); //V = 2 * R * sin (max_rot / 2) - max speed you can stay on arc with
            }
            else if (temp_path[i+1].type == ARC_CIRCLE)
            {
                max_spd = 2 * temp_path[i+1].r * sin(rot_speed.GetR()/2);
            }
            else
                max_spd = max_speed;    //Should never happen
            if (max_spd > max_speed)
                max_spd = max_speed;
        }

        //Check if there is possibility to achieve that speed
        double acc_path;
        if (max_spd > temp_speeds[i+1]) //If next speed lower then current
            acc_path = (max_spd * max_spd - temp_speeds[i+1] * temp_speeds[i+1]) / (2 * (dec + friction));
        else
            acc_path = (temp_speeds[i+1] * temp_speeds[i+1] - max_spd * max_spd) / (2 * (acc - friction));
        if (acc_path > length)  //If object can't break/accelerate enough
        {
            if (max_spd > temp_speeds[i+1]) //Changing speed to achievable
                max_spd = sqrt(fabs(temp_speeds[i+1] * temp_speeds[i+1] - 2*(dec + friction) * length));  //V0 = sqrt (V^2 - 2aS)
            else
                max_spd = sqrt(fabs(temp_speeds[i+1] * temp_speeds[i+1] - 2*(acc - friction) * length));
        }
        temp_speeds[i] = max_spd;
        qDebug() << "MAX_SPEED [" << i << "] = " << temp_speeds[i];
    }

    //3. Path building

    for (int i = 0; i < segment_count; i++)
    {
        if (temp_path[i].type == ARC_CIRCLE)
        {
            path->pts[i].c_x = temp_path[i].cx;
            path->pts[i].c_y = temp_path[i].cy;
            path->pts[i].c_r = temp_path[i].r;
            path->pts[i].s_a = temp_path[i].aA;
            path->pts[i].e_a = temp_path[i].aB;
//            qDebug() << path->pts[i].s_a.GetD() << " " << path->pts[i].e_a.GetD();
        }
        path->pts[i].circle = temp_path[i].type == ARC_CIRCLE;
        path->pts[i].x = temp_path[i].B.GetX();
        path->pts[i].y = temp_path[i].B.GetY();
        path->pts[i].s = temp_speeds[i];
        path->pts[i].direction = temp_path[i].direction;
    }
    path->i = 0;

    delete[] temp_speeds;
}

void Car::FollowPath() //Actually follow built path
{
    //[!] Coords of car are inverted, coords of path - not [!]
    if (path == NULL)
        return;
    if (path->i >= path->num)    //If path completed
        return;
    int i = path->i;
    pathpoint* next_pt = path->pts + i;
    if (path->pts[i].circle)    //If riding a circle
    {
        Angle dir_to_obj(direction_to_point(next_pt->c_x, next_pt->c_y, x, y)); //Angle between circle and object
        double temp_max_speed = next_pt->s;
        double delta;   //Difference between normal angle and current
        if (next_pt->direction == COUNTERCLOCKWISE)
            delta = fabs(anglediff(-angle, dir_to_obj.normalL()));
        else
            delta = fabs(anglediff(-angle, dir_to_obj.normalR()));
        double DEG_EPS = 10 * rot_speed.GetR();
        if (delta < DEG_EPS)
        {
            if (speed <= temp_max_speed)
            {
                Accelerate(temp_max_speed - speed); //If there is room to accelerate, and angle isn't far from normal
            }
            Angle target_angle;
            Angle beta = safe_asin(speed / (2 * next_pt->c_r)); //Half of central angle of chord with length equal to speed
            if (next_pt->direction == COUNTERCLOCKWISE)
            {
                target_angle = dir_to_obj.normalL() + beta;
                RotateTo(-target_angle, CLOCKWISE);
            }
            else
            {
                target_angle = dir_to_obj.normalR() - beta;
                RotateTo(-target_angle, COUNTERCLOCKWISE);
            }
            target_ui_angle = target_angle;
            Angle dir_to_obj_next(direction_to_point(next_pt->c_x, next_pt->c_y,
                                                     x + speed * cos(angle.GetR()), y - speed * sin(angle.GetR()) ));
            //Check if object passed end of arc
            if (angle_between(
                        next_pt->direction == COUNTERCLOCKWISE ? dir_to_obj.GetR() : dir_to_obj_next.GetR(),
                        next_pt->e_a.GetR(),
                        next_pt->direction == COUNTERCLOCKWISE ? dir_to_obj_next.GetR() : dir_to_obj.GetR() ))
            {
                path->i++;  //Go to next segment
                return;
            }
        }
        else    //If direction is wrong
        {
            Deccelerate(speed); //Attempt to full stop
            if (next_pt->direction == COUNTERCLOCKWISE)   //Turn in direction, opposite to arc direction (to not crash into arc)
                RotateR();
            else
                RotateL();
        }
    }
    else    //If linear
    {
        double PATH_EPS = 20;
        Angle max_delta(15, DEGREES);   //Set max delta to 15 degrees
        Angle direction_to_pt(direction_to_point(x, y, next_pt->x, next_pt->y));
        target_ui_angle = direction_to_pt;
        if (fabs(anglediff(-direction_to_pt, angle)) < max_delta.GetR())
        {
            RotateTo(-direction_to_pt);
            //Deciding, whether should object accelerate or deccelerate
            double Sacc = (max_speed * max_speed - speed * speed) / (2 * acc);  //Accelerating distance
            double Sdec = (max_speed * max_speed - next_pt->s * next_pt->s ) / (2 * dec);   //Braking distance
            double S = distance(x, y, next_pt->x, next_pt->y);  //Distance to end point
            if (S < PATH_EPS)
            {
                path->i++;
            }
            if (Sacc + Sdec < S + PATH_EPS)
            {
                Accelerate();   //There is possibility to reach maximum speed and sucesfully drop it
            }
            else
            {
                double braking_dist = (speed * speed - next_pt->s * next_pt->s ) / (2 * dec);   //Braking distance from current speed
                qDebug() << "SHOULD I BRAKE?: " << braking_dist <<'<'<<S;
                if (S <= braking_dist + PATH_EPS)
                {
                    Deccelerate(speed - next_pt->s);
                }
                else
                {
                    double Vmax = sqrt((2*acc*dec*S + speed*speed*dec + next_pt->s*next_pt->s*acc) / (acc + dec));
                    Accelerate(Vmax - speed);
                }
            }
        }
        else    //Decreasing speed and trying to turn around
        {
            Deccelerate(speed - 0.5);
            RotateTo(-direction_to_pt);
        }
    }
}

void Car::ShowPath()   //Drawing path
{
    if (path == nullptr)
        return;
    QPainter pntr(picture);
    QPen penn;
    penn.setWidth(3);
    for (int i = 0; i < path->num; i++)
    {
        if (i == path->i)
        {
            penn.setBrush(QBrush(QColor(100, 200, 100)));
            pntr.setPen(penn);
        }
        else if (i == path->i + 1)
        {
            penn.setBrush(QBrush(QColor(200, 100, 100)));
            pntr.setPen(penn);
        }

        if (path->pts[i].circle)
        {
            double sa = -16 * path->pts[i].s_a.GetD();
            double ea = -16 * (path->pts[i].e_a.GetD() - path->pts[i].s_a.GetD());
            if (path->pts[i].direction < 0)
                sa += 5760;
//            qDebug() << path->pts[i].s_a.GetD() << "|" << path->pts[i].e_a.GetD();
            pntr.drawArc(path->pts[i].c_x - path->pts[i].c_r, path->pts[i].c_y - path->pts[i].c_r,
                         2*path->pts[i].c_r, 2*path->pts[i].c_r,
                         floor(sa), floor(ea));
        }
        else
        {
            if (i == 0)
                pntr.drawLine(path->startx, path->starty, path->pts[i].x, path->pts[i].y);
            else
                pntr.drawLine(path->pts[i-1].x, path->pts[i-1].y, path->pts[i].x, path->pts[i].y);
        }
    }
}

double Car::GetPathMaxSpeed()
{
    if (path == nullptr)
        return 0;
    if (path->i >= path->num)    //If path completed
        return 0;
    return path->pts[path->i].s;
}
