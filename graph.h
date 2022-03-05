#ifndef GRAPH_H
#define GRAPH_H

#define MAX_TEMP_EDGES 4
#define MAX_TEMP_VERTS 8
constexpr unsigned int MAX_PATH_COST = 1000000;

#include "objects.h"
#include "collision.h"
#include "graph_struct.h"

extern QPixmap* picture;



/* GET OBSTACLE OUTLINE
 * GET BITANGENTS
 * CHECK FOR INTERSECTIONS BETWEEN OBSTACLES
 * */


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
