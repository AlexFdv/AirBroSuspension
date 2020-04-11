/*
 * List.h
 *
 *  Created on: Apr 11, 2020
 *      Author: Alex
 */

#ifndef INCLUDE_UTILS_LIST_H_
#define INCLUDE_UTILS_LIST_H_

#include "FreeRTOS.h"

typedef struct ListItem {
    struct ListItem *next;

    void *container;
    void *value;
} ListItem;

typedef struct {
    portBASE_TYPE count;
    ListItem *head;
} List;

List *createList(void);
ListItem *insertListItem(List *list, void *value);
void deleteList(List *list);
void removeListItem(List *list, ListItem *item);
void removeListItemByValue(List *list, void *value);

#define listGET_LIST_COUNT( list ) ( ( list )->count)
#define listGET_LIST_HEAD( list ) ( ( list )->head)


#endif /* INCLUDE_UTILS_LIST_H_ */
