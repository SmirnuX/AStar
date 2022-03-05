#include "game.h"

//=== Comparing of two vertices - for std::priority_queue ===
graph_cmp::graph_cmp(vertex* _ptr)
{
    ptr = _ptr;
}

graph_cmp::graph_cmp()
{
    ptr = nullptr;
}

bool fnctor::operator() (graph_cmp const& lhs, graph_cmp const& rhs)   //Priority comparing
{
    return lhs.ptr->cost > rhs.ptr->cost;
}


//=== Graph class realization ===
graph::graph()
{
    found_way = false;
    start = -1;
    end = -1;
}

bool graph::IsWay()
{
    return found_way;
}

void graph::clear()
{
    for (unsigned int i=0; i < vertices.size(); i++)
        delete vertices[i];
}

graph::~graph()
{
    clear();
}

vertex* add_vert(double x, double y, obstacle* _parent, Angle _angle)    //Vertex creation
{
    vertex* res = new vertex;
    res->point = new Point(x, y);
    res->parent = _parent;
    res->angle = _angle;
    return res;
}

vertex* add_vert(Point* pt, obstacle* _parent, Angle _angle)    //Vertex creation
{
    vertex* res = new vertex;
    res->point = new Point(pt->GetX(), pt->GetY());
    res->parent = _parent;
    res->angle = _angle;
    return res;
}

graph* build_graph(obstacle* objects, int count, uint _start, uint  _end, uint delte) //Building graph
{
    graph* result = new graph();
    bool targets = false;
    //Adding obstacles
    for (int i = 0; i < count; i++) //Getting all connections between all obstacles
    {
        for (int j = i+1; j < count; j++)
        {
            struct edge temp_edges[MAX_TEMP_EDGES];
            struct vertex* temp_verts[MAX_TEMP_VERTS];
            struct temp_edges temp_count;

            obstacle* A = objects + i;
            obstacle* B = objects + j;
            if (A->shape == POINT && B->shape == POINT) //Point-Point - only one line. Should be called once
            {
                temp_count = get_edges_point_to_point(temp_verts, temp_edges, A, B);
                if (!targets && result->start == -1)
                    targets = true;
            }
            else if (A->shape == CIRCLE && B->shape == CIRCLE)  //Circle-Circle - 0, 2 or 4 lines
            {
                temp_count = get_edges_circle_to_circle(temp_verts, temp_edges, A, B);
            }
            else    //Point-Circle
            {
                temp_count = get_edges_point_to_circle(temp_verts, temp_edges, A, B);
            }
            //Intersection check and vertice merging
            for (int k = 0; k < temp_count.temp_edges_count; k++)
            {
                bool inter = false;
                LineCollider lin(temp_edges[k].pA->point->GetX(),
                             temp_edges[k].pA->point->GetY(),
                             temp_edges[k].pB->point->GetX(),
                             temp_edges[k].pB->point->GetY());
                for (int z = 0; z < count; z++) //Check collisions with every other object
                {
                    if (z == i || z == j || objects[z].shape == POINT)
                        continue;
                    CircleCollider crc(objects[z].point->GetX(),
                                       objects[z].point->GetY(),
                                       objects[z].r);
                    if (lin.CheckCollision(&crc))
                    {
                        inter = true;
                        break;
                    }
                }
                if (inter && !targets)    //Don't add intersected edges (and their vertices)
                    continue;
                //Merge vertices
                double eps = 4; //Radius of merging
                bool first = true;  //Is first vertex edge not merged yet
                bool sec = true;
                for (unsigned int z = 0; z < result->vertices.size(); z++) //Z^3-4
                {
                    if (targets)    //Skip merging for start and end points
                        break;
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
                if (!inter)   //Don't add intersected line from start and end points
                    result->edges.push_back(temp_edges[k]);
                if (targets)    //Adding start and end points
                {
                    result->start = result->vertices.size();
                    result->end = result->vertices.size() + 1;
                    if (i == _start)
                    {
                        result->vertices.push_back(temp_edges[k].pA);
                        result->vertices.push_back(temp_edges[k].pB);
                    }
                    else
                    {
                        result->vertices.push_back(temp_edges[k].pB);
                        result->vertices.push_back(temp_edges[k].pA);
                    }
                    targets = false;
                    continue;
                }
                if (first)  //If there was no merge, push new vertices
                {
                    result->vertices.push_back(temp_edges[k].pA);
                }
                if (sec)
                {
                    result->vertices.push_back(temp_edges[k].pB);
                }
            }
        }
    }

    //Connecting lines with arcs
    std::vector<vertex*> brothers;  //Vertices lying on the same obstacle
    for (int i = 0; i < count; i++)
    {
        //"Brothers" search
        if (objects[i].shape != CIRCLE)
            continue;
        for (unsigned int j = 0; j < result->vertices.size(); j++)   //Looking for every vertex with this obstacle as parent
        {
            if (result->vertices[j]->parent != objects+i)
                continue;
            brothers.push_back(result->vertices[j]);
            for (int k = brothers.size()-1; k > 0; k--) //Sort by angle
            {
                if (brothers[k]->angle.GetR() < brothers[k-1]->angle.GetR())
                {
                    vertex* temp = brothers[k];
                    brothers[k] = brothers[k-1];
                    brothers[k-1] = temp;
                }
            }
        }
        //Creating connecting arcs
        int n = brothers.size();
        edge arc;
        arc.chosen = false;
        arc.type = ARC_CIRCLE;
        arc.cx = objects[i].point->GetX();
        arc.cy = objects[i].point->GetY();
        arc.r = objects[i].r;

        for (int j = 0; j < n; j++) //Looking for every vertex on this obstacle
        {
            int next = j+1;
            if (next > n-1)
                next = 0;

            //Checking for intersection with other circles
            bool intersec = false;
            CircleCollider ptA_c(objects[i].point->GetX(),
                                objects[i].point->GetY(), arc.r);
            for (int k = 0; k < count; k++)
            {
                if (objects[k].shape != CIRCLE || k == i)
                    continue;
                CircleCollider cr_c(objects[k].point->GetX(),
                                    objects[k].point->GetY(),
                                    objects[k].r);
                if (ptA_c.CheckCollision(&cr_c))
                {
                    //If there is collision - looking for angle of collision
                    double int_angle = direction_to_point(objects[i].point->GetX(), objects[i].point->GetY(),
                                                          objects[k].point->GetX(), objects[k].point->GetY());
                    //Is angle of collision inside of an arc?
                    double min_a = brothers[j]->angle.GetR();
                    double max_a = brothers[next]->angle.GetR();
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
            //Getting length of arc
            arc.aA = brothers[j]->angle;
            arc.aB = brothers[next]->angle;
            arc.pA = brothers[j];
            arc.pB = brothers[next];
            if (brothers[next]->angle.GetR() < brothers[j]->angle.GetR())
                arc.length = (brothers[next]->angle.GetR() - brothers[j]->angle.GetR() + 2*M_PI) * arc.r;
            else
                arc.length = fabs((brothers[next]->angle.GetR() - brothers[j]->angle.GetR()) * arc.r);
            result->edges.push_back(arc);
        }
        //Clearing
        brothers.clear();
    }
    return result;
}

void graph::AStar() //Pathfinding
{
    for (unsigned int i = 0; i < edges.size(); i++)   //Clear previous path
        edges[i].chosen = false;
    vertex* v_start = vertices[start];
    vertex* v_end = vertices[end];
    v_start->dist = 0;
    v_start->cost = 0;
    v_end->dist = 0;
    v_end->cost = 0;
    bool running = true;
    std::vector<vertex*> closed;
    std::priority_queue<graph_cmp, std::vector<graph_cmp>, fnctor> open;

    for (unsigned int i = 0; i < vertices.size(); i++)   //Marking vertices as open
    {
        if (i == start || i == end)
        {
            continue;
        }
        else
        {
            vertices[i]->dist = distance(*(vertices[i]->point), *(v_end->point));
            vertices[i]->cost = MAX_PATH_COST;
        }
    }
    open.push(graph_cmp(v_start));
    while (running)
    {
        if (open.empty())
        {
            qDebug("PATH NOT FOUND");
            found_way = false;
            return;
        }
        vertex* curr = open.top().ptr;  //Choosing vertex with lowest cost
        open.pop();
        //Adding open vertices
        for (unsigned int i = 0; i < edges.size(); i++)
        {
            vertex* adj;    //Adjacent vertices
            if (edges[i].pA == curr)
                adj = edges[i].pB;
            else if (edges[i].pB == curr)
                adj = edges[i].pA;
            else
                continue;   //If this vertex is not adjacent
            if (adj == v_end) //ACCEPTING FIRST WAY - MAY BE NOT BEST
            {
                edges[i].chosen = true;
                edges[i].passed = true;
                running = false;
                break;
            }
            if (std::find(closed.begin(), closed.end(), adj) != closed.end())   //Is this vertex already in closed
                continue;
            //Cost update
            if (adj->cost > curr->cost + edges[i].length + adj->dist)
                adj->cost = curr->cost + edges[i].length + adj->dist;
            edges[i].passed = true;
            open.push(graph_cmp(adj));
        }
        closed.push_back(curr);
    }
    //Pathfinding
    running = true;
    vertex* back_curr = v_end;  //Current node in backward traversal
    double sum_cost = 0;
    while (running)
    {
        if (closed.empty())
        {
            found_way = false;
            qDebug()<<"ERROR - found there is the way, but can't calculate it";
            return;
        }
        double min_cost = MAX_PATH_COST+1;
        int min_n;
        double min_dist;
        vertex* min_vert = nullptr;
        vertex* back_adj;   //Adjacent vertex in backward traversal
        for (unsigned int i = 0; i < edges.size(); i++)
        {
            if (!edges[i].passed)
                continue;
            if (edges[i].pA == back_curr)
                back_adj = edges[i].pB;
            else if (edges[i].pB == back_curr)
                back_adj = edges[i].pA;
            else
                continue;   //If this vertex is not adjacent
            if (std::find(closed.begin(), closed.end(), back_adj) == closed.end() && closed.back() != back_adj)   //If vertex was not passed
                continue;
            if (back_adj->cost < min_cost)  //Looking for cheapest edge
            {
                min_cost = back_adj->cost;
                min_dist = edges[i].length;
                min_n = i;
                min_vert = back_adj;
            }
        }
        if (min_vert == nullptr)
        {
            throw std::runtime_error("Can't find way");
        }
        auto back_curr_v = std::find(closed.begin(), closed.end(), back_curr);
        if (!(back_curr_v == closed.end() && closed.back() != back_curr))
            closed.erase(back_curr_v);
        sum_cost += min_dist;
        back_curr = min_vert;

        edges[min_n].chosen = true;
        if (back_curr == v_start)
        {
            found_way = true;
            qDebug()<<"PATH LENGTH:" << sum_cost;
            return;
        }
    }

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
                penn.setColor(QColor(0,0,0,50));
            else
                penn.setColor(QColor(0,0,0,30));
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

            sa = -5760.0 / (2 * M_PI) * edges[i].pA->angle.GetR();
            ea = -5760.0 / (2 * M_PI) * (edges[i].pB->angle.GetR()-edges[i].pA->angle.GetR());
            if (ea > 0)
                ea -= 5760;
            pntr.drawArc(edges[i].cx - edges[i].r, edges[i].cy - edges[i].r,
                         2*edges[i].r, 2*edges[i].r,
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

void graph::Show(int x, int y)  //Drawing graph with additional info
{
    QPainter pntr(picture);
    QPen penn;
    penn.setWidth(1);
    int selected_edge = -1;
    int selected_vertex = -1;
    pntr.setPen(penn);
    CircleCollider cursor(x, y, 4);
    for (uint i=0; i < edges.size(); i++)
    {
        if (edges[i].type == LINEAR)
        {
            LineCollider edge_coll(edges[i].pA->point->GetX(), edges[i].pA->point->GetY(),
                                   edges[i].pB->point->GetY(), edges[i].pB->point->GetY());
            if (cursor.CheckCollision(&edge_coll) && selected_edge == -1)
            {
                selected_edge = i;
                penn.setColor(QColor(0,255,255,160));
            }
            else if (edges[i].chosen)
                penn.setColor(QColor(0,255,0,160));
            else if (edges[i].passed)
                penn.setColor(QColor(0,0,0,50));
            else
                penn.setColor(QColor(0,0,0,30));
            pntr.setPen(penn);
            pntr.drawLine(edges[i].pA->point->GetX(), edges[i].pA->point->GetY(), edges[i].pB->point->GetX(), edges[i].pB->point->GetY());
        }
        else
        {
            if (edges[i].chosen)
            {
                penn.setColor(QColor(0,255,0,160));
                penn.setWidth(3);
            }
            else if (edges[i].passed)
            {
                penn.setColor(QColor(0,0,255,160));
                penn.setWidth(1);
            }
            else
            {
                penn.setColor(QColor(255,0,0,30));
                penn.setWidth(1);
            }
            pntr.setPen(penn);
            double sa, ea;

            sa = -5760.0 / (2 * M_PI) * edges[i].pA->angle.GetR();
            ea = -5760.0 / (2 * M_PI) * (edges[i].pB->angle.GetR()-edges[i].pA->angle.GetR());
            if (ea > 0)
                ea -= 5760;
            pntr.drawArc(edges[i].cx - edges[i].r, edges[i].cy - edges[i].r,
                         2*edges[i].r, 2*edges[i].r,
                         floor(sa), floor(ea));
        }

    }
    penn.setColor(QColor(0,255,0,180));
    pntr.setPen(penn);
    for (uint i=0; i < vertices.size(); i++)
    {
        if (fabs(vertices[i]->point->GetX() - x) < 6 &&
            fabs(vertices[i]->point->GetY() - y) < 6 && selected_vertex == -1)
        {
            selected_vertex = i;
            penn.setColor(QColor(0, 255, 255));
            penn.setWidth(2);
            pntr.setPen(penn);
            pntr.drawEllipse(vertices[i]->point->GetX() - 3, vertices[i]->point->GetY() - 3, 6, 6);
        }
        else
        {
            penn.setColor(QColor(0,255,0,180));
            penn.setWidth(1);
            pntr.setPen(penn);
        }
        pntr.drawEllipse(vertices[i]->point->GetX() - 2, vertices[i]->point->GetY() - 2, 4, 4);
    }
    pntr.drawEllipse(x - 2, y - 2, 4, 4);
    penn.setWidth(2);
    penn.setColor(QColor(0, 255, 255));
    pntr.setPen(penn);
    if (selected_edge != -1)
    {
        pntr.drawText(x + 50, y + 50,
                      QString("Length: ") + QString::number(edges[selected_edge].length, 'f', 1));
    }
    if (selected_vertex != -1)
    {
        pntr.drawText(x + 50, y + 80,
                      QString("Cost: ") + QString::number(vertices[selected_vertex]->cost, 'f', 1));
        pntr.drawText(x + 50, y + 100,
                      QString("Dist: ") + QString::number(vertices[selected_vertex]->dist, 'f', 1));
    }
}

struct temp_edges get_edges_point_to_point(struct vertex** verts, struct edge* edges,
                                           struct obstacle* A, struct obstacle* B)  //Add line from point A to point B
{
    struct temp_edges count;
    count.temp_edges_count = 1;
    count.temp_vertices_count = 2;
    verts[0] = add_vert(A->point, A);
    verts[1] = add_vert(B->point, B);

    edge line;
    line.chosen = false;
    line.passed = false;
    line.type = LINEAR;
    line.pA = verts[0];
    line.pB = verts[1];
    line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
    edges[0] = line;

    return count;
}

struct temp_edges get_edges_circle_to_circle(struct vertex** verts, struct edge* edges,
                                           struct obstacle* A, struct obstacle* B)  //Add [0,2,4] lines from circle A to circle B
{
    double dist2 = distance2(*(A->point), *(B->point));
    double dist = sqrt(dist2);
    struct temp_edges count;
    if (dist < fabs(A->r - B->r)) //If one circle contains another - 0 lines
    {
        count.temp_edges_count = 0;
        count.temp_vertices_count = 0;
        return count;
    }

    count.temp_edges_count = 2;
    count.temp_vertices_count = 4;

    double angle = safe_acos((B->point->GetX() - A->point->GetX())/dist);   //Angle between circles
    if ((B->point->GetY() - A->point->GetY()) < 0)
        angle = 2*M_PI - angle;
    double outer_angle = 2 * safe_acos(-fabs(A->r - B->r) / dist);  //Angle between outer tangents

    //Adding vertices
    verts[0] = add_vert(A->point->GetX() + A->r * cos(angle+outer_angle/2),
                        A->point->GetY() + A->r * sin(angle+outer_angle/2),
                        A, angle+outer_angle/2);
    verts[1] = add_vert(B->point->GetX() + B->r * cos(angle+outer_angle/2),
                        B->point->GetY() + B->r * sin(angle+outer_angle/2),
                        B, angle+outer_angle/2);
    verts[2] = add_vert(A->point->GetX() + A->r * cos(angle-outer_angle/2),
                        A->point->GetY() + A->r * sin(angle-outer_angle/2),
                        A, angle-outer_angle/2);
    verts[3] = add_vert(B->point->GetX() + B->r * cos(angle-outer_angle/2),
                        B->point->GetY() + B->r * sin(angle-outer_angle/2),
                        B, angle-outer_angle/2);

    //Adding edges
    struct edge line;
    line.chosen = false;
    line.passed = false;
    line.type = LINEAR;
    line.pA = verts[0];
    line.pB = verts[1];
    line.length = distance(*(line.pA->point), *(line.pB->point));
    edges[0] = line;
    line.pA = verts[2];
    line.pB = verts[3];
    line.length = distance(*(line.pA->point), *(line.pB->point));
    edges[1] = line;

    if (dist > (A->r + B->r)) //If circles dont intersect
    {

        count.temp_edges_count = 4;
        count.temp_vertices_count = 8;

        double inner_angle = safe_acos((A->r + B->r) / dist);  //Angle between inner tangents
        verts[4] = add_vert(A->point->GetX() + A->r * cos(angle+inner_angle),
                                 A->point->GetY() + A->r * sin(angle+inner_angle),
                                 A, angle+inner_angle);
        verts[5] = add_vert(B->point->GetX() + B->r * cos(M_PI+angle+inner_angle),
                                 B->point->GetY() + B->r * sin(M_PI+angle+inner_angle),
                                 B, M_PI+angle+inner_angle);

        verts[6] = add_vert(A->point->GetX() + A->r * cos(angle-inner_angle),
                                 A->point->GetY() + A->r * sin(angle-inner_angle),
                                 A, angle-inner_angle);
        verts[7] = add_vert(B->point->GetX() + B->r * cos(M_PI+angle-inner_angle),
                                 B->point->GetY() + B->r * sin(M_PI+angle-inner_angle),
                                 B, M_PI+angle-inner_angle);

        //Adding inner edges
        edge line;
        line.chosen = false;
        line.passed = false;
        line.type = LINEAR;
        line.pA = verts[4];
        line.pB = verts[5];
        line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
        edges[2] = line;
        line.pA = verts[6];
        line.pB = verts[7];
        line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
        edges[3] = line;
    }
    return count;
}

struct temp_edges get_edges_point_to_circle(struct vertex** verts, struct edge* edges,
                                           struct obstacle* A, struct obstacle* B)  //Add 0 or 2 lines from point A to circle B (or vice-versa)
{
    double dist = sqrt(distance2(*(A->point), *(B->point)));
    obstacle* pt;   //Point
    obstacle* circle;   //Circle
    if (A->shape == CIRCLE) //Deciding who is who
    {
        circle = A;
        pt = B;
    }
    else
    {
        circle = B;
        pt = A;
    }
    struct temp_edges count;
    count.temp_vertices_count = 4;
    count.temp_edges_count = 2;

    double angle = safe_acos((circle->point->GetX() - pt->point->GetX())/dist);
    if ((circle->point->GetY() - pt->point->GetY()) < 0)
        angle = 2*M_PI - angle;
    double outer_angle = 2 * safe_acos(circle->r / dist);  //Angle between outer tangents

    verts[0] = add_vert(pt->point->GetX(),
                             pt->point->GetY(), pt);
    verts[1] = add_vert(pt->point->GetX(),
                             pt->point->GetY(), pt);
    verts[2] = add_vert(circle->point->GetX() + circle->r * cos(M_PI+angle-outer_angle/2),
                             circle->point->GetY() + circle->r * sin(M_PI+angle-outer_angle/2),
                             circle, M_PI+angle-outer_angle/2);
    verts[3] = add_vert(circle->point->GetX() + circle->r * cos(M_PI+angle+outer_angle/2),
                             circle->point->GetY() + circle->r * sin(M_PI+angle+outer_angle/2),
                             circle, M_PI+angle+outer_angle/2);
    struct edge line;
    line.chosen = false;
    line.passed = false;
    line.type = LINEAR;
    line.pA = verts[0];
    line.pB = verts[2];
    line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
    edges[0] = line;
    line.pA = verts[1];
    line.pB = verts[3];
    line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
    edges[1] = line;
    return count;
}
