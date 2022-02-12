#ifndef GAME_H
#define GAME_H

#include <QApplication>
#include <QMainWindow>
#include <QEvent>
#include <QPen>
#include <QKeyEvent>
#include <QTime>
#include <QTimer>
#include "add_math.h"
#include <queue>
#include <vector>
#include <algorithm>
#include "list.h"
#include "collision.h"
#include "objects.h"
#include "tanks.h"
#include "ui.h"
#include "graph.h"

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
    bool key[6];    //Input buffer
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
    void player_set_path();
};

class Path
{
public:
    int num;
    int i;
    double* x;  //Array of: coords, angles and speeds on every segment of path
    double* y;
    double* a;
    double* s;
    double final_x;
    double final_y;
    Path(int _num, double tx, double ty)
    {
        num = _num;
        x = new double[_num];
        y = new double[_num];
        a = new double[_num];
        s = new double[_num];
        i = 0;
        final_x = tx;
        final_y = ty;
    }

    ~Path()
    {
        delete[] x;
        delete[] y;
        delete[] a;
        delete[] s;
    }
};

#endif // GAME_H
