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

    int target_x;
    int target_y;
    bool PAUSE;
    bool UI_ACTIVE;
    uimode UI_MODE;
    Ui_DebugMenu* Menu;
    obstacle* obst = nullptr;
    uint obst_num = 0;

    //UI properties
    static const int SPEED_GUI_SIZE = 240;
    double speeds[SPEED_GUI_SIZE];
    double max_speeds[SPEED_GUI_SIZE];

    //Editor properties
    Point* editing; //Point, that is moved by mouse in current moment
    Entity* chosen; //Whole entity
    bool is_building_poly;  //Is new polygon building now?
    std::vector<Point> current_pts; //New polygon pts

public:
    Car* player;  //Controlled car
    Box* box;
    game(int w, int h, QWidget *parent = 0);
    ~game();

    void initWindow();

    QTimer* timer;
    bool event(QEvent* ev);    //Event handler
    void mousePressEvent(QMouseEvent *event);   //Mouse event handler
    void paintEvent(QPaintEvent* ); //Drawing buffer
    void uiUpdate();
    void showUI();

    bool loadLevel(const QJsonDocument &json);
    void saveLevel(QJsonDocument& json);
    void clearLevel();
    void initLevel();

    int GetW();
    int GetH();
public slots:
    void game_update(); //One tact of game
    void loadFile();
    void saveFile();
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
