//
// Created by Norman on 2021/9/28.
//

#include "../include/StrQueue.h"

int initQueue(struct StrQueue **Q) {
    *Q = (struct StrQueue *) malloc(sizeof(struct StrQueue));
    (*Q)->front = (struct StrQueueNode *) malloc(sizeof(struct StrQueueNode));
    if ((*Q)->front == NULL) {
        printf("allocate memory failed(initialize queue), exiting\n");
        exit(1);
    }
    (*Q)->count = 0;
    (*Q)->rear = (*Q)->front;
    (*Q)->front->next = NULL;
    return 0;
}

int enQueue(struct StrQueue *Q, char *str_in) {
    struct StrQueueNode *new_node = (struct StrQueueNode *) malloc(sizeof(struct StrQueueNode));
    if (new_node == NULL) {
        printf("allocate memory failed(en-queue node), exiting\n");
        exit(1);
    }
    // printf("str_in:%lu\t", strlen(str_in));
    char *new_str = (char *) malloc(sizeof(char) * (strlen(str_in) + 1));  // plus one more space for '\0'
    if (new_str == NULL) {
        printf("allocate memory failed(store string in memory), exiting\n");
        exit(1);
    }
    strncpy(new_str, str_in, strlen(str_in));
    new_str[strlen(str_in)] = '\0';
    // printf("new_str:%lu\n", strlen(new_str));
    if (strlen(new_str) != strlen(str_in)) {
        printf("\nerror queuing:\nlength in queue=%lu, length in file=%lu\n", strlen(new_str), strlen(str_in));
        exit(1);
    }

    new_node->data = new_str;
    new_node->next = NULL;
    Q->rear->next = new_node;
    Q->rear = new_node;
    Q->count += 1;
    return 0;
}

int deQueue(struct StrQueue *Q, char **str_out) {
    if (Q->front == Q->rear) exit(1);
    struct StrQueueNode *ptr;
    ptr = Q->front->next;
    *str_out = ptr->data;
    Q->front->next = ptr->next;
    if (Q->rear == ptr) Q->rear = Q->front;
    free(ptr);
    Q->count -=1 ;
    return 0;
}

int emptyQueue(struct StrQueue *Q) {
    struct StrQueueNode *del_node;
    if (Q->front != Q->rear) {
        while (Q->front->next != Q->rear) {
            del_node = Q->front->next;
            Q->front->next = Q->front->next->next;
            free(del_node->data);
            free(del_node);
        }
        free(Q->front->next);
        Q->rear = Q->front;
    }
    return 0; 
}

int deleteQueue(struct StrQueue **Q) {
    emptyQueue(*Q);
    free(*Q);
    Q = NULL;
    return 0;
}