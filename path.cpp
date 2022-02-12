#include "tanks.h"
extern EntityStack* stack;

std::vector<edge> get_path_from_graph(graph *gr, uint _start = 0, uint _end = 1)  //Getting path from graph
{
    vertex* start = gr->vertices[_start];
    vertex* end = gr->vertices[_end];
    vertex* curr = start;
    vertex* prev = nullptr;

    std::vector<edge> result;
    while (curr != end)
    {
        for (uint i = 0; i < gr->edges.size(); i++)
        {
            vertex* adj;    //Adjacent vertex
            if (gr->edges[i].pA == curr)
            {
                adj = gr->edges[i].pB;
            }
            else if (gr->edges[i].pB == curr)
            {
                adj = gr->edges[i].pA;
            }
            else
                continue;
            if (adj == prev)    //If there is cycle - skip
                continue;
            if (gr->edges[i].chosen)    //If this edge is marked as path
            {
                edge temp_edge;
                if (gr->edges[i].type == LINEAR)
                {
                    temp_edge.A.MoveTo(curr->point->GetX(), curr->point->GetY());
                    temp_edge.B.MoveTo(adj->point->GetX(), adj->point->GetY());
                    temp_edge.type = LINEAR;
                }
                else
                {
                    if (gr->edges[i].pA == curr)
                    {
                        temp_edge.aA = gr->edges[i].aA;
                        temp_edge.aB = gr->edges[i].aB;
                    }
                    else
                    {
                        temp_edge.aB = gr->edges[i].aA;
                        temp_edge.aA = gr->edges[i].aB;
                    }
                    temp_edge.type = ARC_CIRCLE;
                    temp_edge.rA = gr->edges[i].rA;
                    temp_edge.cx = gr->edges[i].cx;
                    temp_edge.cy = gr->edges[i].cy;
                }
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
    if (path != nullptr)   //Deleting old path
        delete path;
    int MAX_POINTS = 600;    //Max length of path
    //    double threshold = sqrt(base_width*base_width + base_length*base_length)/2 + 10;
    //    double friction = 0.1;
    //Next point of the path
    double path_x = x;
    double path_y = y;
    Angle path_angle(-angle.GetD()); //Target angle at next point
    double path_speed = speed;  //Target speed at next point
    path = new Path(MAX_POINTS, gr->vertices[target]->point->GetX(), gr->vertices[target]->point->GetY());
    //Переменные для круговых граней
    Angle start_angle;    //Начальный угол эллиптических граней
    Angle end_angle;  //Конечный угол эллиптических граней
    Point center = Point(0,0);   //Центр окружности
    //1. Поиск пути в графе
    std::vector<edge> temp_path = get_path_from_graph(gr);

    //2. Поиск требуемых скоростей для каждой вершины
    uint vertex_count = temp_path.size()+1;
    double* temp_speeds = new double[vertex_count];
    temp_speeds[vertex_count-1] = 0;    //В финале обьект должен остановиться
    for (int i = temp_path.size()-2; i >= 0; i--)   //Скорости идут начиная с конца
    {
        double length;
        //Вычисление длины пути
        if (temp_path[i].type == LINEAR)
            length = distance(temp_path[i].A, temp_path[i].B);
        else
            length = 2 * M_PI * temp_path[i].rA * Angle(temp_path[i].aB - temp_path[i].aA).GetR();
        //Вычисление макисмальной возможной скорости для отрезка
        double max_spd;
        if (i == 0) //Начальная скорость
            max_spd = speed;
        else
        {
            if (temp_path[i].type == ARC_ELLIPSE)
            {
                max_spd = 2 * temp_path[i].rA * cos(degtorad(rot_speed)/2); //V = 2 * R * cos (max_rot / 2)
            }
            else if (temp_path[i-1].type == ARC_ELLIPSE)    //ОПАСНЫЙ КУСОК КОДА
            {
                max_spd = 2 * temp_path[i-1].rA * cos(degtorad(rot_speed)/2);
            }
            else
                max_spd = max_speed;
            if (max_spd > max_speed)
                max_spd = max_speed;
        }
        //Проверка на возможность достижения этой скорости
        double acc_path;
        if (max_spd > temp_speeds[i+2]) //Если следующая скорость меньше текущей
            acc_path = (max_spd * max_spd - temp_speeds[i+2] * temp_speeds[i+2]) / (2 * dec);    //Тормозной путь
        else
            acc_path = (max_spd * max_spd - temp_speeds[i+2] * temp_speeds[i+2]) / (2 * acc);   //Путь для ускорения
        if (acc_path > length)  //Если ускориться/затормозить за нужное время не получится
        {
            if (max_spd > temp_speeds[i+1])
                max_spd = sqrt(fabs(2 * dec * length - temp_speeds[i+2] * temp_speeds[i+2]));
            else
                max_spd = sqrt(fabs(2 * acc * length - temp_speeds[i+2] * temp_speeds[i+2]));
        }
        temp_speeds[i+1] = max_speed;
    }

    //3. Непосредственно построение пути (и проверка скоростей)
    uint path_i = 0;    //Индекс пути
    for (int i = 0; i < MAX_POINTS; i++)
    {
        edge curr = temp_path[path_i];
        //Поиск следующей точки
        if (curr.type == ARC_CIRCLE)  //Проезд по круглой грани
        {
            //Ускорение до максимальной возможной скорости
            if (path_speed < temp_speeds[path_i+2])
            {
                path_speed += acc;
                if (path_speed > temp_speeds[path_i+2])
                    path_speed = temp_speeds[path_i+2];
            }
            //Поворот
            double linear_diff = sqrt((curr.rA + 5)*(curr.rA + 5) + path_speed*path_speed) - curr.rA - 5;   //d = \/(R^2 + PS^2) - R
            double ratio = linear_diff / path_speed;
            double rotation = safe_asin(ratio);   //Необходимый угол поворота
            Angle start, end;
            start = curr.aA;
            end = curr.aB;
            if (start > end)
                rotation = -rotation;
            if (fabs(rotation) <= degtorad(rot_speed))
            {
                path_angle += rotation;
            }
            else
            {
                path_angle += sign(rotation) * rot_speed;
            }
        }
        else    //Проезд по прямой грани
        {
            Angle line_angle(direction_to_point(path_x, path_y, curr.B.GetX(), curr.B.GetY())); //Угол до конца отрезка
            double dist = distance(path_x, path_y, curr.B.GetX(), curr.B.GetY());
            //Определение тормозного/ускорительного пути
            double temp_acc_path = ((path_speed+acc)*(path_speed+acc) - temp_speeds[path_i+1] * temp_speeds[path_i+1]) / (2 * dec);  //ЗАМЕНИТЬ ACC на противоположность temp_ACC
            if (temp_acc_path < dist-(path_speed+acc))  //Если успеет затормозить на следующем такте
                path_speed+=acc;
            else
            {
                temp_acc_path = ((path_speed)*(path_speed) - temp_speeds[path_i+1] * temp_speeds[path_i+1]) / (2 * dec); //тормозной путь без ускорения
                if (temp_acc_path >= dist-path_speed)  //Если не успеет затормозить на следующем такте
                    path_speed-=dec;
            }
            //Поворот
            if ((path_angle - line_angle).GetD() < rot_speed)
                path_angle = line_angle;
            else if (path_angle < line_angle)
                path_angle += degtorad(rot_speed);
            else
                path_angle -= degtorad(rot_speed);
        }
        //Проверка на конец грани
        if (curr.type == ARC_CIRCLE)  //Проехал ли танк нужный угол
        {
            Angle real_angle(direction_to_point(path_x, path_y, curr.cx, curr.cy));
            Angle start = curr.aA;
            Angle end = curr.aB;
            if (start < end && end < real_angle)
                path_i++;
            else if (start > end && end > real_angle)
                path_i++;
        }
        else    //Проехал ли танк нужную точку
        {
            if (curr.A.GetX() < curr.B.GetX())
            {
                if (path_x > curr.B.GetX())
                    path_i++;
            }
            else
            {
                if (path_x < curr.B.GetX())
                    path_i++;
            }
        }
        //Добавление нового шага в путь
        path_x += path_speed * cos(path_angle.GetR());
        path_y += path_speed * sin(path_angle.GetR());
        path->a[i] = path_angle.GetD();
        path->s[i] = path_speed;
        path->x[i] = path_x;
        path->y[i] = path_y;
        if (path_i >= vertex_count) //Конец пути
        {
            path->num = i+1;
            break;
        }
    }
}
