/* adlist.h - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __LIST_H__
#define __LIST_H__

/* Node, List, and Iterator are the only data structures used currently. */

typedef struct list_node {
    struct list_node *prev;
    struct list_node *next;
    void *value;
} list_node_t;

typedef struct list_iter {
    list_node_t *next;
    int direction;
} list_iter_t;

typedef struct list {
    list_node_t *head;
    list_node_t *tail;
    void *(*dup)(void *ptr);
    void (*free)(void *ptr);
    int (*match)(void *ptr, void *key);
    unsigned long len;
} list_t;

/* Functions implemented as macros */
#define list_size(l) ((l)->len)
#define list_first(l) ((l)->head)
#define list_last(l) ((l)->tail)
#define list_prev(n) ((n)->prev)
//#define list_next(n) ((n)->next)
#define list_value(n) ((n)->value)

#define list_set_clone_method(l,m) ((l)->dup = (m))
#define list_set_free_method(l,m) ((l)->free = (m))
#define list_set_match_method(l,m) ((l)->match = (m))

#define list_get_clone_method(l) ((l)->dup)
#define list_get_free_method(l) ((l)->free)
#define list_get_match_method(l) ((l)->match)

/* Prototypes */
/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
list_t *list_create(void);

/* Free the whole list.
 * This function can't fail. */
void list_free(list_t *list);

/* Add a new node to the list, to head, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
list_t *list_add_head(list_t *list, void *value);


/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
list_t *list_add(list_t *list, void *value);


list_t *list_insert(list_t *list, list_node_t *old_node, void *value, int after);

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
void list_remove(list_t *list, list_node_t *node);


/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
list_iter_t *list_iterator(list_t *list, int direction);

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(list_node_tValue(node));
 * }
 *
 * */
list_node_t *list_next(list_iter_t *iter);

/* Release the iterator memory */
void list_free_iterator(list_iter_t *iter);

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
list_t *list_clone(list_t *orig);

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
list_node_t *list_search(list_t *list, void *key);


/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
list_node_t *list_index(list_t *list, long index);


/* Create an iterator in the list private iterator structure */
void list_rewind(list_t *list, list_iter_t *li);
void list_rewind_tail(list_t *list, list_iter_t *li);

/* Rotate the list removing the tail node and inserting it to the head. */
void list_rotate(list_t *list);

/* Directions for iterators */
#define _START_HEAD 0
#define _START_TAIL 1

#endif /* __ADLIST_H__ */
