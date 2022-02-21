#include "tanks.h"
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
            double dir = 1.0;   //Direction of movement - aA is before aB
            if (gr->edges[i].pA == curr)
            {
                adj = gr->edges[i].pB;
            }
            else if (gr->edges[i].pB == curr)
            {
                dir = -1.0; //Reverse direction
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
                    temp_edge.rA = gr->edges[i].rA;
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

void Tank::graph_to_path(graph* gr, uint target) //Getting path with physical properties
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
    for (int i = temp_path.size()-2; i >= 0; i--)
    {
        double length = temp_path[i].length;
        //Calculating max speed for this segment
        double max_spd;
        if (i == 0) //Start speed
            max_spd = speed;
        else
        {
            if (temp_path[i].type == ARC_CIRCLE)
            {
                max_spd = 2 * temp_path[i].rA * sin(rot_speed.GetR()/2); //V = 2 * R * sin (max_rot / 2) - max speed you can stay on arc with
            }
            else if (temp_path[i-1].type == ARC_CIRCLE)
            {
                max_spd = 2 * temp_path[i-1].rA * sin(rot_speed.GetR()/2);
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
    }

    //3. Path building

    for (int i = 0; i < segment_count; i++)
    {
        if (temp_path[i].type == ARC_CIRCLE)
        {
            path->pts[i].c_x = temp_path[i].cx;
            path->pts[i].c_y = temp_path[i].cy;
            path->pts[i].c_r = temp_path[i].rA;
            path->pts[i].s_a = temp_path[i].aA;
            path->pts[i].e_a = temp_path[i].aB;
            qDebug() << path->pts[i].s_a.GetD() << " " << path->pts[i].e_a.GetD();
        }
        path->pts[i].circle = temp_path[i].type == ARC_CIRCLE;
        path->pts[i].x = temp_path[i].B.GetX();
        path->pts[i].y = temp_path[i].B.GetY();
        path->pts[i].s = temp_speeds[i];
        path->pts[i].direction = temp_path[i].direction;
    }

    delete[] temp_speeds;
//    for (int i = 0; i < MAX_POINTS; i++)
//    {
//        edge curr = temp_path[path_i];
//        if (curr.type == ARC_CIRCLE)  //Arc traversal
//        {
//            //Acceleration to max possible speed
//            if (path_speed < temp_speeds[path_i+1] - EPSILON)
//            {
//                path_speed += acc - friction;
//                if (path_speed > temp_speeds[path_i+1])
//                    path_speed = temp_speeds[path_i+1];
//            }
//            else if (path_speed > temp_speeds[path_i+1] + EPSILON)  //Or braking
//            {
//                path_speed -= dec + friction;
//                if (path_speed < temp_speeds[path_i+1])
//                    path_speed = temp_speeds[path_i+1];
//            }
//            //Rotation
//            double angle_diff = 2 * safe_asin(path_speed / (2 * curr.rA));  //alpha = 2 * asin(L/2R) - difference between current angle on circle and next
//            Angle start, end;
//            start = curr.aA;
//            end = curr.aB;
//            if (start > end)
//                rotation = -rotation;
//            if (fabs(rotation) <= rot_speed.GetR())
//            {
//                path_angle += rotation;
//            }
//            else
//            {
//                path_angle += sign(rotation) * rot_speed.GetR();
//            }
//        }
//        else    //Straight line traversal
//        {
//            Angle line_angle(direction_to_point(path_x, path_y, curr.B.GetX(), curr.B.GetY())); //Direction to end of line
//            double dist = distance(path_x, path_y, curr.B.GetX(), curr.B.GetY());
//            double temp_acc_path = ((path_speed+acc)*(path_speed+acc) - temp_speeds[path_i+1] * temp_speeds[path_i+1]) / (2 * dec);
//            if (temp_acc_path < dist-(path_speed+acc))
//                path_speed+=acc;
//            else
//            {
//                temp_acc_path = ((path_speed)*(path_speed) - temp_speeds[path_i+1] * temp_speeds[path_i+1]) / (2 * dec);    //without acceleration
//                if (temp_acc_path >= dist-path_speed)
//                    path_speed-=dec;
//            }
//            if ((path_angle - line_angle).GetR() < rot_speed.GetR())
//                path_angle = line_angle;
//            else if (path_angle < line_angle)
//                path_angle += rot_speed;
//            else
//                path_angle -= rot_speed;
//        }
//        //Check, if already in the end of path segment
//        if (curr.type == ARC_CIRCLE)
//        {
//            Angle real_angle(direction_to_point(path_x, path_y, curr.cx, curr.cy));
//            Angle start = curr.aA;
//            Angle end = curr.aB;
//            if (start < end && end < real_angle)
//                path_i++;
//            else if (start > end && end > real_angle)
//                path_i++;
//        }
//        else
//        {
//            if (curr.A.GetX() < curr.B.GetX())
//            {
//                if (path_x > curr.B.GetX())
//                    path_i++;
//            }
//            else
//            {
//                if (path_x < curr.B.GetX())
//                    path_i++;
//            }
//        }
//        //Adding new path point
//        path_x += path_speed * cos(path_angle.GetR());
//        path_y += path_speed * sin(path_angle.GetR());
//        path->a[i] = path_angle;
//        path->s[i] = path_speed;
//        path->x[i] = path_x;
//        path->y[i] = path_y;
//        log_str << i << "\t" << path_angle.GetD() << "d\t" << path_speed <<
//                   "\t x = " << path_x << "\t y = " << path_y << "\n";
//        if (path_i >= vertex_count) //End of path
//        {
//            path->num = i+1;
//            break;
//        }
//    }
}

void Tank::FollowPath() //Actually follow built path
{
    if (path == NULL)
        return;
//    if (path->s[path->i] > speed)
//        Accelerate(path->s[path->i] - speed);
//    else
//        Deccelerate(speed - path->s[path->i]);
//    //Tank angle - clockwise, nut path angle - counter clockwise
//    qDebug() << anglediff(-path->a[path->i], angle);
//    if (fabs(anglediff(-path->a[path->i], angle)) < rot_speed)   //If difference between target and current angles can be solved by one frsme
//        SetAngle(-path->a[path->i]);
//    else if (fabs(anglediff(-path->a[path->i], angle)) > EPSILON)  //If there is difference, but it can't be solved in moment
//    {
//        if (angle > -path->a[path->i])
//            Turn(-rot_speed);
//        else
//            Turn(rot_speed);
//    }
//    path->i++;
//    if (path->i == path->num || (almostEq(x, path->final_x, 2) && almostEq(y, path->final_y, 2)))
//    {
//        delete path;
//        path = nullptr;
//    }
}

void Tank::ShowPath()   //Drawing path
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
            qDebug() << path->pts[i].s_a.GetD() << "|" << path->pts[i].e_a.GetD();
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
