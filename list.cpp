#include "list.h"

EntityStackItem::EntityStackItem(Entity* _entity, EntityStackItem* _next)
{
    entity = _entity;
    Next = _next;
}

EntityStackItem::~EntityStackItem() //Clear entity
{
    delete entity;
}


EntityStack::EntityStack()   //Empty stack
{
    size = 0;
    first = NULL;
    current = NULL;
}

void EntityStack::Add(Entity* _entity) //Add new entity (current is not changed)
{
    EntityStackItem* new_entity = new EntityStackItem(_entity, first);
    first = new_entity;
    size++;
}

void EntityStack::Delete()  //Deleting current entity (next is chosen as current)
{
    EntityStackItem* deleted = current;
    current = current->Next;
    delete deleted;
    size--;
}

bool EntityStack::Delete(Entity* _entity) //Delete entity with _entity value. If deleted current entity, that next is chosen as current
{
    EntityStackItem* curr = first;
    EntityStackItem* prev = NULL;
    for(unsigned int i = 0; i < size && curr != NULL; i++)
    {
        if (curr->entity == _entity)
        {
            EntityStackItem* deleted = curr;
            if (deleted == first)
            {
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
            return true;
        }
        prev = curr;
        curr = curr->Next;
    }
    return false;
}

void EntityStack::Next()    //Chosing next entity as current
{
    current = current->Next;
}

void EntityStack::Reset()   //Going back to start of stack (current entity is first entity)
{
    current = first;
}

void EntityStack::Clear()   //Clearing stack
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
