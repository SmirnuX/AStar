#ifndef GAME_H
#define GAME_H

#define EPSILON 0.001   //TODO - переделать через constexpr

//Класс, в котором и происходит вся движуха
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

class game : public QMainWindow //Основное окно, в котором запускается игра
{
    Q_OBJECT
private:
    int width;  //Ширина окна
    int height; //Высота окна
    bool key[6];    //Буфер ввода (для поддержки одновременного нажатия кнопок клавиатуры)
    //Entity **entities;   //Указатель на массив объектов, расположенных на уровне
    //int entities_count;  //Количество обьектов в массиве
    graph* path_graph;  //Граф для поиска путей
    EntityStack *visible;
    int target_x;
    int target_y;
    bool PAUSE;
    bool UI_ACTIVE;
    bool SHOW_PATH;
    Ui_DebugMenu* Menu;
public:
    Tank* player;  //Указатель на танк игрока
    Box* box;   //Указатель на коробку для обхезда ТЕСТ
    //TODO - добавить второй танк
    game(int w, int h, QWidget *parent = 0);    //Конструктор окна размером w x h пикселей
    ~game();    //TODO - сделать очистку памяти
    //Методы класса
    bool event(QEvent* ev);    //Обработчик событий
    void mousePressEvent(QMouseEvent *event);   //Обработчик событий мыши
    void paintEvent(QPaintEvent* ); //Отрисовка буфера в окне
    void uiUpdate();
    //Получение значений
    int GetW();
    int GetH();
public slots:
    //Методы класса
    void game_update(); //Действие, выполняемое каждый такт
    void player_set_path();
};

class Path
{
public:
    int num;
    int i;
    double* x;
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
