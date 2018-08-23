#ifndef SCULL_QSET_H
#define SCULL_QSET_H

#define QTUM_ARRAY_SIZE 4000
#define QTUM_PTR_ARRAY_SIZE 1000

typedef char (qtum_array)[QTUM_ARRAY_SIZE];

typedef qtum_array* (qptr_array)[QTUM_PTR_ARRAY_SIZE];

struct scull_qset
{
	struct scull_qset *qset_next; 
	qptr_array   *qtum_ptr;
};
#endif