//
// Created by Norman on 2021/9/28.
//

#ifndef CSLH_STRQUEUE_H
#define CSLH_STRQUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct StrQueueNode {
    char *data;
    struct StrQueueNode *next;
};

struct StrQueue {
    struct StrQueueNode *rear, *front;
    int count;
};

int initQueue(struct StrQueue **Q);

int enQueue(struct StrQueue *Q, char *str_in);

int deQueue(struct StrQueue *Q, char **str_out);

int emptyQueue(struct StrQueue *Q);

int deleteQueue(struct StrQueue **Q);

#endif //CSLH_STRQUEUE_H
