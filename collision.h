#ifndef COLLISION_H
#define COLLISION_H
#include <QPainter>
//Обработка столкновений

class Point //Класс точки
{
protected:
    //Координаты точки
    float x;
    float y;
public:
    Point()
    {

    }

    Point(float _x, float _y);

    bool operator== (const Point &b) const;    //Равенство двух точек
    bool operator!= (const Point &b) const;
    //Передвижение точки
    virtual void MoveTo(float _x, float _y);  //Передвинуть в указанную позицию
    virtual void Drag(float dx, float dy);  //Передвинуть на dx пикселей по горизонтали и на dy по вертикали
    virtual void Turn(double angle, Point& pivot);    //Повернуть точку относительно точки pivot
    //Получение координат
    float GetX();
    float GetY();
};

class Line  //Класс линии
{
public:
    Line(Point p1, Point p2);   //Задание линии от точки p1 до p2.
    ~Line();
    void Update();  //Обновить k, b и проверить min_p и max_p
    void Turn(double angle, Point &pivot);
    double k;
    double b;
    Point* min_p;  //Левая точка (min x)
    Point* max_p;  //Правая точка (max x)
};

class Circle   //Окружность
{
public:
    Circle(float _x, float _y, float _r);   //Создание окружности
    ~Circle();
    Point* center;  //Центр окружности
    float r;    //Радиус окружности
};

class Polygon //Многоугольник
{
    float angle;    //Угол относительно начального расположения многоугольника
public:
    Polygon(double* x_s, double* y_s, int num);  //НЕ СДЕЛАНО (и деструктор)
    ~Polygon();
    unsigned int num;   //Количество точек в многоугольнике
    Point** orig_points; //Массив точек многоугольника
    Point** points; //Точки многоугольника с учетом поворота

    void Turn(double angle, Point &pivot);   //TODO - продумать реализацию

};

//Обработка столкновений

class PointCollider;
class LineCollider;
class CircleCollider;
class PolygonCollider;

class Collider:public Point  //Интерфейс столкновений
{
public:
    double angle;
    int collisions;

    Collider(double o_x, double o_y);
    //virtual bool CheckCollision2(Collider* other) = 0;           //Проверка пересечения с неизвестным обьектом


    virtual bool CheckCollision(Collider* other) = 0;           //Проверка пересечения с неизвестным обьектом
    virtual bool CheckCollision(PointCollider* other) = 0;              //Проверка пересечения с точкой
    virtual bool CheckCollision(LineCollider* other) = 0;               //Проверка пересечения с линией
    virtual bool CheckCollision(CircleCollider* other) = 0;     //Проверка пересечения с кругом
    virtual bool CheckCollision(PolygonCollider* other) = 0;            //Проверка пересечения с многоугольником
    virtual void ShowCollider() = 0;           //Отрисовка коллайдера
\
    virtual void Turn(double angle) = 0;    //Поворот относительно центра (o_x; o_y) в конструкторе
    virtual void SetAngle(double angle);

    //virtual void UpdateBoundingBox(float x, float y) = 0;   //Обновить координаты
    //Polygon* BoundingBox;   //Прямоугольник для первичной проверки.
};

class PointCollider : public Collider
{
public:
    Point* point;

    PointCollider(float _x, float _y);
    ~PointCollider();

    bool CheckCollision(Collider* other);           //Проверка пересечения с неизвестным обьектом
    bool CheckCollision(PointCollider* other);              //Проверка пересечения с точкой
    bool CheckCollision(LineCollider* other);               //Проверка пересечения с линией
    bool CheckCollision(CircleCollider* other);     //Проверка пересечения с кругом
    bool CheckCollision(PolygonCollider* other);            //Проверка пересечения с многоугольником
    void ShowCollider();                            //Отрисовка коллайдера

    void MoveTo(float _x, float _y);  //Передвинуть в указанную позицию
    void Drag(float dx, float dy);  //Передвинуть на dx пикселей по горизонтали и на dy по вертикали
    void Turn(double angle, Point& pivot);    //Повернуть точку относительно точки pivot
    void Turn(double angle);    //Поворот относительно центра - точка просто остается на месте

};

class LineCollider : public Collider
{
public:
    Line* line;

    LineCollider(float x1, float y1, float x2, float y2);
    ~LineCollider();

    bool CheckCollision(Collider* other);           //Проверка пересечения с неизвестным обьектом
    bool CheckCollision(PointCollider* other);      //Проверка пересечения с точкой
    bool CheckCollision(LineCollider* other);       //Проверка пересечения с линией
    bool CheckCollision(CircleCollider* other);     //Проверка пересечения с кругом
    bool CheckCollision(PolygonCollider* other);            //Проверка пересечения с многоугольником
    void ShowCollider();                            //Отрисовка коллайдера

    void MoveTo(float _x, float _y);  //Передвинуть в указанную позицию - при этом передвигается первая точка линии, вторая двигается за ней
    void Drag(float dx, float dy);  //Передвинуть на dx пикселей по горизонтали и на dy по вертикали
    void Turn(double angle, Point& pivot);    //Повернуть линию относительно pivot
    void Turn(double angle);    //Поворот относительно первой точки
    void SetAngle(double angle);   //Установка угла относительно первой точки
};

class CircleCollider : public Collider
{
public:
    Circle* circle;

    CircleCollider(float _x, float _y, float _r);
    ~CircleCollider();

    bool CheckCollision(Collider* other);           //Проверка пересечения с неизвестным обьектом
    bool CheckCollision(PointCollider* other);      //Проверка пересечения с точкой
    bool CheckCollision(LineCollider* other);       //Проверка пересечения с линией
    bool CheckCollision(CircleCollider* other);     //Проверка пересечения с кругом
    bool CheckCollision(PolygonCollider* other);            //Проверка пересечения с многоугольником
    void ShowCollider();

    void MoveTo(float _x, float _y);  //Передвинуть центр в указанную позицию
    void Drag(float dx, float dy);  //Передвинуть центр на dx пикселей по горизонтали и на dy по вертикали
    void Turn(double angle, Point& pivot);    //Повернуть круг вокруг pivot
    void Turn(double angle);    //Поворот относительно центра - круг просто остается на месте
};

class PolygonCollider : public Collider
{
public:
    Point** points;
    Point** orig_points;
    int count;

    PolygonCollider(double* x_s, double* y_s, int num, double orig_x, double orig_y);
    ~PolygonCollider();

    bool CheckCollision(Collider* other);           //Проверка пересечения с неизвестным обьектом
    bool CheckCollision(PointCollider* other);      //Проверка пересечения с точкой
    bool CheckCollision(LineCollider* other);       //Проверка пересечения с линией
    bool CheckCollision(CircleCollider* other);     //Проверка пересечения с кругом
    bool CheckCollision(PolygonCollider* other);            //Проверка пересечения с многоугольником
    void ShowCollider();

    void MoveTo(float _x, float _y);  //Передвинуть центр в указанную позицию
    void Drag(float dx, float dy);  //Передвинуть центр на dx пикселей по горизонтали и на dy по вертикали
    void Turn(double angle, Point& pivot);    //Повернуть круг вокруг pivot
    void Turn(double angle);    //Поворот относительно центра - круг просто остается на месте
    void SetAngle(double angle);
};




#endif // COLLISION_H
