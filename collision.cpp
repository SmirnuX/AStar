#include "game.h"
#include <limits>

Collider::Collider(double o_x, double o_y) : Point (o_x, o_y)
{
    angle = 0;
    collisions = 0;
}

void Collider::SetAngle(double angle)
{

}

Line::Line(Point p1, Point p2)   //Задание линии от точки p1 до p2.
{
    min_p = new Point(p1.GetX(), p1.GetY());
    max_p = new Point(p2.GetX(), p2.GetY());
    Update();
}

Line::~Line()
{
    delete min_p;
    delete max_p;
}

void Line::Update()  //Обновить k, b и проверить min_p и max_p
{
    if (max_p->GetX() < min_p->GetX())
    {
        Point* temp = min_p;
        min_p = max_p;
        max_p = temp;
    }
    if (max_p->GetX() == min_p->GetX())
        k = 10000;  //КОСТЫЛЬ
    else
        k = (max_p->GetY() - min_p->GetY())/(max_p->GetX()-min_p->GetX());
    b = min_p->GetY() - k*min_p->GetX();
}

void Line::Turn(double angle, Point &pivot) //Поворот линии относительно точки pivot
{
    min_p->Turn(angle, pivot);
    max_p->Turn(angle, pivot);
    Update();
}

Circle::Circle(float _x, float _y, float _r)   //Создание окружности
{
    center = new Point(_x, _y);
    r = _r;
}

Circle::~Circle()
{
    delete center;
}


//====== Реализация PointCollider ======

PointCollider::PointCollider(float _x, float _y) : Collider(_x, _y)
{
    point = new Point(_x, _y);
}

PointCollider::~PointCollider()
{
    delete point;
}

bool PointCollider::CheckCollision(Collider* other)        //Проверка пересечения с неизвестным обьектом
{
    return other->CheckCollision(this);
}

bool PointCollider::CheckCollision(PointCollider* other)              //Проверка пересечения с точкой
{
    if (&(other->point) == &point)
        return true;
    else
        return false;
}

bool PointCollider::CheckCollision(LineCollider* other)               //Проверка пересечения с линией
{
    float im_y = point->GetX() * other->line->k + other->line->b;
    if (almostEq(im_y, point->GetY()))
        return true;
    else
        return false;
}

bool PointCollider::CheckCollision(CircleCollider* other)     //Проверка пересечения с кругом
{
    double dx = point->GetX() - other->circle->center->GetX();
    double dy = point->GetY() - other->circle->center->GetY();
    double distance = dx * dx + dy * dy;                        //Расстояние от точки до центра
    return (distance < (other->circle->r * other->circle->r));
}

bool PointCollider::CheckCollision(PolygonCollider* other)            //Проверка пересечения с многоугольником
{
    //Пускаем луч из точки параллельно оси x. Если по пути четное пересечение границ - значит, снаружи. Иначе - внутри.
    double x_ray = point->GetX();   //Начало луча (минимальная точка)
    double y_ray = point->GetY();   //Высота луча

    Point* curr;    //Первая точка рассматриваемого отрезка
    Point* next;    //Вторая точка рассматриваемого отрезка
    bool inside = false;    //Изначально считаем, что точка снаружи

    for(int i=0; i < other->count; i++)
    {
        curr = other->points[i];
        if (i < other->count - 1)
            next = other->points[i+1];
        else
            next = other->points[0];

        if (almostEq(y_ray, curr->GetY()))  //первая точка на луче (или позади него)
        {
            if (almostEq(y_ray, next->GetY()))  //вторая точка тоже на луче - отрезок параллелен лучу
            {
                if (almostEq(x_ray, curr->GetX()))  //первая точка совпадает с началома луча
                    return true;
                if ( (x_ray < curr->GetX()) != (x_ray < next->GetX()) ) //Если точки отрезка находятся по разные стороны от точки
                    return true;
                //ИНАЧЕ - не учитываем эту грань как пересечение. НО - пересечение должно быть рассмотрено в следующей точке
            }
            else //вторая точка над или под лучом
            {
                if (almostEq(x_ray, curr->GetX()))  //первая точка совпадает с точкой
                    return true;
                if (x_ray < curr->GetX())   //первая точка находится на луче
                    inside = !inside;
                //ИНАЧЕ - первая точка позади луча, пересечений нет
            }
        }
        else if (y_ray < curr->GetY())  //Луч находится над первой точкой
        {
            if (almostEq(y_ray, next->GetY()))  //Вторая точка находится на/за лучем
            {
                if (almostEq(x_ray, next->GetX()))   //Если вторая точка совпадает с точкой
                    return true;
                if (x_ray < next->GetX()) //Если вторая точка лежит на луче
                    inside = !inside;
                //ИНАЧЕ - Если вторая точка за лучом - пересечений нет
            }
            else if (y_ray > next->GetY())  //Отрезок пересекает луч снизу вверх. Определяем, слева ли точка
            {
                double inverse_k = (next->GetX() - curr->GetX()) / (next->GetY() - curr->GetY());   //Представим отрезок в качестве уравнения x = ky + b
                double inverse_b = next->GetX() - inverse_k * next->GetY();
                double intersec_x = inverse_k * y_ray + inverse_b;  //Точка пересечения луча и отрезка
                if (x_ray < intersec_x)
                    inside = !inside;
            }
            //ИНАЧЕ - Отрезок находится под лучом, пересечений нет
        }
        else    //Луч находится под первой точкой. Полностью повторяет верхний блок. TODO - проверить, не избыточен ли код
        {
            if (almostEq(y_ray, next->GetY()))  //Вторая точка находится на/за лучем
            {
                if (almostEq(x_ray, next->GetX()))   //Если вторая точка совпадает с точкой
                    return true;
                if (x_ray < next->GetX()) //Если вторая точка лежит на луче
                    inside = !inside;
                //ИНАЧЕ - Если вторая точка за лучом - пересечений нет
            }
            else if (y_ray < next->GetY())  //Отрезок пересекает луч снизу вверх. Определяем, слева ли точка
            {
                double inverse_k = (next->GetX() - curr->GetX()) / (next->GetY() - curr->GetY());   //Представим отрезок в качестве уравнения x = ky + b
                double inverse_b = next->GetX() - inverse_k * next->GetY();
                double intersec_x = inverse_k * y_ray + inverse_b;  //Точка пересечения луча и отрезка
                if (x_ray < intersec_x)
                    inside = !inside;
            }
            //ИНАЧЕ - Отрезок находится под лучом, пересечений нет
        }

    }
    return inside;
}

void PointCollider::ShowCollider()  //Отрисовка коллайдера
{
    QPainter painter(picture);
    if (collisions > 0)
        painter.setPen(QColor(0,255,0));
    else
        painter.setPen(QColor(255,0,0));
    painter.drawPoint(point->GetX(), point->GetY());
}

void PointCollider::MoveTo(float _x, float _y)  //Передвинуть в указанную позицию
{
    point->MoveTo(_x, _y);
}

void PointCollider::Drag(float dx, float dy)  //Передвинуть на dx пикселей по горизонтали и на dy по вертикали
{
    point->Drag(dx, dy);
}

void PointCollider::Turn(double angle, Point& pivot)    //Повернуть точку относительно точки pivot
{
    point->Turn(angle, pivot);
}

void PointCollider::Turn(double angle)    //Поворот относительно центра - точка просто остается на месте
{

}

//====== Реализация LineCollider ======
LineCollider::LineCollider(float x1, float y1, float x2, float y2):Collider(x1 ,y1)
{
    line = new Line(Point(x1, y1), Point(x2, y2));
}

LineCollider::~LineCollider()
{
    delete line;
}

bool LineCollider::CheckCollision(Collider* other)           //Проверка пересечения с неизвестным обьектом
{
    return other->CheckCollision(this);
}

bool LineCollider::CheckCollision(PointCollider* other)      //Проверка пересечения с точкой
{
    return other->CheckCollision(this);
}

bool LineCollider::CheckCollision(LineCollider* other)       //Проверка пересечения с линией
{
    if (almostEq(other->line->max_p->GetX(), other->line->min_p->GetX()))   //КОСТЫЛЬ для вертикальных линий
    {
        if (almostEq(line->min_p->GetX(), line->max_p->GetX())) //Две вертикальные линии
            return almostEq(line->min_p->GetX(), other->line->min_p->GetX());
        double quer_y = line->k * other->line->max_p->GetX() + line->b;
        if (line->min_p->GetX() < other->line->min_p->GetX() && other->line->min_p->GetX() < line->max_p->GetX())
            return ( (other->line->min_p->GetY() < quer_y && quer_y < other->line->max_p->GetY()) || (other->line->max_p->GetY() < quer_y && quer_y < other->line->min_p->GetY()) );
        else
            return false;
    }
    if (almostEq(line->max_p->GetX(), line->min_p->GetX())) //КОСТЫЛЬ для первой линии
        return other-> CheckCollision(this);

    if (other->line->max_p->GetX() < line->min_p->GetX() || other->line->min_p->GetX() > line->max_p->GetX())
        return false;   //Отрезки не пересекаются в принципе

    if (almostEq(other->line->k, line->k))  //Линии параллельны
    {
        if (almostEq(other->line->b, line->b))  //Прямые совпадают
            return true;
        else
            return false;   //отрезки параллельны
    }
    double quer_x = (line->b - other->line->b) / (other->line->k - line->k);    //Точка пересечения прямых
    //qDebug()<<"QUER X" << quer_x;
    if (line->min_p->GetX() > quer_x || line->max_p->GetX() < quer_x)
        return false;
    if (other->line->min_p->GetX() > quer_x || other->line->max_p->GetX() < quer_x)
        return false;
    //double min_x = (line->min_p->GetX() < other->line->min_p->GetX())?line->min_p->GetX():other->line->min_p->GetX();
    //double max_x = (line->max_p->GetX() > other->line->max_p->GetX())?line->max_p->GetX():other->line->max_p->GetX();

    //if (min_x < quer_x && quer_x < max_x)
        //return true;
    //else
        return false;

}

bool LineCollider::CheckCollision(CircleCollider* other)     //Проверка пересечения с кругом
{
    //Находим ближайшую точку прямой к центру круга
    double normal_k;
    double near_x, near_y;  //Координаты ближайшей точки
    if (!almostEq(line->k, 0))
        normal_k = -1/line->k;   //Коэффициент в уравнении прямой для нормали
    else
        normal_k = 10000;  //ПОДГОООН
    double normal_b = other->circle->center->GetY() - normal_k * other->circle->center->GetX();

    //Находим пересечение нормали из центра круга и линии
    double quer_x = (line->b - normal_b) / (normal_k - line->k);    //Точка пересечения прямых
    if (almostEq(line->min_p->GetX(), line->max_p->GetX())) //КОСТЫЛЬ для вертикальных линий
    {
        near_x = line->min_p->GetX();
        near_y = other->circle->center->GetY();
        if ( (line->min_p->GetY() > near_y || line->max_p->GetY() < near_y) && (line->max_p->GetY() > near_y || line->min_p->GetY() < near_y) )
            return false;
    }
    else if (quer_x >= line->min_p->GetX() && quer_x <= line->max_p->GetX()) //Если нормаль пересекает прямую
    {
        near_x = quer_x;
        near_y = normal_k * quer_x + normal_b;
    }
    else if (quer_x > line->max_p->GetX())
    {
        near_x = line->max_p->GetX();
        near_y = line->max_p->GetY();
    }
    else
    {
        near_x = line->min_p->GetX();
        near_y = line->min_p->GetY();
    }
    double distance = (near_y - other->circle->center->GetY()) * (near_y - other->circle->center->GetY()) + (near_x - other->circle->center->GetX()) * (near_x - other->circle->center->GetX());  //Расстояние от центра круга до ближайшей точки линии
    return (distance < other->circle->r * other->circle->r);
}

bool LineCollider::CheckCollision(PolygonCollider* other)            //Проверка пересечения с многоугольником
{
    Point* curr;    //Первая точка рассматриваемого отрезка
    Point* next;    //Вторая точка рассматриваемого отрезка
    LineCollider line_c(0,0,1,1);

    for(int i=0; i < other->count; i++)   //Проверка пересечения с линиями
    {
        curr = other->points[i];
        if (i < other->count-1)
            next = other->points[i+1];
        else
            next = other->points[0];
        line_c.line->min_p->MoveTo(curr->GetX(), curr->GetY());
        line_c.line->max_p->MoveTo(next->GetX(), next->GetY());
        line_c.line->Update();
        if (line_c.CheckCollision(this))
            return true;
    }
    PointCollider point_c(line->min_p->GetX(), line->min_p->GetY());
    return point_c.CheckCollision(other);    //Проверяем, входит ли отрезок в многоугольник полностью
}

void LineCollider::ShowCollider()                            //Отрисовка коллайдера
{
    QPainter painter(picture);
    if (collisions > 0)
        painter.setPen(QColor(0,255,0));
    else
        painter.setPen(QColor(255,0,0));
    painter.drawLine(line->min_p->GetX(), line->min_p->GetY(), line->max_p->GetX(), line->max_p->GetY());
    collisions = 0;
}

void LineCollider::MoveTo(float _x, float _y)  //Передвинуть в указанную позицию - при этом передвигается первая точка линии, вторая двигается за ней
{
    double dx = line->max_p->GetX() - line->min_p->GetX();
    double dy = line->max_p->GetY() - line->min_p->GetY();
    line->min_p->MoveTo(_x, _y);
    line->max_p->MoveTo(_x + dx, _y + dy);
}

void LineCollider::Drag(float dx, float dy)  //Передвинуть на dx пикселей по горизонтали и на dy по вертикали
{
    line->min_p->Drag(dx, dy);
    line->max_p->Drag(dx, dy);
}

void LineCollider::Turn(double angle, Point& pivot)    //Повернуть линию относительно pivot
{
    line->Turn(angle, pivot);
}

void LineCollider::Turn(double angle)    //Поворот относительно первой точки
{
    line->Turn(angle, *line->min_p);
}

void LineCollider::SetAngle(double angle)   //Установка угла относительно первой точки
{
    double length = sqrt((line->max_p->GetY() - line->min_p->GetY()) * (line->max_p->GetY() - line->min_p->GetY()) + (line->max_p->GetX() - line->min_p->GetX()) * (line->max_p->GetX() - line->min_p->GetX()));
    line->max_p->MoveTo(line->min_p->GetX() + cos(angle) * length, line->min_p->GetY() - sin(angle) * length);
    line->Update();
}


CircleCollider::CircleCollider(float _x, float _y, float _r) : Collider(_x, _y)
{
    circle = new Circle(_x ,_y, _r);
}

CircleCollider::~CircleCollider()
{
    delete circle;
}

bool CircleCollider::CheckCollision(Collider* other)           //Проверка пересечения с неизвестным обьектом
{
    return other->CheckCollision(this);
}

bool CircleCollider::CheckCollision(PointCollider* other)      //Проверка пересечения с точкой
{
    return other->CheckCollision(this);
}

bool CircleCollider::CheckCollision(LineCollider* other)       //Проверка пересечения с линией
{
    return other->CheckCollision(this);
}

bool CircleCollider::CheckCollision(CircleCollider* other)    //Проверка пересечения с кругом
{
    double dx = (circle->center->GetX() - other->circle->center->GetX());
    double dy = (circle->center->GetY() - other->circle->center->GetY());
    double distance = dx*dx + dy*dy;
    return (distance < (circle->r + other->circle->r)*(circle->r + other->circle->r));
}

bool CircleCollider::CheckCollision(PolygonCollider* other)            //Проверка пересечения с многоугольником
{
    //Входит ли центр круга в многоугольник
    PointCollider point_c(circle->center->GetX(), circle->center->GetY());
    if(point_c.CheckCollision(other))    //Проверяем, входит ли отрезок в многоугольник полностью
        return true;

    //Проверяем, пересекаются ли стороны многоугольника с кругом
    LineCollider line_c(0,0,1,1);
    Point* curr;    //Первая точка рассматриваемого отрезка
    Point* next;    //Вторая точка рассматриваемого отрезка

    for(int i=0; i < other->count; i++)
    {
        curr = other->points[i];
        if (i < other->count - 1)
            next = other->points[i+1];
        else
            next = other->points[0];
        line_c.line->min_p->MoveTo(curr->GetX(), curr->GetY());
        line_c.line->max_p->MoveTo(next->GetX(), next->GetY());
        line_c.line->Update();
        if (line_c.CheckCollision(this))
            return true;
    }
    return false;
}

void CircleCollider::ShowCollider()
{
    QPainter painter(picture);
    if (collisions > 0)
        painter.setPen(QColor(0,255,0));
    else
        painter.setPen(QColor(255,0,0));
    painter.drawEllipse(circle->center->GetX() - circle->r, circle->center->GetY() - circle->r, 2 * circle->r, 2 * circle->r);
    collisions = 0;
}

void CircleCollider::MoveTo(float _x, float _y) //Передвинуть центр в указанную позицию
{
    circle->center->MoveTo(_x,_y);
}

void CircleCollider::Drag(float dx, float dy)  //Передвинуть центр на dx пикселей по горизонтали и на dy по вертикали
{
    circle->center->Drag(dx, dy);
}

void CircleCollider::Turn(double angle, Point& pivot)   //Повернуть круг вокруг pivot
{
    circle->center->Turn(angle, pivot);
}

void CircleCollider::Turn(double angle)    //Поворот относительно центра - круг просто остается на месте
{

}

PolygonCollider::PolygonCollider(double* x_s, double* y_s, int num, double orig_x = 0, double orig_y = 0) : Collider(orig_x, orig_y)
{
    points = new Point*[num];
    orig_points = new Point*[num];
    for(int i=0; i<num; i++)
    {
        points[i] = new Point(x_s[i], y_s[i]);
        orig_points[i] = new Point(x_s[i] - x, y_s[i] - y);
    }
    count = num;
}

PolygonCollider::~PolygonCollider()
{
    for(int i=0; i<count; i++)
        delete(points[i]);
    delete[](points);
}

bool PolygonCollider::CheckCollision(Collider* other)           //Проверка пересечения с неизвестным обьектом
{
    return other->CheckCollision(this);
}

bool PolygonCollider::CheckCollision(PointCollider* other)      //Проверка пересечения с точкой
{
    return other->CheckCollision(this);
}

bool PolygonCollider::CheckCollision(LineCollider* other)       //Проверка пересечения с линией
{
    return other->CheckCollision(this);
}

bool PolygonCollider::CheckCollision(CircleCollider* other)     //Проверка пересечения с кругом
{
    return other->CheckCollision(this);
}

bool PolygonCollider::CheckCollision(PolygonCollider* other)            //Проверка пересечения с многоугольником
{
    //TODO - оптимизировать
    //Первичная проверка всех верщин
    PointCollider point_c(0,0);
    for(int i=0; i < count; i++)
    {
        point_c.point->MoveTo(points[i]->GetX(), points[i]->GetY());
        if (point_c.CheckCollision(other))
        {
            qDebug() << "Inside of rectangle";
            return true;
        }
    }
    //Проверка всех сторон со всеми
    Point* curr, *next;
    LineCollider line_c(0,0,1,1);
    for(int i=0; i < count; i++)
    {
        curr = points[i];
        if (i < count-1)
            next = points[i+1];
        else
            next = points[0];
        line_c.line->min_p->MoveTo(curr->GetX(), curr->GetY());
        line_c.line->max_p->MoveTo(next->GetX(), next->GetY());
        line_c.line->Update();
        if (line_c.CheckCollision(other))
            return true;
    }
    return false;
}

void PolygonCollider::ShowCollider()
{
    QPainter painter(picture);
    Point* curr, *next;
    if (collisions > 0)
        painter.setPen(QColor(0,255,0));
    else
        painter.setPen(QColor(255,0,0));
    for(int i=0; i < count; i++)
    {
        curr = points[i];
        if (i < count-1)
            next = points[i+1];
        else
            next = points[0];
        painter.drawLine(curr->GetX(), curr->GetY(), next->GetX(), next->GetY());
    }
    collisions = 0;
}

void PolygonCollider::MoveTo(float _x, float _y)  //Передвинуть центр в указанную позицию
{
    Point::MoveTo(_x, _y);
    for(int i=0; i < count; i++)
    {
        points[i]->Drag(_x - points[i]->GetX(), _y - points[i]->GetY());
    }
}

void PolygonCollider::Drag(float dx, float dy)  //Передвинуть центр на dx пикселей по горизонтали и на dy по вертикали
{
    Point::Drag(dx, dy);
    for(int i=0; i < count; i++)
    {
        points[i]->Drag(dx, dy);
    }
}

void PolygonCollider::Turn(double angle, Point& pivot)    //Повернуть круг вокруг pivot
{
    for(int i=0; i < count; i++)
    {
        points[i]->Turn(angle, pivot);
    }
}

void PolygonCollider::Turn(double angle)    //Поворот относительно центра - круг просто остается на месте
{
    Point a(x, y);
    for(int i=0; i < count; i++)
    {
        points[i]->Turn(angle, a);
    }
}

void PolygonCollider::SetAngle(double angle)
{
    Point a(x, y);
    for(int i=0; i < count; i++)
    {
        points[i]->MoveTo(orig_points[i]->GetX() + x, orig_points[i]->GetY() + y);
        points[i]->Turn(degtorad(angle), a);
    }
}





