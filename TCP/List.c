#include "List.h"
#include <stdlib.h>
#include <stdio.h>

// Node & List Data Structures
typedef struct _node
{
	double _speed;
	double _bandwith;
	struct _node *_next;
} Node;

struct _List
{
	Node *_head;
	size_t _size;
};

//------------------------------------------------
// Node implementation
//------------------------------------------------

Node *Node_alloc(double speed, double bandwith,
				 Node *next)
{
	Node *p = (Node *)malloc(sizeof(Node));
	p->_speed = speed;
	p->_bandwith = bandwith;
	return p;
}

void Node_free(Node *node)
{
	free(node);
}
//------------------------------------------------

//------------------------------------------------
// List implementation
//------------------------------------------------

List *List_alloc()
{
	List *p = (List *)malloc(sizeof(List));
	p->_head = NULL;
	p->_size = 0;
	return p;
}

void List_free(List *list)
{
	if (list == NULL)
		return;
	Node *p1 = list->_head;
	Node *p2;
	while (p1)
	{
		p2 = p1;
		p1 = p1->_next;
		Node_free(p2);
	}
	free(list);
}

/*
 * Inserts an element in the end of the StrList.
 */
void List_insertLast(List *list, const double speed, const double bandwith)
{
	if (list->_head == NULL)
	{
		list->_head = Node_alloc(speed, bandwith, NULL);
		++(list->_size);
	}
	else
	{
		Node *curr = list->_head;
		Node *new = Node_alloc(speed, bandwith, NULL);
		if (new != NULL)
		{
			while (curr->_next != NULL)
			{
				curr = curr->_next;
			}
			curr->_next = new;
			++(list->_size);
		}
	}
}

void List_print(const List *list)
{
	int i = 1;
	Node* p = list->_head;
	printf("\n");
	printf("----------------------------------\n");
	printf("-          * Statistics *        -\n");

	while (p)
	{
		printf("- Run #%d Data: Time=%.2fms; Speed=%.2fMB/s\n", i, p->_speed, p->_bandwith);
		i++;
		p = p->_next;
	}

	printf("-								  \n");
	printf("- Average time: %.2fms\n", calcAvgTime(list));
	printf("- Average bandwith: %.2fMB/s\n", calcAvgBandwith(list));
	printf("----------------------------------\n");
}

double calcAvgTime(const List *list)
{
	double sum = 0;
	Node* p = list->_head;

	while(p)
	{
		sum += p->_speed;
		p = p->_next;
	}

	return sum / (double)list->_size;
}

double calcAvgBandwith(const List *list)
{
	double sum = 0;
	Node* p = list->_head;

	while(p)
	{
		sum += p->_bandwith;
		p = p->_next;
	}

	return sum / (double)list->_size;
}
