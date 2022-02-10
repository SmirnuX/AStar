#ifndef LIST_H
#define LIST_H

//Стек, в котором хранятся сущности

#include "game.h"

class Entity;

class EntityStackItem    //Структура для хранения сущностей
{
public:
    Entity* entity;
    EntityStackItem* Next;

    EntityStackItem(Entity* _entity, EntityStackItem* _next = NULL);
    ~EntityStackItem();  //Очищает память, выделенную под entity
};

class EntityStack    //Стек сущностей
{
private:
    EntityStackItem* first;   //Первый объект в стеке
public:
    unsigned int size;  //Количество объектов в стеке
    EntityStackItem* current; //Текущий объект в стеке
    EntityStack();   //Конструктор пустого стека
    void Add(Entity* _entity); //Добавление нового объекта в стек (текущий объект не изменяется)
    void Delete();  //Удаление текущего объекта. Текущим объектом становится следующий объект стека
    void Delete(Entity* _entity);
    void Next();//Переход к следующему объекту
    void Reset();   //Возвращение в начало стека
    void Clear();   //Очистка стека
    ~EntityStack();
};

#endif // LIST_H
