#include "game.h"
#include "objects.h"
#include <QThread>
#include <QThreadPool>

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
    {
        delete vertices[i]->point;
        delete vertices[i];
    }
    vertices.clear();
    edges.clear();
}

graph::~graph()
{
    clear();
}

void ShowObstacle(obstacle* obst)
{
    QPainter pntr(picture);
    QPen penn;
    penn.setWidth(1);
    penn.setColor(QColor(255,0,0));
    pntr.setPen(penn);
    if (obst->shape == CIRCLE)
    {
        pntr.drawEllipse(obst->point->GetX() - obst->r, obst->point->GetY() - obst->r,
                     2*obst->r, 2*obst->r);
    }
    else if (obst->shape == POLYGON)
    {
        for (uint i=0; i < obst->num; i++)
        {
            if (obst->outline[i].type == LINEAR)
            {
                pntr.drawLine(obst->outline[i].A.GetX(),
                              obst->outline[i].A.GetY(),
                              obst->outline[i].B.GetX(),
                              obst->outline[i].B.GetY());
            }
            else
            {
//                penn.setColor(QColor(90, 90, 90));
//                pntr.setPen(penn);
//                pntr.drawEllipse(obst->outline[i].cx - obst->outline[i].r, obst->outline[i].cy - obst->outline[i].r,
//                                 2*obst->outline[i].r, 2*obst->outline[i].r);
//                penn.setColor(QColor(255,0,0));
//                pntr.setPen(penn);
                double sa, ea;
                sa = -5760.0 / (2 * M_PI) * obst->outline[i].aA.GetR();
                ea = -5760.0 / (2 * M_PI) * (obst->outline[i].aB.GetR()-obst->outline[i].aA.GetR());
                if (ea > 0)
                    ea -= 5760;
                pntr.drawArc(obst->outline[i].cx - obst->outline[i].r, obst->outline[i].cy - obst->outline[i].r,
                             2*obst->outline[i].r, 2*obst->outline[i].r,
                             floor(sa), floor(ea));
            }
        }
    }
}

void DeleteObstacle(obstacle* obst) //Deleting obstacle
{
    switch (obst->shape)
    {
    case POINT:
    case CIRCLE:
        delete obst->point;
        break;
    case POLYGON:
        delete[] obst->outline;
        break;
    }
}

vertex* add_vert(double x, double y, obstacle* _parent, Angle _angle)    //Vertex creation
{
    vertex* res = new vertex;
    res->poly_i = -1;
    res->point = new Point(x, y);
    res->parent = _parent;
    res->angle = _angle;
    return res;
}

vertex* add_vert(Point* pt, obstacle* _parent, Angle _angle)    //Vertex creation
{
    vertex* res = new vertex;
    res->poly_i = -1;
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
            std::vector<edge> temp_edges;
            std::vector<vertex*> temp_verts;
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
            else if (A->shape == POLYGON && B->shape == POLYGON)    //Poly-Poly - from 0 to 4N*M lines
            {
                temp_count = get_edges_polygon_to_polygon(temp_verts, temp_edges, A, B);
            }
            else if (A->shape == POINT || B->shape == POINT)
            {
                if (A->shape == CIRCLE || B->shape == CIRCLE)
                    temp_count = get_edges_point_to_circle(temp_verts, temp_edges, A, B);   //Point-Circle - 0 or 2 lines
                else
                    temp_count = get_edges_point_to_polygon(temp_verts, temp_edges, A, B);   //Point-Poly - up to 2N lines
            }
            else    //Circle-Poly - from 0 to 4N lines
            {
                temp_count = get_edges_circle_to_polygon(temp_verts, temp_edges, A, B);
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
                    if (objects[z].shape == CIRCLE) //Intersection with circles
                    {
                        CircleCollider crc(objects[z].point->GetX(),
                                           objects[z].point->GetY(),
                                           objects[z].r);
                        if (lin.CheckCollision(&crc))
                        {
                            inter = true;
                            break;
                        }
                    }
                    else    //Intersection with polygons
                    {
                        for (uint _i = 0; _i < objects[z].num; _i++)
                        {
                            edge* edg = objects[z].outline + _i;
                            if (edg->type == LINEAR)
                            {
                                LineCollider lc(edg->A.GetX(), edg->A.GetY(),
                                                edg->B.GetX(), edg->B.GetY());
                                if (lin.CheckCollision(&lc))
                                {
                                    inter = true;
                                    break;
                                }
                            }
                            else    //Intersection with arcs
                            {
                                Line edge_ln;
                                edge_ln.Set(temp_edges[k].pA->point->GetX(), temp_edges[k].pA->point->GetY(),
                                         temp_edges[k].pB->point->GetX(), temp_edges[k].pB->point->GetY());
                                if (arc_line_collision(Circle(edg->cx, edg->cy, edg->r),
                                                       edge_ln,
                                                       edg->direction == COUNTERCLOCKWISE ? edg->aA.GetR() : edg->aB.GetR(),
                                                       edg->direction == COUNTERCLOCKWISE ? edg->aB.GetR() : edg->aA.GetR()))
                                {
                                    inter = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (inter && !targets)    //Don't add intersected edges (and their vertices)
                {
                    delete temp_edges[k].pA;
                    delete temp_edges[k].pB;
                    continue;
                }
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
            temp_edges.clear();
            temp_verts.clear();
        }
    }

    //Connecting lines with arcs
    std::vector<vertex*> brothers;  //Vertices lying on the same obstacle
    for (int i = 0; i < count; i++)
    {
        //"Brothers" search
        if (objects[i].shape != CIRCLE && objects[i].shape != POLYGON)
            continue;
        if (objects[i].shape == CIRCLE) //DIVIDE INTO FUNCTION
        {
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
            arc.passed = false;
            arc.type = ARC_CIRCLE;
            arc.direction = COUNTERCLOCKWISE;
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
        }
        else if (objects[i].shape == POLYGON) //DIVIDE INTO FUNCTION
        {
            for (unsigned int j = 0; j < result->vertices.size(); j++)   //Looking for every vertex with this obstacle as parent
            {
                if (result->vertices[j]->parent != objects+i)
                    continue;
                brothers.push_back(result->vertices[j]);
                assert(result->vertices[j]->poly_i != -1);  //If vertex is on polygon obstacle, it should have index
                for (int k = brothers.size()-1; k > 0; k--) //Sort by edge index
                {
                    if (brothers[k]->poly_i < brothers[k-1]->poly_i)
                    {
                        vertex* temp = brothers[k];
                        brothers[k] = brothers[k-1];
                        brothers[k-1] = temp;
                    }
                }
            }
            unsigned int _i = 0;    //Number in brothers vector
            edge* obst_outline = objects[i].outline;
            vertex* first = nullptr;
            if (obst_outline[0].type == LINEAR)
                first = add_vert(&obst_outline[0].A, objects+i);
            else
                first = add_vert(obst_outline[0].cx + obst_outline[0].r * cos(obst_outline[0].aA.GetR()),
                        obst_outline[0].cy + obst_outline[0].r * sin(obst_outline[0].aA.GetR()),
                        objects+i);
            result->vertices.push_back(first);
            vertex* last = first;
            for (int j = 0; j < objects[i].num; j++)
            {
                vertex* next = nullptr;

                if (obst_outline[j].type == LINEAR) //Duplicating linear edge into graph
                {
                    if (j == objects[i].num - 1)
                    {
                        next = first;
                        next->point->MoveTo(obst_outline[j].B.GetX(), obst_outline[j].B.GetY());
                    }
                    else
                    {
                        next = add_vert(&obst_outline[j].B, objects+i);
                        result->vertices.push_back(next);
                    }
                    edge line;
                    line.chosen = false;
                    line.passed = false;
                    line.type = LINEAR;
                    line.pA = last;
                    line.pB = next;
                    line.length = distance(last->point->GetX(), last->point->GetY(), next->point->GetX(), next->point->GetY());
                    //The end
                    last = next;
                    result->edges.push_back(line);
                    continue;
                }
                //Adding arcs
                edge arc;   //Template of arc
                arc.type = ARC_CIRCLE;
                arc.r = obst_outline[j].r;
                arc.cx = obst_outline[j].cx;
                arc.cy = obst_outline[j].cy;
                arc.direction = COUNTERCLOCKWISE;
                arc.passed = false;
                arc.chosen = false;

                std::vector<vertex*> temp_vec;  //Vector of vertices on current arc
                for (int k = _i; k < brothers.size(); k++)
                {
                    if (brothers[k]->poly_i == j)
                        temp_vec.push_back(brothers[k]);
                    else
                    {
                        _i = k; //End of current segment
                        break;
                    }
                }

                if (j == objects[i].num - 1)
                {
                    next = first;
                    next->angle = obst_outline[j].aB;
                }
                else
                {
                    next = add_vert(obst_outline[j].cx + obst_outline[j].r * cos(obst_outline[j].aB.GetR()),
                            obst_outline[j].cy + obst_outline[j].r * sin(obst_outline[j].aB.GetR()),
                            objects+i);
                    next->angle = obst_outline[j].aB;
                    result->vertices.push_back(next);
                }
                last->angle = obst_outline[j].aA;

                if (temp_vec.size() == 0)   //If there is no vertices on this arc
                {
                    arc.aA = obst_outline[j].aA;
                    arc.aB = obst_outline[j].aB;
                    arc.pA = last;
                    arc.pB = next;
                    if (arc.aB.GetR() < arc.aA.GetR())  //Getting length
                        arc.length = (arc.aB.GetR() - arc.aA.GetR() + 2*M_PI) * arc.r;
                    else
                        arc.length = fabs((arc.aB.GetR() - arc.aA.GetR()) * arc.r);
                    result->edges.push_back(arc);
                }
                else
                {
                    double startA = obst_outline[j].aA.GetR();  //Start angle of arc [0; 2PI]
                    double endA = obst_outline[j].aB.GetR();    //End angle of arc [0; 4PI]
                    if (endA < startA)
                        endA += 2 * M_PI;
                    //Sort by angle
                    std::sort(temp_vec.begin(), temp_vec.end(),
                              [&] (vertex* a, vertex* b) -> bool
                    {
                        double aA = a->angle.GetR() > startA ? a->angle.GetR() : a->angle.GetR() + 2*M_PI;
                        double bA = b->angle.GetR() > startA ? b->angle.GetR() : b->angle.GetR() + 2*M_PI;
                        return  aA < bA;
                    });
                    //First arc
                    arc.pA = last;
                    arc.pB = temp_vec[0];
                    arc.aA = arc.pA->angle;
                    arc.aB = arc.pB->angle;
                    if (arc.aB.GetR() < arc.aA.GetR())
                        arc.length = (arc.aB.GetR() - arc.aA.GetR() + 2*M_PI) * arc.r;
                    else
                        arc.length = fabs((arc.aB.GetR() - arc.aA.GetR()) * arc.r);
                    result->edges.push_back(arc);
                    //Middle connections
                    for (int k = 0; k < temp_vec.size()-1; k++)
                    {
                        arc.pA = temp_vec[k];
                        arc.pB = temp_vec[k+1];
                        arc.aA = arc.pA->angle;
                        arc.aB = arc.pB->angle;
                        if (arc.aB.GetR() < arc.aA.GetR())
                            arc.length = (arc.aB.GetR() - arc.aA.GetR() + 2*M_PI) * arc.r;
                        else
                            arc.length = fabs((arc.aB.GetR() - arc.aA.GetR()) * arc.r);
                        result->edges.push_back(arc);
                    }
                    //Last connection
                    arc.pA = temp_vec[temp_vec.size()-1];
                    arc.pB = next;
                    arc.aA = arc.pA->angle;
                    arc.aB = arc.pB->angle;
                    if (arc.aB.GetR() < arc.aA.GetR())
                        arc.length = (arc.aB.GetR() - arc.aA.GetR() + 2*M_PI) * arc.r;
                    else
                        arc.length = fabs((arc.aB.GetR() - arc.aA.GetR()) * arc.r);
                    result->edges.push_back(arc);
                }
                last = next;
            }
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
            way_length = sum_cost;
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
                penn.setColor(QColor(0,0,0,100));
            else
                penn.setColor(QColor(0,0,0,50));
            pntr.setPen(penn);
            pntr.drawLine(edges[i].pA->point->GetX(), edges[i].pA->point->GetY(), edges[i].pB->point->GetX(), edges[i].pB->point->GetY());
        }
        else
        {
            if (edges[i].chosen)
                penn.setColor(QColor(0,255,0,160));
            else if (edges[i].passed)
                penn.setColor(QColor(0,0,0,100));
            else
                penn.setColor(QColor(0,0,0,50));
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
                                   edges[i].pB->point->GetX(), edges[i].pB->point->GetY());
            if (cursor.CheckCollision(&edge_coll) && selected_edge == -1)
            {
                selected_edge = i;
                penn.setColor(QColor(0,255,255,160));
                penn.setWidth(3);
            }
            else if (edges[i].chosen)
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
            pntr.drawLine(edges[i].pA->point->GetX(), edges[i].pA->point->GetY(), edges[i].pB->point->GetX(), edges[i].pB->point->GetY());
        }
        else
        {
            if (arc_line_collision(Circle(edges[i].cx, edges[i].cy, edges[i].r),
                                   Line(Point(x-5, y-5), Point(x+5, y+5)),
                                   edges[i].aA.GetR(), edges[i].aB.GetR())
                     && selected_edge == -1)
            {
                selected_edge = i;
                penn.setColor(QColor(0,255,255,160));
                penn.setWidth(3);
            }
            else if (edges[i].chosen)
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
    penn.setColor(QColor(1, 1, 1));
    pntr.setPen(penn);
    if (selected_edge != -1)
    {
        pntr.drawText(x + 50, y + 50,
                      QString("Length: ") + QString::number(edges[selected_edge].length, 'f', 1));
        if (edges[selected_edge].type == ARC_CIRCLE)
        {
            pntr.drawText(x + 50, y + 60,
                          QString("Start angle: ") + QString::number(edges[selected_edge].aA.GetD(), 'f', 1));
            pntr.drawText(x + 50, y + 70,
                          QString("End angle: ") + QString::number(edges[selected_edge].aB.GetD(), 'f', 1));
        }
    }
    if (selected_vertex != -1)
    {
        pntr.drawText(x + 50, y + 80,
                      QString("Cost: ") + QString::number(vertices[selected_vertex]->cost, 'f', 1));
        pntr.drawText(x + 50, y + 100,
                      QString("Dist: ") + QString::number(vertices[selected_vertex]->dist, 'f', 1));
    }
}

struct temp_edges get_edges_point_to_point(std::vector<vertex*>& verts, std::vector<edge>& edges,
                                           struct obstacle* A, struct obstacle* B)  //Add line from point A to point B
{
    struct temp_edges count;
    count.temp_edges_count = 1;
    count.temp_vertices_count = 2;
    verts.push_back(add_vert(A->point, A));
    verts.push_back(add_vert(B->point, B));

    edge line;
    line.chosen = false;
    line.passed = false;
    line.type = LINEAR;
    line.pA = verts[0];
    line.pB = verts[1];
    line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
    edges.push_back(line);

    return count;
}

struct temp_edges get_edges_circle_to_circle(std::vector<vertex*>& verts, std::vector<edge>& edges,
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
    verts.push_back(    add_vert(A->point->GetX() + A->r * cos(angle+outer_angle/2),
                        A->point->GetY() + A->r * sin(angle+outer_angle/2),
                        A, angle+outer_angle/2));
    verts.push_back(    add_vert(B->point->GetX() + B->r * cos(angle+outer_angle/2),
                        B->point->GetY() + B->r * sin(angle+outer_angle/2),
                        B, angle+outer_angle/2));
    verts.push_back(    add_vert(A->point->GetX() + A->r * cos(angle-outer_angle/2),
                        A->point->GetY() + A->r * sin(angle-outer_angle/2),
                        A, angle-outer_angle/2));
    verts.push_back(    add_vert(B->point->GetX() + B->r * cos(angle-outer_angle/2),
                        B->point->GetY() + B->r * sin(angle-outer_angle/2),
                        B, angle-outer_angle/2));

    //Adding edges
    struct edge line;
    line.chosen = false;
    line.passed = false;
    line.type = LINEAR;
    line.pA = verts[0];
    line.pB = verts[1];
    line.length = distance(*(line.pA->point), *(line.pB->point));
    edges.push_back(line);
    line.pA = verts[2];
    line.pB = verts[3];
    line.length = distance(*(line.pA->point), *(line.pB->point));
    edges.push_back(line);

    if (dist > (A->r + B->r)) //If circles dont intersect
    {

        count.temp_edges_count = 4;
        count.temp_vertices_count = 8;

        double inner_angle = safe_acos((A->r + B->r) / dist);  //Angle between inner tangents
        verts.push_back(    add_vert(A->point->GetX() + A->r * cos(angle+inner_angle),
                            A->point->GetY() + A->r * sin(angle+inner_angle),
                            A, angle+inner_angle));
        verts.push_back(    add_vert(B->point->GetX() + B->r * cos(M_PI+angle+inner_angle),
                            B->point->GetY() + B->r * sin(M_PI+angle+inner_angle),
                            B, M_PI+angle+inner_angle));

        verts.push_back(    add_vert(A->point->GetX() + A->r * cos(angle-inner_angle),
                            A->point->GetY() + A->r * sin(angle-inner_angle),
                            A, angle-inner_angle));
        verts.push_back(    add_vert(B->point->GetX() + B->r * cos(M_PI+angle-inner_angle),
                            B->point->GetY() + B->r * sin(M_PI+angle-inner_angle),
                            B, M_PI+angle-inner_angle));

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

struct temp_edges get_edges_point_to_circle(std::vector<vertex*>& verts, std::vector<edge>& edges,
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

    verts.push_back(add_vert(pt->point->GetX(),
                             pt->point->GetY(), pt));
    verts.push_back(add_vert(pt->point->GetX(),
                             pt->point->GetY(), pt));
    verts.push_back(add_vert(circle->point->GetX() + circle->r * cos(M_PI+angle-outer_angle/2),
                             circle->point->GetY() + circle->r * sin(M_PI+angle-outer_angle/2),
                             circle, M_PI+angle-outer_angle/2));
    verts.push_back(add_vert(circle->point->GetX() + circle->r * cos(M_PI+angle+outer_angle/2),
                             circle->point->GetY() + circle->r * sin(M_PI+angle+outer_angle/2),
                             circle, M_PI+angle+outer_angle/2));
    struct edge line;
    line.chosen = false;
    line.passed = false;
    line.type = LINEAR;
    line.pA = verts[0];
    line.pB = verts[2];
    line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
    edges.push_back(line);
    line.pA = verts[1];
    line.pB = verts[3];
    line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
    edges.push_back(line);
    return count;
}

struct temp_edges get_edges_point_to_polygon(std::vector<vertex*>& verts, std::vector<edge>& edges,
                                            struct obstacle* A, struct obstacle* B)  //Add from 0 to 2N lines from point A to polygon B (or vice-versa)
{
    double dist, angle;
    struct edge line;
    line.chosen = false;
    line.passed = false;
    line.type = LINEAR;
    obstacle* pt;   //Point
    obstacle* poly;   //Polygon
    if (A->shape == POLYGON) //Deciding who is who
    {
        poly = A;
        pt = B;
    }
    else
    {
        poly = B;
        pt = A;
    }
    struct temp_edges count;
    count.temp_vertices_count = 0;
    count.temp_edges_count = 0;

    for (int i = 0; i < poly->num; i++)
    {
        if (poly->outline[i].type == LINEAR)
            continue;
        edge* curr_edge = poly->outline + i;
        dist = distance(pt->point->GetX(), pt->point->GetY(), curr_edge->cx, curr_edge->cy);
        angle = direction_to_point(curr_edge->cx, curr_edge->cy, pt->point->GetX(), pt->point->GetY());
        double outer_angle = safe_acos(curr_edge->r / dist);  //Angle between outer tangents
        //Trying to build first tangent
        double tang_angle = Angle(angle + outer_angle).GetR(); //Angle between center of an arc and tangent
        if (angle_between(
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aA.GetR() : curr_edge->aB.GetR(),
                    tang_angle,
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aB.GetR() : curr_edge->aA.GetR()))
        {
            int j = verts.size();
            verts.push_back(add_vert(pt->point->GetX(),
                                     pt->point->GetY(), pt));
            verts.push_back(add_vert(curr_edge->cx + curr_edge->r * cos(tang_angle),
                                     curr_edge->cy + curr_edge->r * sin(tang_angle),
                                     poly, tang_angle));
            line.pA = verts[j];
            verts[j+1]->poly_i = i;
            line.pB = verts[j+1];
            line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
            edges.push_back(line);
            count.temp_vertices_count += 2;
            count.temp_edges_count++;
        }
        //Trying to build second tangent
        tang_angle = Angle(angle - outer_angle).GetR(); //Angle between center of an arc and tangent
        if (angle_between(
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aA.GetR() : curr_edge->aB.GetR(),
                    tang_angle,
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aB.GetR() : curr_edge->aA.GetR()))
        {
            int j = verts.size();
            verts.push_back(add_vert(pt->point->GetX(),
                                     pt->point->GetY(), pt));
            verts.push_back(add_vert(curr_edge->cx + curr_edge->r * cos(tang_angle),
                                     curr_edge->cy + curr_edge->r * sin(tang_angle),
                                     poly, tang_angle));        
            line.pA = verts[j];
            verts[j+1]->poly_i = i;
            line.pB = verts[j+1];
            line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
            edges.push_back(line);
            count.temp_vertices_count += 2;
            count.temp_edges_count++;
        }
    }
    return count;
}

struct temp_edges get_edges_circle_to_polygon(std::vector<vertex*>& verts, std::vector<edge>& edges,
                                                struct obstacle* A, struct obstacle* B)  //Add from 0 to 4N lines from circle A to polygon B (or vice-versa)
{
    double dist, angle;
    struct edge line;
    line.chosen = false;
    line.passed = false;
    line.type = LINEAR;
    obstacle* circle;   //Circle
    obstacle* poly;     //Polygon
    if (A->shape == POLYGON) //Deciding who is who
    {
        poly = A;
        circle = B;
    }
    else
    {
        poly = B;
        circle = A;
    }
    struct temp_edges count;
    count.temp_vertices_count = 0;
    count.temp_edges_count = 0;

    for (int i = 0; i < poly->num; i++)
    {
        if (poly->outline[i].type == LINEAR)
            continue;
        edge* curr_edge = poly->outline + i;
        dist = distance(circle->point->GetX(), circle->point->GetY(), curr_edge->cx, curr_edge->cy);
        if (dist < fabs(circle->r - curr_edge->r))  //Check whether circle contain this arc
            continue;
        angle = direction_to_point(circle->point->GetX(), circle->point->GetY(), curr_edge->cx, curr_edge->cy);
        double outer_angle = safe_acos(fabs(circle->r - curr_edge->r) / dist);  //Angle between outer tangents
        //Trying to build first outer tangent
        double tang_angle = Angle(angle + outer_angle).GetR(); //Angle between center of an arc and tangent
        if (angle_between(
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aA.GetR() : curr_edge->aB.GetR(),
                    tang_angle,
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aB.GetR() : curr_edge->aA.GetR()))
        {
            int j = verts.size();
            verts.push_back(add_vert(circle->point->GetX() + circle->r * cos(tang_angle),
                                     circle->point->GetY() + circle->r * sin(tang_angle),
                                     circle, tang_angle));
            verts.push_back(add_vert(curr_edge->cx + curr_edge->r * cos(tang_angle),
                                     curr_edge->cy + curr_edge->r * sin(tang_angle),
                                     poly, tang_angle));
            line.pA = verts[j];
            verts[j+1]->poly_i = i;
            line.pB = verts[j+1];
            line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
            edges.push_back(line);
            count.temp_vertices_count += 2;
            count.temp_edges_count++;
        }
        //Trying to build second outer tangent
        tang_angle = Angle(angle - outer_angle).GetR(); //Angle between center of an arc and tangent
        if (angle_between(
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aA.GetR() : curr_edge->aB.GetR(),
                    tang_angle,
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aB.GetR() : curr_edge->aA.GetR()))
        {
            int j = verts.size();
            verts.push_back(add_vert(circle->point->GetX() + circle->r * cos(tang_angle),
                                     circle->point->GetY() + circle->r * sin(tang_angle),
                                     circle, tang_angle));
            verts.push_back(add_vert(curr_edge->cx + curr_edge->r * cos(tang_angle),
                                     curr_edge->cy + curr_edge->r * sin(tang_angle),
                                     poly, tang_angle));
            line.pA = verts[j];
            verts[j+1]->poly_i = i;
            line.pB = verts[j+1];
            line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
            edges.push_back(line);
            count.temp_vertices_count += 2;
            count.temp_edges_count++;
        }
        //=== Buildind inner tangents ===
        if (dist <= (circle->r + curr_edge->r)) //If circles intersects - there is no inner bitangents
            continue;
        double inner_angle = safe_acos((circle->r + curr_edge->r) / dist);
        //Trying to build first inner tangent
        tang_angle = Angle(angle + inner_angle).GetR(); //Angle between center of an arc and tangent
        double poly_tang_angle = Angle(tang_angle + M_PI).GetR();   //Same angle, but inverted
        if (angle_between(
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aA.GetR() : curr_edge->aB.GetR(),
                    poly_tang_angle,
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aB.GetR() : curr_edge->aA.GetR()))
        {
            int j = verts.size();
            verts.push_back(add_vert(circle->point->GetX() + circle->r * cos(tang_angle),
                                     circle->point->GetY() + circle->r * sin(tang_angle),
                                     circle, tang_angle));
            verts.push_back(add_vert(curr_edge->cx + curr_edge->r * cos(poly_tang_angle),
                                     curr_edge->cy + curr_edge->r * sin(poly_tang_angle),
                                     poly, poly_tang_angle));
            line.pA = verts[j];
            verts[j+1]->poly_i = i;
            line.pB = verts[j+1];
            line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
            edges.push_back(line);
            count.temp_vertices_count += 2;
            count.temp_edges_count++;
        }
        //Trying to build second inner tangent
        tang_angle = Angle(angle - inner_angle).GetR();     //Angle between center of an arc and tangent
        poly_tang_angle = Angle(tang_angle + M_PI).GetR();  //Same angle, but inverted
        if (angle_between(
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aA.GetR() : curr_edge->aB.GetR(),
                    poly_tang_angle,
                    curr_edge->direction == COUNTERCLOCKWISE ? curr_edge->aB.GetR() : curr_edge->aA.GetR()))
        {
            int j = verts.size();
            verts.push_back(add_vert(circle->point->GetX() + circle->r * cos(tang_angle),
                                     circle->point->GetY() + circle->r * sin(tang_angle),
                                     circle, tang_angle));
            verts.push_back(add_vert(curr_edge->cx + curr_edge->r * cos(poly_tang_angle),
                                     curr_edge->cy + curr_edge->r * sin(poly_tang_angle),
                                     poly, poly_tang_angle));
            line.pA = verts[j];
            verts[j+1]->poly_i = i;
            line.pB = verts[j+1];
            verts[j+1]->poly_i = i;
            line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
            edges.push_back(line);
            count.temp_vertices_count += 2;
            count.temp_edges_count++;
        }
        //===End of adding bitangents===
    }
    return count;
}

struct temp_edges get_edges_polygon_to_polygon(std::vector<vertex*>& verts, std::vector<edge>& edges,
                                                struct obstacle* A, struct obstacle* B)  //Add from 0 to 4MN lines from polygon A to polygon B
{
    double dist, angle;
    struct edge line;   //Template of edge
    line.chosen = false;
    line.passed = false;
    line.type = LINEAR;

    struct temp_edges count;    //Total count of new vertices and edges
    count.temp_vertices_count = 0;
    count.temp_edges_count = 0;


    for (int i = 0; i < A->num; i++)
    {
        for (int j = 0; j < B->num; j++)
        {
            if (A->outline[i].type == LINEAR || B->outline[j].type == LINEAR)
                continue;
            edge* a_edge = A->outline + i;
            edge* b_edge = B->outline + j;

            dist = distance(a_edge->cx, a_edge->cy, b_edge->cx, b_edge->cy);
            if (dist < fabs(a_edge->r - b_edge->r))  //Check whether arcs contain each other
                continue;
            angle = direction_to_point(a_edge->cx, a_edge->cy, b_edge->cx, b_edge->cy);
            double outer_angle = safe_acos(-fabs(a_edge->r - b_edge->r) / dist);  //Angle between outer tangents
            //Trying to build first outer tangent
            double tang_angle = Angle(angle + outer_angle).GetR(); //Angle between center of an arc and tangent
            if (angle_between(
                        a_edge->direction == COUNTERCLOCKWISE ? a_edge->aA.GetR() : a_edge->aB.GetR(),
                        tang_angle,
                        a_edge->direction == COUNTERCLOCKWISE ? a_edge->aB.GetR() : a_edge->aA.GetR())
                && angle_between(
                        b_edge->direction == COUNTERCLOCKWISE ? b_edge->aA.GetR() : b_edge->aB.GetR(),
                        tang_angle,
                        b_edge->direction == COUNTERCLOCKWISE ? b_edge->aB.GetR() : b_edge->aA.GetR()))
            {
                int k = verts.size();
                verts.push_back(add_vert(a_edge->cx + a_edge->r * cos(tang_angle),
                                         a_edge->cy + a_edge->r * sin(tang_angle),
                                         A, tang_angle));
                verts.push_back(add_vert(b_edge->cx + b_edge->r * cos(tang_angle),
                                         b_edge->cy + b_edge->r * sin(tang_angle),
                                         B, tang_angle));
                line.pA = verts[k];
                verts[k]->poly_i = i;
                line.pB = verts[k+1];
                verts[k+1]->poly_i = j;
                line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                edges.push_back(line);
                count.temp_vertices_count += 2;
                count.temp_edges_count++;
            }
            //Trying to build second outer tangent
            tang_angle = Angle(angle - outer_angle).GetR(); //Angle between center of an arc and tangent
            if (angle_between(
                        a_edge->direction == COUNTERCLOCKWISE ? a_edge->aA.GetR() : a_edge->aB.GetR(),
                        tang_angle,
                        a_edge->direction == COUNTERCLOCKWISE ? a_edge->aB.GetR() : a_edge->aA.GetR())
                && angle_between(
                        b_edge->direction == COUNTERCLOCKWISE ? b_edge->aA.GetR() : b_edge->aB.GetR(),
                        tang_angle,
                        b_edge->direction == COUNTERCLOCKWISE ? b_edge->aB.GetR() : b_edge->aA.GetR()))
            {
                int k = verts.size();
                verts.push_back(add_vert(a_edge->cx + a_edge->r * cos(tang_angle),
                                         a_edge->cy + a_edge->r * sin(tang_angle),
                                         A, tang_angle));
                verts.push_back(add_vert(b_edge->cx + b_edge->r * cos(tang_angle),
                                         b_edge->cy + b_edge->r * sin(tang_angle),
                                         B, tang_angle));
                line.pA = verts[k];
                verts[k]->poly_i = i;
                line.pB = verts[k+1];
                verts[k+1]->poly_i = j;
                line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                edges.push_back(line);
                count.temp_vertices_count += 2;
                count.temp_edges_count++;
            }
            //=== Buildind inner tangents ===
            if (dist <= (a_edge->r + b_edge->r)) //If circles intersects - there is no inner bitangents
                continue;
            double inner_angle = safe_acos((a_edge->r + b_edge->r) / dist);
            //Trying to build first inner tangent
            tang_angle = Angle(angle + inner_angle).GetR(); //Angle between center of an arc and tangent
            double poly_tang_angle = Angle(tang_angle + M_PI).GetR();   //Same angle, but inverted
            if (angle_between(
                        a_edge->direction == COUNTERCLOCKWISE ? a_edge->aA.GetR() : a_edge->aB.GetR(),
                        tang_angle,
                        a_edge->direction == COUNTERCLOCKWISE ? a_edge->aB.GetR() : a_edge->aA.GetR())
                && angle_between(
                        b_edge->direction == COUNTERCLOCKWISE ? b_edge->aA.GetR() : b_edge->aB.GetR(),
                        poly_tang_angle,
                        b_edge->direction == COUNTERCLOCKWISE ? b_edge->aB.GetR() : b_edge->aA.GetR()))
            {
                int k = verts.size();
                verts.push_back(add_vert(a_edge->cx + a_edge->r * cos(tang_angle),
                                         a_edge->cy + a_edge->r * sin(tang_angle),
                                         A, tang_angle));
                verts.push_back(add_vert(b_edge->cx + b_edge->r * cos(poly_tang_angle),
                                         b_edge->cy + b_edge->r * sin(poly_tang_angle),
                                         B, poly_tang_angle));
                line.pA = verts[k];
                verts[k]->poly_i = i;
                line.pB = verts[k+1];
                verts[k+1]->poly_i = j;
                line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                edges.push_back(line);
                count.temp_vertices_count += 2;
                count.temp_edges_count++;
            }
            //Trying to build second inner tangent
            tang_angle = Angle(angle - inner_angle).GetR();     //Angle between center of an arc and tangent
            poly_tang_angle = Angle(tang_angle + M_PI).GetR();  //Same angle, but inverted
            if (angle_between(
                        a_edge->direction == COUNTERCLOCKWISE ? a_edge->aA.GetR() : a_edge->aB.GetR(),
                        tang_angle,
                        a_edge->direction == COUNTERCLOCKWISE ? a_edge->aB.GetR() : a_edge->aA.GetR())
                && angle_between(
                        b_edge->direction == COUNTERCLOCKWISE ? b_edge->aA.GetR() : b_edge->aB.GetR(),
                        poly_tang_angle,
                        b_edge->direction == COUNTERCLOCKWISE ? b_edge->aB.GetR() : b_edge->aA.GetR()))
            {
                int k = verts.size();
                verts.push_back(add_vert(a_edge->cx + a_edge->r * cos(tang_angle),
                                         a_edge->cy + a_edge->r * sin(tang_angle),
                                         A, tang_angle));
                verts.push_back(add_vert(b_edge->cx + b_edge->r * cos(poly_tang_angle),
                                         b_edge->cy + b_edge->r * sin(poly_tang_angle),
                                         B, poly_tang_angle));
                line.pA = verts[k];
                verts[k]->poly_i = i;
                line.pB = verts[k+1];
                verts[k+1]->poly_i = j;
                line.length = sqrt(distance2(*(line.pA->point), *(line.pB->point)));
                edges.push_back(line);
                count.temp_vertices_count += 2;
                count.temp_edges_count++;
            }
            //===End of adding bitangents===
        }
    }
    return count;
}


void add_line(obstacle* objects, int i, int j, graph* result, std::mutex& res_guard, int count, int _start)
{
    std::vector<edge> temp_edges;
    std::vector<vertex*> temp_verts;
    struct temp_edges temp_count;

    bool targets = false;

    obstacle* A = objects + i;
    obstacle* B = objects + j;
    if (A->shape == POINT && B->shape == POINT) //Point-Point - only one line. Should be called once
    {
        temp_count = get_edges_point_to_point(temp_verts, temp_edges, A, B);
        targets = true;
    }
    else if (A->shape == CIRCLE && B->shape == CIRCLE)  //Circle-Circle - 0, 2 or 4 lines
    {
        temp_count = get_edges_circle_to_circle(temp_verts, temp_edges, A, B);
    }
    else if (A->shape == POLYGON && B->shape == POLYGON)    //Poly-Poly - from 0 to 4N*M lines
    {
        temp_count = get_edges_polygon_to_polygon(temp_verts, temp_edges, A, B);
    }
    else if (A->shape == POINT || B->shape == POINT)
    {
        if (A->shape == CIRCLE || B->shape == CIRCLE)
            temp_count = get_edges_point_to_circle(temp_verts, temp_edges, A, B);   //Point-Circle - 0 or 2 lines
        else
            temp_count = get_edges_point_to_polygon(temp_verts, temp_edges, A, B);   //Point-Poly - up to 2N lines
    }
    else    //Circle-Poly - from 0 to 4N lines
    {
        temp_count = get_edges_circle_to_polygon(temp_verts, temp_edges, A, B);
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
            if (objects[z].shape == CIRCLE) //Intersection with circles
            {
                CircleCollider crc(objects[z].point->GetX(),
                                   objects[z].point->GetY(),
                                   objects[z].r);
                if (lin.CheckCollision(&crc))
                {
                    inter = true;
                    break;
                }
            }
            else    //Intersection with polygons
            {
                for (uint _i = 0; _i < objects[z].num; _i++)
                {
                    edge* edg = objects[z].outline + _i;
                    if (edg->type == LINEAR)
                    {
                        LineCollider lc(edg->A.GetX(), edg->A.GetY(),
                                        edg->B.GetX(), edg->B.GetY());
                        if (lin.CheckCollision(&lc))
                        {
                            inter = true;
                            break;
                        }
                    }
                    else    //Intersection with arcs
                    {
                        Line edge_ln;
                        edge_ln.Set(temp_edges[k].pA->point->GetX(), temp_edges[k].pA->point->GetY(),
                                 temp_edges[k].pB->point->GetX(), temp_edges[k].pB->point->GetY());
                        if (arc_line_collision(Circle(edg->cx, edg->cy, edg->r),
                                               edge_ln,
                                               edg->direction == COUNTERCLOCKWISE ? edg->aA.GetR() : edg->aB.GetR(),
                                               edg->direction == COUNTERCLOCKWISE ? edg->aB.GetR() : edg->aA.GetR()))
                        {
                            inter = true;
                            break;
                        }
                    }
                }
            }
        }
        if (inter && !targets)    //Don't add intersected edges (and their vertices)
        {
            delete temp_edges[k].pA;
            delete temp_edges[k].pB;
            continue;
        }
        //Merge vertices
        double eps = 4; //Radius of merging
        bool first = true;  //Is first vertex edge not merged yet
        bool sec = true;

        std::lock_guard<std::mutex> a (res_guard); // [ ACQUIRING LOCK ] <= bottleneck

//        qDebug() << i << " : " << j << ", ";

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

graph* build_graph_thread(obstacle* objects, int count, uint _start, uint  _end, uint delte) //Building graph, but multithreaded
{
    graph* result = new graph();

    std::mutex res_guard;

    std::vector<std::future<int>> futures;    //std::async version

    //Adding obstacles
    for (int i = 0; i < count; i++) //Getting all connections between all obstacles
    {
        for (int j = i+1; j < count; j++)
        {
            futures.push_back(  //std::async version
                std::async(std::launch::async, [&objects, i, j, result, &res_guard, count, _start] {
                    add_line(objects, i, j, result, res_guard, count, _start);
                    return 0;}
                )
            );

//            QThreadPool::globalInstance()->start(   //QThread version
//                QRunnable::create(
//                    [&objects, i, j, result, &res_guard, count, _start] {
//                        add_line(objects, i, j, result, res_guard, count, _start);}
//                )
//            );


        }
    }

//    QThreadPool::globalInstance()->waitForDone();   //QThread version

    for (std::future<int>& fut : futures)   //std::async version
    {
        fut.wait();
    }

    //Connecting lines with arcs
    std::vector<vertex*> brothers;  //Vertices lying on the same obstacle
    for (int i = 0; i < count; i++)
    {
        //"Brothers" search
        if (objects[i].shape != CIRCLE && objects[i].shape != POLYGON)
            continue;
        if (objects[i].shape == CIRCLE) //DIVIDE INTO FUNCTION
        {
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
            arc.passed = false;
            arc.type = ARC_CIRCLE;
            arc.direction = COUNTERCLOCKWISE;
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
        }
        else if (objects[i].shape == POLYGON) //DIVIDE INTO FUNCTION
        {
            for (unsigned int j = 0; j < result->vertices.size(); j++)   //Looking for every vertex with this obstacle as parent
            {
                if (result->vertices[j]->parent != objects+i)
                    continue;
                brothers.push_back(result->vertices[j]);
                assert(result->vertices[j]->poly_i != -1);  //If vertex is on polygon obstacle, it should have index
                for (int k = brothers.size()-1; k > 0; k--) //Sort by edge index
                {
                    if (brothers[k]->poly_i < brothers[k-1]->poly_i)
                    {
                        vertex* temp = brothers[k];
                        brothers[k] = brothers[k-1];
                        brothers[k-1] = temp;
                    }
                }
            }
            unsigned int _i = 0;    //Number in brothers vector
            edge* obst_outline = objects[i].outline;
            vertex* first = nullptr;
            if (obst_outline[0].type == LINEAR)
                first = add_vert(&obst_outline[0].A, objects+i);
            else
                first = add_vert(obst_outline[0].cx + obst_outline[0].r * cos(obst_outline[0].aA.GetR()),
                        obst_outline[0].cy + obst_outline[0].r * sin(obst_outline[0].aA.GetR()),
                        objects+i);
            result->vertices.push_back(first);
            vertex* last = first;
            for (int j = 0; j < objects[i].num; j++)
            {
                vertex* next = nullptr;

                if (obst_outline[j].type == LINEAR) //Duplicating linear edge into graph
                {
                    if (j == objects[i].num - 1)
                    {
                        next = first;
                        next->point->MoveTo(obst_outline[j].B.GetX(), obst_outline[j].B.GetY());
                    }
                    else
                    {
                        next = add_vert(&obst_outline[j].B, objects+i);
                        result->vertices.push_back(next);
                    }
                    edge line;
                    line.chosen = false;
                    line.passed = false;
                    line.type = LINEAR;
                    line.pA = last;
                    line.pB = next;
                    line.length = distance(last->point->GetX(), last->point->GetY(), next->point->GetX(), next->point->GetY());
                    //The end
                    last = next;
                    result->edges.push_back(line);
                    continue;
                }
                //Adding arcs
                edge arc;   //Template of arc
                arc.type = ARC_CIRCLE;
                arc.r = obst_outline[j].r;
                arc.cx = obst_outline[j].cx;
                arc.cy = obst_outline[j].cy;
                arc.direction = COUNTERCLOCKWISE;
                arc.passed = false;
                arc.chosen = false;

                std::vector<vertex*> temp_vec;  //Vector of vertices on current arc
                for (int k = _i; k < brothers.size(); k++)
                {
                    if (brothers[k]->poly_i == j)
                        temp_vec.push_back(brothers[k]);
                    else
                    {
                        _i = k; //End of current segment
                        break;
                    }
                }

                if (j == objects[i].num - 1)
                {
                    next = first;
                    next->angle = obst_outline[j].aB;
                }
                else
                {
                    next = add_vert(obst_outline[j].cx + obst_outline[j].r * cos(obst_outline[j].aB.GetR()),
                            obst_outline[j].cy + obst_outline[j].r * sin(obst_outline[j].aB.GetR()),
                            objects+i);
                    next->angle = obst_outline[j].aB;
                    result->vertices.push_back(next);
                }
                last->angle = obst_outline[j].aA;

                if (temp_vec.size() == 0)   //If there is no vertices on this arc
                {
                    arc.aA = obst_outline[j].aA;
                    arc.aB = obst_outline[j].aB;
                    arc.pA = last;
                    arc.pB = next;
                    if (arc.aB.GetR() < arc.aA.GetR())  //Getting length
                        arc.length = (arc.aB.GetR() - arc.aA.GetR() + 2*M_PI) * arc.r;
                    else
                        arc.length = fabs((arc.aB.GetR() - arc.aA.GetR()) * arc.r);
                    result->edges.push_back(arc);
                }
                else
                {
                    double startA = obst_outline[j].aA.GetR();  //Start angle of arc [0; 2PI]
                    double endA = obst_outline[j].aB.GetR();    //End angle of arc [0; 4PI]
                    if (endA < startA)
                        endA += 2 * M_PI;
                    //Sort by angle
                    std::sort(temp_vec.begin(), temp_vec.end(),
                              [&] (vertex* a, vertex* b) -> bool
                    {
                        double aA = a->angle.GetR() > startA ? a->angle.GetR() : a->angle.GetR() + 2*M_PI;
                        double bA = b->angle.GetR() > startA ? b->angle.GetR() : b->angle.GetR() + 2*M_PI;
                        return  aA < bA;
                    });
                    //First arc
                    arc.pA = last;
                    arc.pB = temp_vec[0];
                    arc.aA = arc.pA->angle;
                    arc.aB = arc.pB->angle;
                    if (arc.aB.GetR() < arc.aA.GetR())
                        arc.length = (arc.aB.GetR() - arc.aA.GetR() + 2*M_PI) * arc.r;
                    else
                        arc.length = fabs((arc.aB.GetR() - arc.aA.GetR()) * arc.r);
                    result->edges.push_back(arc);
                    //Middle connections
                    for (int k = 0; k < temp_vec.size()-1; k++)
                    {
                        arc.pA = temp_vec[k];
                        arc.pB = temp_vec[k+1];
                        arc.aA = arc.pA->angle;
                        arc.aB = arc.pB->angle;
                        if (arc.aB.GetR() < arc.aA.GetR())
                            arc.length = (arc.aB.GetR() - arc.aA.GetR() + 2*M_PI) * arc.r;
                        else
                            arc.length = fabs((arc.aB.GetR() - arc.aA.GetR()) * arc.r);
                        result->edges.push_back(arc);
                    }
                    //Last connection
                    arc.pA = temp_vec[temp_vec.size()-1];
                    arc.pB = next;
                    arc.aA = arc.pA->angle;
                    arc.aB = arc.pB->angle;
                    if (arc.aB.GetR() < arc.aA.GetR())
                        arc.length = (arc.aB.GetR() - arc.aA.GetR() + 2*M_PI) * arc.r;
                    else
                        arc.length = fabs((arc.aB.GetR() - arc.aA.GetR()) * arc.r);
                    result->edges.push_back(arc);
                }
                last = next;
            }
        }
        //Clearing
        brothers.clear();
    }
    return result;
}
