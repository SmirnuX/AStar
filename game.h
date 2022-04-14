#ifndef GAME_H
#define GAME_H

#include <QApplication>
#include <QMainWindow>
#include <QEvent>
#include <QPen>
#include <QKeyEvent>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <queue>
#include <vector>
#include <algorithm>
#include "ui.h"
#include "graph.h"
#include "objects.h"
#include "cars.h"
#include "list.h"
#include "add_math.h"
#include <QtCore>
#include <chrono>

extern QFile* path_log;

extern QPixmap* picture;
extern bool SHOW_COLLIDERS;

class Car;
class EntityStack;

enum uimode{
    NONE,       //Show no ui
    COLLIDERS,  //Show collision masks
    GRAPH,      //Show path graph
    PATH,       //Show built path
    OBJECTS,    //Move objects
    RADAR       //Radar data
};

class game : public QMainWindow //Main window
{
    Q_OBJECT
private:
    double player_start_x, player_start_y;
    int total_collisions;
    int width;  //Window width
    int height; //Height width
    bool key[7];    //Input buffer
    //Path graph
    graph* path_graph;
    std::chrono::duration<double, std::milli> build_time;
    std::chrono::duration<double, std::milli> pathfind_time;
    static const int SPEED_GUI_SIZE = 240;
    double speeds[SPEED_GUI_SIZE];
    double max_speeds[SPEED_GUI_SIZE];

    EntityStack *visible;
    int target_x;
    int target_y;
    bool PAUSE;
    bool UI_ACTIVE;
    uimode UI_MODE;
    Ui_DebugMenu* Menu;
    obstacle* obst = nullptr;
    uint obst_num = 0;
public:
    Car* player;  //Controlled car
    Box* box;
    game(int w, int h, QWidget *parent = 0);
    ~game();

    QTimer* timer;
    bool event(QEvent* ev);    //Event handler
    void mousePressEvent(QMouseEvent *event);   //Mouse event handler
    void paintEvent(QPaintEvent* ); //Drawing buffer
    void uiUpdate();
    void showUI();

    bool loadSave(const QJsonDocument &json);

    int GetW();
    int GetH();
public slots:
    void game_update(); //One tact of game
};

struct pathpoint
{
    double x, y;
    double s;
    Angle s_a, e_a;
    DIRECTION direction;
    bool circle;
    double c_x, c_y, c_r;
};

class Path
{
public:
    int num;    //Count of segments
    int i;      //Current segment

    struct pathpoint* pts;
    double startx, starty;
    double final_x, final_y;

    Path(int _num, double tx, double ty);
    ~Path();
};

#endif // GAME_H
