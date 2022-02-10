#ifndef GRAPH_H
#define GRAPH_H

#define MAX_TEMP_EDGES 4
#define MAX_TEMP_VERTS 8

#include "game.h"

extern QPixmap* picture;

enum EDGE_TYPE {LINEAR,         //Прямая линия между двумя точками pA и pB
                ARC_CIRCLE,     //Дуга на окружности радиусом rA между углами aA и aB
                ARC_ELLIPSE};   //Дуга на эллипсе с радиусами rA и rB между углами aA и aB

enum OBJ_SHAPE {POINT,      //Форма препятствия
                CIRCLE,     //Окружность
                ELLIPSE};   //TODO - сделать для эллипсов

struct obstacle //Структура обьектов на поле
{
    OBJ_SHAPE shape;
    Point* point;
    double rA, rB;
};

struct vertex   //Структура вершины
{
    Point* point;
    obstacle* parent;   //Препятствие, которому "принадлежит" вершина
    double angle;       //Наклон вектора между центром parent и вершиной
    double cost;    //Стоимость
    double dist;    //Дистанция до финальной точки
};

struct graph_cmp
{
    vertex* ptr;

    graph_cmp(vertex* _ptr)
    {
        ptr = _ptr;
    }
    graph_cmp()
    {
        ptr = NULL;
    }
};

struct fnctor
{
    bool operator() (graph_cmp const& lhs, graph_cmp const& rhs)   //Оператор сравнения приоритетов
    {
        return lhs.ptr->cost > rhs.ptr->cost;
    }
};

struct edge //Структура грани
{
    EDGE_TYPE type; //Тип грани
    vertex *pA, *pB;   //Вершины, соединямые гранями - используется в графе
    Point A, B;   //Начальная и конечная координаты - используются в пути
    //Для ARC_CIRCLE и ARC_ELLIPSE
    double aA, aB;  //Начальный и конечный углы на арке
    double rA, rB;  //Радиусы эллипса (или rA - радиус круга, а rB - не используется для ARC_CIRCLE)
    double cx, cy;  //Координаты центра
    double length;  //Длина грани
    bool chosen;
    bool passed;
};

class graph //Граф
{
public:
    std::vector<vertex*> vertices;   //Вершины графа
    std::vector<edge> edges;    //Грани

    graph()
    {

    }

    void clear() //Очистка памяти
    {
        for (unsigned int i=0; i < vertices.size(); i++)
            delete vertices[i];
        vertices.clear();
        edges.clear();
    }

    ~graph()
    {
        clear();
    }

    void AStar();

    void Show();

};

vertex* add_vert(double x, double y, obstacle* _parent, double _angle=0);    //Создание вершины
vertex* add_vert(Point* pt, obstacle* _parent, double _angle=0);    //Создание вершины
graph* build_graph(obstacle* objects, int count); //Построение графа

#endif // GRAPH_H
