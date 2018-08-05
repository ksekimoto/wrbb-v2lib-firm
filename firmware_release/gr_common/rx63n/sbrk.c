/*
 * sbrk.c
 *
 *  Created on: 2018/08/05
 *      Author: jbh00
 */

extern char end; /* Set by linker.  */
extern char heapmax;
char *heap_end = 0;

void *sbrk(int incr)
{
	char * prev_heap_end;

	if (heap_end == 0)
		heap_end = &end;

	if (((int)heap_end + incr) > (int)(&heapmax))
		return (void *)-1;

	prev_heap_end = heap_end;
	heap_end += incr;

	return (void *) prev_heap_end;
}

