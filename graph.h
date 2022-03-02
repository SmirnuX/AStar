#ifndef GRAPH_H
#define GRAPH_H

#define MAX_TEMP_EDGES 4
#define MAX_TEMP_VERTS 8
constexpr uint MAX_PATH_COST = 1000000;

#include "objects.h"
#include "collision.h"


extern QPixmap* picture;

enum EDGE_TYPE {LINEAR,         //Straight line
                ARC_CIRCLE,     //Arc on circle
                ARC_ELLIPSE};   //Arc on ellipse [NOT IMPLEMENTED]

enum OBJ_SHAPE {POINT,      //No obstacle - used for start and end points
                CIRCLE,     //Circle
                ELLIPSE};   //Ellipse [NOT IMPLEMENTED]

struct obstacle
{
    OBJ_SHAPE shape;
    Point* point;
    double rA, rB;
};

struct vertex
{
    Point* point;
    obstacle* parent;   //Obstacle, where point is lied on
    Angle angle;       //Angle of vector between center of parent and vertex
    double cost;
    double dist;    //Distance to end point
};

struct graph_cmp
{
    vertex* ptr;

    graph_cmp(vertex* _ptr);
    graph_cmp();
};

struct fnctor
{
    bool operator() (graph_cmp const& lhs, graph_cmp const& rhs);   //Priority comparing
};

struct edge
{
    EDGE_TYPE type; //Type of edge
    vertex *pA, *pB;   //Vertices, connected by that edge
    Point A, B;   //Star and end points

    Angle aA, aB;  //Start and end angles
    double direction;   //Direction of an arc - 1 - clockwise, -1 - counter-clockwise
    double rA, rB;  //Radiuses
    double cx, cy;  //Center coords
    double length;  //Length of edge
    bool chosen;    //Is this edge part of path
    bool passed;    //Was this edge already passed
};

class graph
{
private:
    bool found_way;
public:
    std::vector<vertex*> vertices;
    std::vector<edge> edges;

    graph();
    void clear();
    ~graph();
    void AStar();
    int start;  //Indices of start and end points in vector
    int end;
    bool IsWay();
    void Show();
    void Show(int x, int y);

};

struct temp_edges
{
    int temp_vertices_count;
    int temp_edges_count;
};


vertex* add_vert(double x, double y, obstacle* _parent, Angle _angle = Angle(0));
vertex* add_vert(Point* pt, obstacle* _parent, Angle _angle = Angle(0));
graph* build_graph(obstacle* objects, int count,  uint _start=0, uint _end=1, uint delte = 10);

struct temp_edges get_edges_point_to_point(struct vertex** verts, struct edge* edges,
                                           struct obstacle* A, struct obstacle* B);  //Add line from point A to point B

struct temp_edges get_edges_circle_to_circle(struct vertex** verts, struct edge* edges,
                                           struct obstacle* A, struct obstacle* B);  //Add [0,2,4] lines from circle A to circle B

struct temp_edges get_edges_point_to_circle(struct vertex** verts, struct edge* edges,
                                           struct obstacle* A, struct obstacle* B);  //Add 0 or 2 lines from point A to circle B (or vice-versa)

#endif // GRAPH_H
