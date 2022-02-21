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
#include "tanks.h"
#include "list.h"
#include "add_math.h"

extern QFile* path_log;

extern QPixmap* picture;
extern bool SHOW_COLLIDERS;

class Tank;
class EntityStack;

class game : public QMainWindow //Main window
{
    Q_OBJECT
private:
    int width;  //Window width
    int height; //Height width
    bool key[7];    //Input buffer
    graph* path_graph;
    EntityStack *visible;
    int target_x;
    int target_y;
    bool PAUSE;
    bool UI_ACTIVE;
    bool SHOW_PATH;
    Ui_DebugMenu* Menu;
public:
    Tank* player;  //Player tank
    Box* box;
    game(int w, int h, QWidget *parent = 0);
    ~game();


    QTimer* timer;
    bool event(QEvent* ev);    //Event handler
    void mousePressEvent(QMouseEvent *event);   //Mouse event handler
    void paintEvent(QPaintEvent* ); //Drawing buffer
    void uiUpdate();

    int GetW();
    int GetH();
public slots:
    void game_update(); //One tact of game
};

class Path
{
public:
    int num;    //Count of segments
    int i;      //Current segment
    //Array of coords
    double* x;
    double* y;
    //Array of maximum speeds
    double* s;
    //Array of segment types
    bool* circle;
    //Array of start and end angles
    Angle* s_a;
    Angle* e_a;
    //Array of center points and radiuses
    double* c_x;
    double* c_y;
    double* c_r;
    //Final point
    double final_x;
    double final_y;
    Path(int _num, double tx, double ty);

    ~Path();
};

#endif // GAME_H
