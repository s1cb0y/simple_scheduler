#pragma once
// simple linked list implementation taken from brennan.io
struct list_head{
    struct list_head *next, *prev;
};

void list_init(struct list_head *list){
    list->next = list;
    list->prev = list;
}

/**
 * @brief Insert @a item at the beginning of list headed by @a head.
 * @param head List head which we insert into
 * @param item List node to insert
 */
void list_insert(struct list_head *head, struct list_head *item){
    item->prev = head;
    item->next = head->next;
    item->next->prev = item;
    head->next = item;
}

/**
 * @brief Insert @a item at the end of list headed by @a head.
 * @param head List head which we insert into
 * @param item List node to insert
 */
void list_insert_end(struct list_head *head, struct list_head *item){
    item->next = head;
    item->prev = head->prev;
    head->prev->next = item;
    head->prev = item;
}

/**
 * @brief Remove @a item from whatever list contains it.
 * @param item List node to remove
 *
 * Note that this function does not require a pointer to the contained list.
 * Everything it needs can be found from @a item.
 */
void list_remove(struct list_head *item){
    item->prev->next = item->next;
	item->next->prev = item->prev;
}

/**
 * @brief Iterate through struct list_head nodes of a list.
 * @param variable a variable name (of type `struct sc_list_head *`) which is
 * set to point at each element in the loop body
 * @param head pointer (of type `struct sc_list_head *`) to the head of the list
 *
 * @warning This is probably not what you want! See #sc_list_for_each_entry() for
 * simpler iteration.
 *
 * This macro allows you to iterate through each `struct sc_list_head *` pointer
 * in a list. Normally, `these` nodes are embedded within a structure at some
 * offset, which means that you need to do some pointer arithmetic to get at the
 * data you care about. The #sc_list_for_each_entry() macro takes care of this
 * for you.
 *
 * As the name may imply, this should be used as if it were a specialized for
 * loop, with a code block to execute for each iteration. The loop may be
 * terminated via break, skipped via continue, etc. The list pointers MUST NOT
 * be modified during iteration.
 */
#define list_for_each(variable, head)                                       \
	for (variable = (head)->next; variable != (head); variable = variable->next)


int list_len(struct list_head *head)
{
	int count = 0;
	struct list_head *node;
	list_for_each(node, head)
	{
		count += 1;
	}
	return count;
}

/**
 * @brief Offset, in bytes, of a field named `member` within type `type`.
 * @param type The name of a type, probably a struct (e.g `struct foo`)
 * @param member The name of a member field within @a type
 */
#define list_offsetof(type, member) ((size_t) & (((type *)0)->member))

/**
 * @brief Return a pointer to the struct containing a member:
 * @param ptr pointer to a member within a struct
 * @param type type of the containing struct (not a pointer, just the type)
 * @param member field name of member within the struct
 */
#define container_of(ptr, type, member)                                     \
	((type *)((char *)ptr - list_offsetof(type, member)))

/**
 * @brief Iterate over every item in a list. Use like a normal for statement.
 * @param var_ptr variable which is a pointer to the container type, used as the
 *     iteration variable
 * @param head pointer to struct list_head which is the header of the list
 * @param field_name name of the struct list_head within the container type
 * @param container_type type of the struct containing struct list_head (not
 *     pointer)
 *
 * This macro differs from the Linux kernel "list_for_each_entry" in that it
 * takes a fourth argument, @a container_type. Unfortunately, the `typeof`
 * construct used by the kernel to implement this is not a standard part of C89.
 * To help this library be maximally compatible, I've added this fourth
 * argument.
 *
 * @warning This interface **does not** allow the list pointers to be modified.
 * If you need this, see #list_for_each_safe().
 *
 * Sample code:
 *
 * @code{.c}
 *     struct my_example {
 *         struct list_head list;
 *         char *message;
 *     }
 *     struct list_head head;
 *
 *     // assume everything is initialized and populated
 *
 *     struct my_example *iter;
 *     list_for_each_entry(iter, &head, list, struct my_example) {
 *         printf("list entry: \"%s\"\n", iter->message);
 *     }
 * @endcode
 */
#define list_for_each_entry(var_ptr, head, field_name, container_type)      \
	for (var_ptr = container_of((head)->next, container_type,           \
	                               field_name);                            \
	     &var_ptr->field_name != (head);                                   \
	     var_ptr = container_of(var_ptr->field_name.next,               \
	                               container_type, field_name))
