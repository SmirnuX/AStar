#ifndef LIST_H
#define LIST_H

//Stack to store entities in

#include "game.h"



class Entity;

class EntityStackItem    //Struct for storing one entity
{
public:
    Entity* entity;
    EntityStackItem* Next;

    EntityStackItem(Entity* _entity, EntityStackItem* _next = NULL);
    ~EntityStackItem();  //Clear entity
};

class EntityStack    //stack of entities
{
private:
    EntityStackItem* first;   //First object in stack
public:
    unsigned int size;  //Count of entities
    EntityStackItem* current; //Current entity
    EntityStack();   //Empty stack
    void Add(Entity* _entity); //Add new entity (current is not changed)
    void Delete();  //Deleting current entity (next is chosen as current)
    bool Delete(Entity* _entity);   //Delete entity with _entity value. If deleted current entity, that next is chosen as current
    void Next();    //Chosing next entity as current
    void Reset();   //Going back to start of stack (current entity is first entity)
    void Clear();   //Clearing stack
    ~EntityStack();
};

#endif // LIST_H
