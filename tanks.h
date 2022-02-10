#ifndef TANKS_H
#define TANKS_H

//Танки и снаряды

#include "game.h"

class graph;

class BaseTank: public MovingEntity //Базовый класс танка
{
protected:
    //Переменные
    float cannon_angle; //Угол поворота башни
    //Характеристики
    float FOV_distance; //Радиус зрения танка
    float FOV_angle;    //Угол зрения танка
    float tank_speed;   //Скорость движения танка
    float acc;          //Ускорение танка
    float dec;          //Торможение танка
    float rot_speed;    //Скорость поворота
    int rel_time;   //Скорость перезарядки
    //Характеристики корпуса
    int base_width; //Ширина корпуса
    int base_length;    //Длина корпуса
    int cannon_width;   //Ширина пушки
    int cannon_length;  //Длина пушки
public:
    Collider* FOV_collider;
    float reload_timeout;   //Время, оставшееся до возможности следующего выстрела
    BaseTank(float _x, float _y);
    void Show();    //Отрисовка танка
    void Hide();    //Скрытие танка
    virtual void ShowTracks() = 0;  //Отрисовка гусениц танка
    virtual void ShowBase() = 0;  //Отрисовка корпуса танка
    virtual void ShowCannon() = 0;  //Отрисовка пушки танка
    virtual void HideTracks() = 0;  //Сокрытие гусениц танка
    virtual void HideBase() = 0;  //Сокрытие корпуса танка
    virtual void HideCannon() = 0;  //Сокрытие пушки танка

    void ShowFOV(QPainter* pic_pntr, float _x, float _y); //Отрисовка поля зрения танка
    void HideFOV(QPainter* pic_pntr, float _x, float _y); //Сокрытие поля зрения танка

    virtual void Shoot() = 0;   //Выстрел
    void SetCannonAngle(double _angle); //Повернуть пушку танка

    void Accelerate()   //Ускорение
    {
        Accelerate(acc);
    }

    void Accelerate(double _acc)
    {
        SetSpeed(speed + _acc);
    }

    void Deccelerate()  //Торможение/обратный ход
    {
        Deccelerate(dec);
    }

    void Deccelerate(double _dec)
    {
        SetSpeed(speed - _dec);
    }

    void Rotate(double delta_angle)   //Поворот
    {
        if (delta_angle > rot_speed)
            delta_angle = rot_speed;
        if (delta_angle < -rot_speed)
            delta_angle = -rot_speed;
        Turn(delta_angle);
    }

    void RotateL()
    {
        Turn(rot_speed);
    }

    void RotateR()
    {
        Turn(-rot_speed);
    }

};

class Path;

class Tank: public BaseTank
{
public:
    Tank(float _x, float _y);
    ~Tank();

    Path* path;

    void Shoot();
    void OnStep();

    void ShowTracks();  //Отрисовка гусениц танка
    void ShowBase();  //Отрисовка корпуса танка
    void ShowCannon();  //Отрисовка пушки танка
    void HideTracks();  //Сокрытие гусениц танка
    void HideBase();  //Сокрытие корпуса танка
    void HideCannon();  //Сокрытие пушки танка

    void RideTo(Box* obstacle); //ТЕСТ обьезд квдарата

    void BuildPath(double tx, double ty);  //Построение ПРОСТОГО пути
    void BuildPath(double tx, double ty, Box* obstacle);    //ТЕСТ Обьезд коробки
    void graph_to_path(graph* gr);  //Построение пути на основании графа
    void ShowPath();
    void FollowPath();
    //Получение информации
    QString GetName();  //Получить название обьекта
};

class EnemyTank: public BaseTank
{
public:
    EnemyTank(float _x, float _y);
    ~EnemyTank();
    void Shoot();
    void OnStep();
    void ShowTracks();  //Отрисовка гусениц танка
    void ShowBase();  //Отрисовка корпуса танка
    void ShowCannon();  //Отрисовка пушки танка
    void HideTracks();  //Сокрытие гусениц танка
    void HideBase();  //Сокрытие корпуса танка
    void HideCannon();  //Сокрытие пушки танка

    //Получение информации
    QString GetName();  //Получить название обьекта
};

class BaseBullet: public MovingEntity
{
protected:
    Entity* parent;
public:
    BaseBullet(float _x, float _y, Entity* _parent = NULL);
};

class Bullet: public BaseBullet
{
public:
    Bullet(float _x, float _y, double _angle, Entity* _parent = NULL);
    void Show();
    void Hide();
    void OnStep();

    //Получение информации
    QString GetName();  //Получить название обьекта
};

#endif // TANKS_H
