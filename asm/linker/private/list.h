#ifndef LIST_H
#define LIST_H

typedef struct list_t list_t;
typedef struct list_node_t list_node_t;

list_t *new_list();
void list_add(list_t *list, void *data);
void list_remove(list_t *list, list_node_t *node);
list_node_t *list_head(list_t *list);
list_node_t *list_next(list_node_t *node);
void *list_data(list_node_t *node);
void free_list(list_t *list);
void free_list_ptr(list_t **list);

#endif // LIST_H
