/*
 * List.c
 *
 *  Created on: Apr 11, 2020
 *      Author: Alex
 */

#include "List.h"
#include "FreeRTOS.h"

List *createList(void)
{
    List *newList = (List *)pvPortMalloc(sizeof(List));

    if (!newList)
        return 0;
    newList->count = 0;
    newList->head = 0;

    return newList;
}

/* adds new 'value' to the list by copying of pointer. That means that value should be allocated in the heap.
 * list takes ownership of value
 */
ListItem *insertListItem(List *list, void *value)
{
    ListItem *newItem = (ListItem *)pvPortMalloc(sizeof(ListItem));
    newItem->value = value;
    newItem->container = list;
    newItem->next = list->head;
    list->head = newItem;
    ++(list->count);

    return newItem;
}

void deleteList(List *list)
{
    ListItem *item = list->head;

    for(; item; item = list->head)
    {
        list->head = item->next;
        vPortFree(item->value);
        vPortFree(item);
    }

    list->count = 0;
}

void removeListItem(List *list, ListItem *item)
{
    // not implemented
}

void removeListItemByValue(List *list, void *value)
{
    // not implemented
}
