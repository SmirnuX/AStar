#ifndef OBJECTS_H
#define OBJECTS_H

//Иерархия сущностей и движимых сущностей

#include "collision.h"
#include "list.h"
#include <math.h>
#include <QThread>
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QColor>
#include <QPoint>


class Entity: public Point   //Базовый класс сущностей
{
protected:

public:
    Entity(float _x, float _y, Collider* _collider = NULL);
    Collider* collision_mask;   //Маска столкновений
    //Отрисовка
    virtual void Show() = 0; //Отрисовать сущность
    virtual void Hide() = 0; //Скрыть сущность

    //Функции, выполняющиеся каждый такт
    void EntityUpdate();  //Функция, вызываемая каждый такт. В отличии от OnStep, не должна изменяться в дочерних классах
    virtual void AutoMove();    //Движение согласно скорости и углу объекта
    virtual void OnStep();  //Действие, которое выполняет сущность каждый такт

    //Передвижение точки
    void MoveTo(float _x, float _y);  //Передвинуть в указанную позицию
    void Drag(float dx, float dy);  //Передвинуть на dx пикселей по горизонтали и на dy по вертикали

    //Получение информации
    virtual QString GetName();  //Получить название обьекта
    virtual QString GetInfo();  //Получить информацию об обьекте
    //Проверка столкновения
    //void CheckPosition(float _x, float _y);  //Проверяет столкновения при перемещении сущности в точку (_x; _y)

};

class MovingEntity: public Entity  //Класс движущихся сущностей
{
protected:
    double speed;   //Скорость объекта
    double max_speed;
    float angle;    //Угол поворота объекта
public:
    MovingEntity(float _x, float _y, double _speed = 0, float _angle = 0, Collider* _collider = NULL);

    //Функции, выполняющиеся каждый такт
    void AutoMove();    //Движение согласно скорости и углу объекта

    //Изменение свойств объекта
    void Turn(float _angle);    //Поворот сущности
    void SetSpeed(double _speed); //Установка скорости
    void SetAngle(float _angle);    //Установка угла

    //Получение информации
    QString GetName();  //Получить название обьекта
    QString GetInfo();

    //Получение значений
    double GetSpeed();
    float GetAngle();
};

class Wall: public Entity   //Вертикальная стена
{
public:
    Wall(float _x, float _y);
    ~Wall();

    void Show();
    void Hide();

    //Получение информации
    QString GetName();  //Получить название обьекта
};

class Box: public Entity    //Коробка, которую надо обьехать
{
public:
    Box(float _x, float _y);
    ~Box();
    double a;
    void Show();
    void Hide();

    //Получение информации
    QString GetName();  //Получить название обьекта
};

#endif // OBJECTS_H
