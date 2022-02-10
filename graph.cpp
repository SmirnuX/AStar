#include "game.h"

double safe_acos(double ang)
{
    if (ang > 1)
        ang = 0.999;
    if (ang < -1)
        ang = -0.999;
    double res = acos(ang);
    if (res < 0)
        res += 2 * M_PI;
    if (res > 2*M_PI)
        res -= 2*M_PI;
    return res;
}

double safe_asin(double ang)
{
    if (ang > 1)
        ang = 0.999;
    if (ang < -1)
        ang = -0.999;
    double res = asin(ang);
    if (res < 0)
        res += 2 * M_PI;
    if (res > 2*M_PI)
        res -= 2*M_PI;
    return res;
}

vertex* add_vert(double x, double y, obstacle* _parent, double _angle)    //Создание вершины
{
    vertex* res = new vertex;
    res->point = new Point(x, y);
    res->parent = _parent;
    res->angle = _angle;
    if (res->angle < 0)
        res->angle += 2 * M_PI;
    if (res->angle > 2*M_PI)
        res->angle -= 2*M_PI;
    return res;
}

vertex* add_vert(Point* pt, obstacle* _parent, double _angle)    //Создание вершины
{
    vertex* res = new vertex;
    res->point = new Point(pt->GetX(), pt->GetY());
    res->parent = _parent;
    res->angle = _angle;
    if (res->angle < 0)
        res->angle += 2 * M_PI;
    if (res->angle > 2*M_PI)
        res->angle -= 2*M_PI;
    return res;
}

graph* build_graph(obstacle* objects, int count) //Построение графа
{
    graph* result = new graph(); //Граф
    for (int i = 0; i < count; i++)
    {
        for (int j = i+1; j < count; j++)
        {
            obstacle* A = objects + i;
            obstacle* B = objects + j;

            int temp_count = 0;
            struct edge temp_edges[MAX_TEMP_EDGES];
            int temp_vert_count = 0;
            struct vertex* temp_verts[MAX_TEMP_VERTS];

            if (A->shape == POINT && B->shape == POINT) //Точка-точка
            {
                //Временные вершины
                temp_vert_count = 2;
                temp_verts[0] = add_vert(A->point, A);
                temp_verts[1] = add_vert(B->point, B);

                struct vertex* path_verts[2];   //Начальные и конечные точки
                path_verts[0] = add_vert(A->point, A);
                path_verts[1] = add_vert(B->point, B);

                //Временная линия
                temp_count = 1;
                edge line;
                line.chosen = false;
                line.passed = false;
                line.type = LINEAR;
                line.pA = temp_verts[0];
                line.pB = temp_verts[1];
                line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                temp_edges[0] = line;

                result->vertices.push_back(path_verts[0]);
                result->vertices.push_back(path_verts[1]);

                //continue;
            }
            else if (A->shape == CIRCLE && B->shape == CIRCLE)  //Круг-круг
            {
                //Дистанция между центрами
                double dist2 = distance2(*(A->point), *(B->point));
                double dist = sqrt(dist2);
                if (dist < fabs(A->rA - B->rA)) //Один круг содержит в себе другой
                {
                    temp_vert_count = 0;
                    temp_count = 0;
                    continue;   //Не добавляются дуги
                }
                else
                {
                    temp_vert_count = 4;
                    temp_count = 2;

                    //Поиск необходимого угла
                    double angle = safe_acos((B->point->GetX() - A->point->GetX())/dist);
                    if ((B->point->GetY() - A->point->GetY()) < 0)
                        angle = 2*M_PI - angle;
                    double outer_angle = 2 * safe_acos(-fabs(A->rA - B->rA) / dist);  //Угол между внешними касательными

                    //Добавление вершин
                    temp_verts[0] = add_vert(A->point->GetX() + A->rA * cos(angle+outer_angle/2),
                                             A->point->GetY() + A->rA * sin(angle+outer_angle/2),
                                             A, angle+outer_angle/2);
                    temp_verts[1] = add_vert(B->point->GetX() + B->rA * cos(angle+outer_angle/2),
                                             B->point->GetY() + B->rA * sin(angle+outer_angle/2),
                                             B, angle+outer_angle/2);

                    temp_verts[2] = add_vert(A->point->GetX() + A->rA * cos(angle-outer_angle/2),
                                             A->point->GetY() + A->rA * sin(angle-outer_angle/2),
                                             A, angle-outer_angle/2);
                    temp_verts[3] = add_vert(B->point->GetX() + B->rA * cos(angle-outer_angle/2),
                                             B->point->GetY() + B->rA * sin(angle-outer_angle/2),
                                             B, angle-outer_angle/2);

                    //Добавление граней
                    edge line;
                    line.chosen = false;
                    line.passed = false;
                    line.type = LINEAR;
                    line.pA = temp_verts[0];
                    line.pB = temp_verts[1];
                    line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                    temp_edges[0] = line;
                    line.pA = temp_verts[2];
                    line.pB = temp_verts[3];
                    line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                    temp_edges[1] = line;

                    if (dist > (A->rA + B->rA)) //Окружности не пересекаются
                    {

                        temp_vert_count = 8;
                        temp_count = 4;

                        //Поиск необходимого угла
                        //double angle = atan((B->point->GetY() - A->point->GetY())/(B->point->GetX() - A->point->GetX()));
                        //double angle = acos((B->point->GetX() - A->point->GetX())/dist);
                        double inner_angle = safe_acos((A->rA + B->rA) / dist);  //Угол между внешними касательными

                        //Добавление вершин
                        temp_verts[4] = add_vert(A->point->GetX() + A->rA * cos(angle+inner_angle),
                                                 A->point->GetY() + A->rA * sin(angle+inner_angle),
                                                 A, angle+inner_angle);
                        temp_verts[5] = add_vert(B->point->GetX() + B->rA * cos(M_PI+angle+inner_angle),
                                                 B->point->GetY() + B->rA * sin(M_PI+angle+inner_angle),
                                                 B, M_PI+angle+inner_angle);

                        temp_verts[6] = add_vert(A->point->GetX() + A->rA * cos(angle-inner_angle),
                                                 A->point->GetY() + A->rA * sin(angle-inner_angle),
                                                 A, angle-inner_angle);
                        temp_verts[7] = add_vert(B->point->GetX() + B->rA * cos(M_PI+angle-inner_angle),
                                                 B->point->GetY() + B->rA * sin(M_PI+angle-inner_angle),
                                                 B, M_PI+angle-inner_angle);

                        //Добавление граней
                        edge line;
                        line.chosen = false;
                        line.passed = false;
                        line.type = LINEAR;
                        line.pA = temp_verts[4];
                        line.pB = temp_verts[5];
                        line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                        temp_edges[2] = line;
                        line.pA = temp_verts[6];
                        line.pB = temp_verts[7];
                        line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                        temp_edges[3] = line;
                    }

                }
            }
            else    //Точка-круг (и наоборот)
            {
                double dist = sqrt(distance2(*(A->point), *(B->point)));
                obstacle* pt;
                obstacle* circle;
                if (A->shape == CIRCLE)
                {
                    circle = A;
                    pt = B;
                }
                else
                {
                    circle = B;
                    pt = A;
                }
                temp_vert_count = 4;
                temp_count = 2;

                //Поиск необходимого угла
                double angle = safe_acos((circle->point->GetX() - pt->point->GetX())/dist);
                if ((circle->point->GetY() - pt->point->GetY()) < 0)
                    angle = 2*M_PI - angle;
                double outer_angle = 2 * safe_acos(circle->rA / dist);  //Угол между внешними касательными

                //Добавление вершин
                temp_verts[0] = add_vert(pt->point->GetX(),
                                         pt->point->GetY(), pt);
                temp_verts[1] = add_vert(pt->point->GetX(),
                                         pt->point->GetY(), pt);
                temp_verts[2] = add_vert(circle->point->GetX() + circle->rA * cos(M_PI+angle-outer_angle/2),
                                         circle->point->GetY() + circle->rA * sin(M_PI+angle-outer_angle/2),
                                         circle, M_PI+angle-outer_angle/2);
                temp_verts[3] = add_vert(circle->point->GetX() + circle->rA * cos(M_PI+angle+outer_angle/2),
                                         circle->point->GetY() + circle->rA * sin(M_PI+angle+outer_angle/2),
                                         circle, M_PI+angle+outer_angle/2);
                //Добавление граней
                edge line;
                line.chosen = false;
                line.passed = false;
                line.type = LINEAR;
                line.pA = temp_verts[0];
                line.pB = temp_verts[2];
                line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                temp_edges[0] = line;
                line.pA = temp_verts[1];
                line.pB = temp_verts[3];
                line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                temp_edges[1] = line;
            }
            //Проверка на пересечение
            for (int k = 0; k < temp_count; k++)
            {
                bool end = false;
                LineCollider lin(temp_edges[k].pA->point->GetX(),
                             temp_edges[k].pA->point->GetY(),
                             temp_edges[k].pB->point->GetX(),
                             temp_edges[k].pB->point->GetY());
                for (int z = 0; z < count; z++) //Z^3-4
                {
                    if (z == i || z == j || objects[z].shape == POINT)
                        continue;
                    CircleCollider crc(objects[z].point->GetX(),
                                       objects[z].point->GetY(),
                                       objects[z].rA);
                    if (lin.CheckCollision(&crc))
                    {
                        end = true;
                        break;
                    }
                }
                if (end)
                    continue;
                //Слияние рядом стоящих вершин
                double eps = 4;
                bool first = true;
                bool sec = true;
                for (unsigned int z = 0; z < result->vertices.size(); z++) //Z^3-4
                {
                    if (first)
                    {
                        if (almostEq(result->vertices[z]->point->GetX(), temp_edges[k].pA->point->GetX(), eps) &&
                            almostEq(result->vertices[z]->point->GetY(), temp_edges[k].pA->point->GetY(), eps))
                        {
                            first = false;
                            delete temp_edges[k].pA;
                            temp_edges[k].pA = result->vertices[z];
                        }
                    }
                    if (sec)
                    {
                        if (almostEq(result->vertices[z]->point->GetX(), temp_edges[k].pB->point->GetX(), eps) &&
                            almostEq(result->vertices[z]->point->GetY(), temp_edges[k].pB->point->GetY(), eps))
                        {
                            sec = false;
                            delete temp_edges[k].pB;
                            temp_edges[k].pB = result->vertices[z];
                        }
                    }

                }
                result->edges.push_back(temp_edges[k]);
                if (first)
                {
                    result->vertices.push_back(temp_edges[k].pA);
                    //qDebug()<<temp_edges[k].pA->angle;
                }
                if (sec)
                {
                    result->vertices.push_back(temp_edges[k].pB);
                    //qDebug()<<temp_edges[k].pB->angle;
                }
            }
        }
    }
    //Соединение дугами - по часовой стрелке
    std::vector<vertex*> brothers;  //Вершины, лежащие на одном круге
    for (int i = 0; i < count; i++)
    {
        //Поиск "братьев"
        if (objects[i].shape != CIRCLE)
            continue;
        for (unsigned int j = 0; j < result->vertices.size(); j++)   //Поиск всех вершин, прилежащих к кругу
        {
            if (result->vertices[j]->parent != objects+i)
                continue;
            brothers.push_back(result->vertices[j]);
            for (int k = brothers.size()-1; k > 0; k--) //Сортировка по углу
            {
                if (brothers[k]->angle < brothers[k-1]->angle)
                {
                    vertex* temp = brothers[k];
                    brothers[k] = brothers[k-1];
                    brothers[k-1] = temp;
                }
            }
        }
        //Непосредственно соединение дугами
        int n = brothers.size();
        edge arc;
        arc.chosen = false;
        arc.type = ARC_CIRCLE;
        arc.cx = objects[i].point->GetX();
        arc.cy = objects[i].point->GetY();
        arc.rA = objects[i].rA;

        for (int j = 0; j < n; j++)
        {
            int next = j+1;
            if (next > n-1)
                next = 0;

            //Проверка на пересечение с другими окружностями
            bool intersec = false;
            CircleCollider ptA_c(objects[i].point->GetX(),
                                objects[i].point->GetY(), arc.rA);
            for (int k = 0; k < count; k++)
            {
                if (objects[k].shape != CIRCLE || k == i)
                    continue;
                CircleCollider cr_c(objects[k].point->GetX(),
                                    objects[k].point->GetY(),
                                    objects[k].rA);
                if (ptA_c.CheckCollision(&cr_c))
                {
                    //Если круги пересекаются - ищем угол
                    double dx = objects[k].point->GetX() - objects[i].point->GetX();
                    double dy = objects[k].point->GetY() - objects[i].point->GetY();
                    double circ_dist = sqrt(dx*dx + dy*dy);
                    double int_angle = safe_acos(dx/circ_dist);
                    if (dy<0)
                        int_angle = 2*M_PI - int_angle;
                    //Проверка, лежит ли линяи между окружностями на дуге
                    double min_a = brothers[j]->angle;
                    double max_a = brothers[next]->angle;
                    if (min_a > max_a)
                       min_a -= 2*M_PI;
                    if (min_a <= int_angle && int_angle <= max_a)
                    {
                        intersec = true;
                        break;
                    }
                }
            }
            if (intersec)
                continue;
            arc.aA = brothers[j]->angle;
            arc.aB = brothers[next]->angle;
            arc.pA = brothers[j];
            arc.pB = brothers[next];
            if (brothers[next]->angle < brothers[j]->angle)
                arc.length = (brothers[next]->angle - brothers[j]->angle + 2*M_PI) * arc.rA;
            else
                arc.length = (brothers[next]->angle - brothers[j]->angle) * arc.rA;
            result->edges.push_back(arc);
        }
        //Очистка
        brothers.clear();
    }
    return result;
}

void graph::AStar() //Поиск пути
{
    //ПОИСК МЕЖДУ 0 и 1 ВЕРШИНАМИ
    for (unsigned int i = 0; i < edges.size(); i++)   //Сброс выбранного ранее пути
        edges[i].chosen = false;
    vertex* start = vertices[0];
    vertex* end = vertices[1];
    start->dist = 0;
    start->cost = 0;
    bool running = true;
    std::vector<vertex*> closed;
    std::priority_queue<graph_cmp, std::vector<graph_cmp>, fnctor> open;

    for (unsigned int i = 1; i < vertices.size(); i++)   //Пометка вершин как открытых и вычисление начальной стоимости
    {
        if (i==1)
        {
            end->dist = 0;
            end->cost = 0;
        }
        else
        {
            vertices[i]->dist = sqrt(distance2(*(vertices[i]->point), *(end->point)));
            vertices[i]->cost = 100000;
        }
    }
    open.push(graph_cmp(start));  //Начальная вершина
    int path_n; //Номер грани
    while (running)
    {
        if (open.empty())
        {
            qDebug("PATH NOT FOUND");
            return;
        }
        vertex* curr = open.top().ptr;  //Выборка вершины с наименьшей стоимостью
        open.pop();
        //Добавление открытых вершин
        for (unsigned int i = 0; i < edges.size(); i++)
        {
            vertex* adj;    //Прилежащая вершина
            if (edges[i].pA == curr)
                adj = edges[i].pB;
            else if (edges[i].pB == curr)
                adj = edges[i].pA;
            else
                continue;
            if (adj == end) //Путь найден
            {
                edges[i].chosen = true;
                path_n = i;
                running = false;
                break;
            }
            if (std::find(closed.begin(), closed.end(), adj) != closed.end())   //Поиск вершины в закрытых
                continue;
            bool found = false;
            /*
            for (int j = 0; j < open.size(); j++)   //Поиск вершины в открытых
            {
                if (open[j].ptr == adj)
                {
                    found = true;
                    break;
                }
            }*/
            if (found)
                continue;
            //Обновление стоимости - TODO - поменять формулу
            if (adj->cost > curr->cost + edges[i].length + adj->dist)
                adj->cost = curr->cost + edges[i].length + adj->dist;
            edges[i].passed = true;
            //tree.push_back(edges[i]);
            open.push(graph_cmp(adj));
        }
        closed.push_back(curr);
    }
    //Поиск самого пути - TODO - change
    running = true;
    vertex* back_curr = end;
    double sum_cost = 0;
    while (running)
    {
        if (closed.empty())
            return;
        double min_cost = 10000;
        int min_n;
        double min_dist;
        vertex* min_vert;
        for (unsigned int i = 0; i < edges.size(); i++)
        {
            vertex* back_adj;    //Прилежащая вершина
            if (edges[i].pA == back_curr)
                back_adj = edges[i].pB;
            else if (edges[i].pB == back_curr)
                back_adj = edges[i].pA;
            else
                continue;
            if (std::find(closed.begin(), closed.end(), back_adj) == closed.end())   //Если вершина не в закрытых
                continue;
            if (back_adj->cost < min_cost)  //Поиск самой дешевой точки
            {
                min_cost = back_adj->cost;  //
                min_dist = edges[i].length; //Длина грани
                min_n = i;
                min_vert = back_adj;
            }
        }
        sum_cost += min_dist;
        back_curr = min_vert;
        closed.erase(std::find(closed.begin(), closed.end(), back_curr));
        edges[min_n].chosen = true;
        if (back_curr == start)
        {
            qDebug()<<"PATH LENGTH:" << sum_cost;
            return;
        }
    }

}

Point* circle_tangent(double cx, double cy, double cr, double a, double b, double c)    //Поиск пересечения прямой и окружности
{
    int new_a = b;
    int new_b = a;
    int new_c = -a*cx - b*cy;

    int x = (c/b - new_b - new_c) / (new_a - a/b);
    int y = (-a * x / b);

    return new Point(x, y);

}

double solve_square1(double a, double b, double c)
{
    double D = b*b - 4*a*c;
    if (D < 0)
        D = 0;
    return (-b - sqrt(D))/(2*a);
}

void graph::Show()
{
    QPainter pntr(picture);
    QPen penn;
    penn.setWidth(1);

    pntr.setPen(penn);
    for (uint i=0; i < edges.size(); i++)
    {
        if (edges[i].type == LINEAR)
        {
            if (edges[i].chosen)
                penn.setColor(QColor(0,255,0,160));
            else if (edges[i].passed)
                penn.setColor(QColor(0,0,255,160));
            else
                penn.setColor(QColor(255,0,0,30));
            pntr.setPen(penn);
            pntr.drawLine(edges[i].pA->point->GetX(), edges[i].pA->point->GetY(), edges[i].pB->point->GetX(), edges[i].pB->point->GetY());
        }
        else
        {
            if (edges[i].chosen)
                penn.setColor(QColor(0,255,0,160));
            else if (edges[i].passed)
                penn.setColor(QColor(0,0,255,160));
            else
                penn.setColor(QColor(255,0,0,30));
            pntr.setPen(penn);
            double sa, ea;

            sa = -5760.0 / (2 * M_PI) * edges[i].pA->angle;
            ea = -5760.0 / (2 * M_PI) * (edges[i].pB->angle-edges[i].pA->angle);
            if (ea > 0)
                ea -= 5760;
            pntr.drawArc(edges[i].cx - edges[i].rA, edges[i].cy - edges[i].rA,
                         2*edges[i].rA, 2*edges[i].rA,
                         floor(sa), floor(ea));
        }
        pntr.drawText((int)(edges[i].pA->point->GetX() + (edges[i].pB->point->GetX() - edges[i].pA->point->GetX())/2), //Отрисовка посередине
                      (int)(edges[i].pA->point->GetY() + (edges[i].pB->point->GetY() - edges[i].pA->point->GetY())/2),
                      QString::number(edges[i].length, 'f', 1));
    }
    penn.setColor(QColor(0,255,0,180));
    pntr.setPen(penn);
    for (uint i=0; i < vertices.size(); i++)
    {
        pntr.drawEllipse(vertices[i]->point->GetX(), vertices[i]->point->GetY(), 4, 4);
    }
}
