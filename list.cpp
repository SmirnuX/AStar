#include "list.h"

EntityStackItem::EntityStackItem(Entity* _entity, EntityStackItem* _next)   //Конструктор элемента стека
{
    entity = _entity;
    Next = _next;
}

EntityStackItem::~EntityStackItem() //Очищает память, выделенную под entity
{
    delete entity;
}


EntityStack::EntityStack()   //Конструктор пустого стека
{
    size = 0;
    first = NULL;
    current = NULL;
}

void EntityStack::Add(Entity* _entity) //Добавление нового объекта в стек (текущий объект не изменяется)
{
    EntityStackItem* new_entity = new EntityStackItem(_entity, first);
    first = new_entity;
    size++;
}

void EntityStack::Delete()  //Удаление текущего объекта. Текущим объектом становится следующий объект стека
{
    EntityStackItem* deleted = current;
    current = current->Next;
    //Поменять указатель предыдущего
    delete deleted;
    size--;
}

void EntityStack::Delete(Entity* _entity) //Удаление сущности с указателем _entity
{
    //qDebug()<<"Looking for" << _entity;
    EntityStackItem* curr = first;
    EntityStackItem* prev = NULL;
    for(unsigned int i = 0; i < size && curr != NULL; i++)
    {
        if (curr->entity == _entity)    //Перепись предыдущего
        {
            //qDebug() << "Deleting entity " << curr->entity;
            EntityStackItem* deleted = curr;
            if (deleted == first)
            {
             //   qDebug() << "First deleted";
                first = first->Next;
            }
            else
            {
                prev->Next = deleted->Next;
            }
            if (deleted == current)
            {
                current = current->Next;
            }
            delete deleted;
            size--;
            return;
        }
        prev = curr;
        curr = curr->Next;
    }
    //qDebug() << "Not found";
}

void EntityStack::Next()    //Переход к следующему объекту
{
    current = current->Next;
}

void EntityStack::Reset()   //Возвращение в начало стека
{
    current = first;
}

void EntityStack::Clear()   //Очистка стека
{
    for (EntityStackItem* curr = first; curr != NULL; )
    {
        EntityStackItem* deleted = curr;
        curr = curr->Next;
        delete deleted;
    }
}

EntityStack::~EntityStack()
{
    Clear();
}
