#include <stdlib.h>

/********************************************************************************
 *
 * A List library.
 *
 * This library provides a List of doubles data structure.
 *
 * This library will fail in unpredictable ways when the system memory runs
 * out.
 *
 ********************************************************************************/

/*
 * List represents a List data structure.
 */

// Node & List Data Structures
typedef struct _node
{
	double _speed;
	double _bandwith;
	struct _node *_next;
} Node;

typedef struct _List
{
	Node *_head;
	size_t _size;
}List;

/*
 * Allocates a new empty List.
 * It's the user responsibility to free it with List_free.
 */
List* List_alloc();

/*
 * Frees the memory and resources allocated to list.
 * If list==NULL does nothing (same as free).
 */
void List_free(List* list);

/*
 * Inserts an element in the begining of the list.
 */
void List_insertLast(List *list, const double speed, const double bandwith);

/*
 * Prints the list to the standard output.
 */
void List_print(const List* list);

/*
 * Claculates the Average runtime of the list
 */
double calcAvgTime(const List *list);

/*
 * Claculates the Average bandwith of the list
 */
double calcAvgBandwith(const List *list);

