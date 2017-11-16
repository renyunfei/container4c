/* adlist.c - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdlib.h>
#include "list.h"
#include "zmalloc.h"

list_t *list_create(void)
{
    list_t *list;

    if ((list = zmalloc(sizeof(*list))) == NULL)
        return NULL;
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;
    return list;
}

void list_free(list_t *list)
{
    unsigned long len;
    list_node_t *current, *next;

    current = list->head;
    len = list->len;
    while(len--) {
        next = current->next;
        if (list->free) list->free(current->value);
        zfree(current);
        current = next;
    }
    zfree(list);
}

list_t *list_add_head(list_t *list, void *value)
{
    list_node_t *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->len++;
    return list;
}

list_t *list_add(list_t *list, void *value)
{
    list_node_t *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    return list;
}

list_t *list_insert(list_t *list, list_node_t *old_node, void *value, int after) 
{
    list_node_t *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (after) {
        node->prev = old_node;
        node->next = old_node->next;
        if (list->tail == old_node) {
            list->tail = node;
        }
    } else {
        node->next = old_node;
        node->prev = old_node->prev;
        if (list->head == old_node) {
            list->head = node;
        }
    }
    if (node->prev != NULL) {
        node->prev->next = node;
    }
    if (node->next != NULL) {
        node->next->prev = node;
    }
    list->len++;
    return list;
}

void list_remove(list_t *list, list_node_t *node)
{
    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;
    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    if (list->free) list->free(node->value);
    zfree(node);
    list->len--;
}

list_iter_t *list_iterator(list_t *list, int direction)
{
    list_iter_t *iter;

    if ((iter = zmalloc(sizeof(*iter))) == NULL) return NULL;
    if (direction == _START_HEAD)
        iter->next = list->head;
    else
        iter->next = list->tail;
    iter->direction = direction;
    return iter;
}

void list_free_iterator(list_iter_t *iter) 
{
    zfree(iter);
}

void list_rewind(list_t *list, list_iter_t *li) 
{
    li->next = list->head;
    li->direction = _START_HEAD;
}

void list_rewind_tail(list_t *list, list_iter_t *li) 
{
    li->next = list->tail;
    li->direction = _START_TAIL;
}

list_node_t *list_next(list_iter_t *iter)
{
    list_node_t *current = iter->next;

    if (current != NULL) {
        if (iter->direction == _START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

list_t *list_clone(list_t *orig)
{
    list *copy;
    list_iter_t *iter;
    list_node_t *node;

    if ((copy = list_create()) == NULL)
        return NULL;
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    iter = list_iterator(orig, _START_HEAD);
    while((node = list_next(iter)) != NULL) {
        void *value;

        if (copy->dup) {
            value = copy->dup(node->value);
            if (value == NULL) {
                list_free(copy);
                list_free_iterator(iter);
                return NULL;
            }
        } else
            value = node->value;
        if (list_add(copy, value) == NULL) {
            list_free(copy);
            list_free_iterator(iter);
            return NULL;
        }
    }
    list_free_iterator(iter);
    return copy;
}

list_node_t *list_search(list_t *list, void *key)
{
    list_iter_t *iter;
    list_node_t *node;

    iter = list_iterator(list, _START_HEAD);
    while((node = list_next(iter)) != NULL) {
        if (list->match) {
            if (list->match(node->value, key)) {
                list_free_iterator(iter);
                return node;
            }
        } else {
            if (key == node->value) {
                list_free_iterator(iter);
                return node;
            }
        }
    }
    list_free_iterator(iter);
    return NULL;
}

list_node_t *list_index(list_t *list, long index) 
{
    list_node_t *n;

    if (index < 0) {
        index = (-index)-1;
        n = list->tail;
        while(index-- && n) n = n->prev;
    } else {
        n = list->head;
        while(index-- && n) n = n->next;
    }
    return n;
}

void list_rotate(list_t *list) 
{
    list_node_t *tail = list->tail;

    if (list_size(list) <= 1) return;

    /* Detach current tail */
    list->tail = tail->prev;
    list->tail->next = NULL;
    /* Move it as head */
    list->head->prev = tail;
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}
