#ifndef GRAPH_STRUCT_H
#define GRAPH_STRUCT_H

#include "objects.h"

enum EDGE_TYPE {LINEAR,         //Straight line
                ARC_CIRCLE,     //Arc on circle
                ARC_ELLIPSE};   //Arc on ellipse [NOT IMPLEMENTED]

enum OBJ_SHAPE {POINT,      //No obstacle - used for start and end points
                CIRCLE,     //Circle
                POLYGON};   //Combination of arcs and lines

enum SIDE {LEFT,
           RIGHT,
           FORWARD};

enum DIRECTION {NODIRECTION,
                CLOCKWISE,
                COUNTERCLOCKWISE};

struct edge;

struct obstacle //[TODO] Replace with classes - obstacle with outline around it
{
    OBJ_SHAPE shape;
    Point* point;   //Center coords [POINT, CIRCLE]
    unsigned int num;   //Number of edges [POLYGON]
    edge* outline;  //Array of linear and circular edges [POLYGON]
    double r;       //Radius [CIRCLE]
};

class ObstacleMap  //Map of obstacles, found by distance sensors
{
public:
    std::vector< std::vector<Point> > obstacles;

    ObstacleMap();
    ~ObstacleMap();

    void AddPoint(const Point &pt, int d);
    void DeletePoint(Point& pt);

    void AddLine(const Point& pt1, const Point& pt2, double d);

    int Size();

    obstacle GetObstacle(int i, double r);

    void ToAlpha(int i, int d);


    void Show();
};

struct vertex
{
    Point* point;
    obstacle* parent;   //Obstacle, where point is lied on
    Angle angle;        //Angle of vector between center of parent and vertex
    int poly_i;         //Index of edge, where point is lied on
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
    Point A, B;   //Start and end points

    Angle aA, aB;  //Start and end angles
    DIRECTION direction;   //Direction of an arc - 1 - clockwise, -1 - counter-clockwise
    double r;  //Radius
    double cx, cy;  //Center coords
    double length;  //Length of edge
    bool chosen;    //Is this edge part of path
    bool passed;    //Was this edge already passed
};

#endif // GRAPH_STRUCT_H
