#ifndef SCULL_QSET_H
#define SCULL_QSET_H

#define QTUM_ARRAY_SIZE 4000
#define QTUM_PTR_ARRAY_SIZE 1000

typedef char (qtum_array)[QTUM_ARRAY_SIZE];
typedef qtum_array (*qtum_array_ptr);

typedef qtum_array_ptr (qptr_array)[QTUM_PTR_ARRAY_SIZE];
typedef qptr_array (*qptr_array_ptr);

struct scull_qset
{
	struct scull_qset *qset_next; 
	qptr_array_ptr * qtum_ptr;
};
#endif