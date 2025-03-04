#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (!new)
        return NULL;
    INIT_LIST_HEAD(new);
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *pos, *q;
    list_for_each_safe (pos, q, head) {
        element_t *entry = list_entry(pos, element_t, list);
        q_release_element(entry);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = (element_t *) malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *entry = list_first_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, entry->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    list_del(&entry->list);
    return entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    return q_remove_head(head->prev->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *pos;
    list_for_each (pos, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *left = head->next, *right = head->prev;
    while (left != right && left->next != right) {
        left = left->next;
        right = right->prev;
    }
    list_del(right);
    element_t *entry = list_entry(right, element_t, list);
    q_release_element(entry);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;
    struct list_head *pos, *q;
    list_for_each_safe (pos, q, head) {
        element_t const *entry = list_entry(pos, element_t, list);
        struct list_head *next = pos->next;
        while (next != head) {
            element_t *next_entry = list_entry(next, element_t, list);
            if (!strcmp(entry->value, next_entry->value)) {
                list_del(next);
                q_release_element(next_entry);
            } else {
                next = next->next;
            }
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *pos, *q;
    list_for_each_safe (pos, q, head) {
        list_move(pos, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || k <= 1)
        return;
    struct list_head *pos, *q;
    int count = 0;
    list_for_each_safe (pos, q, head) {
        count++;
        if (count == k) {
            struct list_head *next = pos->next;
            list_cut_position(head, pos, next);
            q_reverse(head);
            list_splice(next, head);
            count = 0;
        }
    }
}
static int q_merge_two(struct list_head *first,
                       struct list_head *second,
                       bool descend)
{
    if (!first || list_empty(first) || !second || list_empty(second))
        return 0;
    int size = 0;
    struct list_head tmp;
    INIT_LIST_HEAD(&tmp);
    while (!list_empty(first) && !list_empty(second)) {
        element_t *first_entry = list_first_entry(first, element_t, list);
        element_t *second_entry = list_first_entry(second, element_t, list);
        element_t *cmp_value;
        if (!descend)
            cmp_value = strcmp(first_entry->value, second_entry->value) < 0
                            ? first_entry
                            : second_entry;
        else
            cmp_value = strcmp(first_entry->value, second_entry->value) > 0
                            ? first_entry
                            : second_entry;
        list_move_tail(&cmp_value->list, &tmp);
        size++;
    }
    size += q_size(first);
    list_splice_tail_init(first, &tmp);
    size += q_size(second);
    list_splice_tail_init(second, &tmp);
    list_splice(&tmp, first);
    return size;
};
/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *left, *right, *mid;
    left = right = head;
    do {
        left = left->next;
        right = right->prev;
    } while (left != right && left->next != right);
    mid = left;
    LIST_HEAD(second);
    list_cut_position(&second, mid, head->prev);
    q_sort(head, descend);
    q_sort(&second, descend);
    q_merge_two(head, &second, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;
    struct list_head *pos, *q;
    list_for_each_safe (pos, q, head) {
        element_t *entry = list_entry(pos, element_t, list);
        struct list_head *next_node = pos->next;
        while (next_node != head) {
            element_t const *next_entry =
                list_entry(next_node, element_t, list);
            if (strcmp(entry->value, next_entry->value) > 0) {
                list_del(&entry->list);
                q_release_element(entry);
                break;
            }
            next_node = next_node->next;
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;
    struct list_head *pos, *q;
    list_for_each_safe (pos, q, head) {
        element_t *entry = list_entry(pos, element_t, list);
        struct list_head *next_node = pos->next;
        while (next_node != head) {
            element_t const *next_entry =
                list_entry(next_node, element_t, list);
            if (strcmp(entry->value, next_entry->value) < 0) {
                list_del(&entry->list);
                q_release_element(entry);
                break;
            }
            next_node = next_node->next;
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return q_size(list_first_entry(head, queue_contex_t, chain)->q);
    int queue_size = 0;
    queue_contex_t *first, *second;
    first = list_first_entry(head, queue_contex_t, chain);
    second = list_first_entry(head->next, queue_contex_t, chain);
    queue_contex_t *end = NULL;
    while (second != end) {
        queue_size = q_merge_two(first->q, second->q, descend);
        if (!end)
            end = second;
        list_move_tail(&second->chain, head);
        second = list_entry(first->chain.next, queue_contex_t, chain);
    }
    return queue_size;
}
