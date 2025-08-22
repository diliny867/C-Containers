#pragma once

#include <stdlib.h>

/* example:
struct ddarray {
	int* data;
	int count;
	int cap;
}
*/

#define _DDA_DEFAULT_CAP 32

#define dda_push(dda, val) \
	do { \
		if(!dda) break; \
		if(dda->count >= dda->cap){ \
			if(dda->cap == 0) dda->cap = _DDA_DEFAULT_CAP; \
			else dda->cap *= 2; \
			dda->data = realloc(dda->data, dda->cap * sizeof(*dda->data)) \
		} \
		dda->data[count++] = val \
	while(0);


#define sdda_push(dda, val) \
	do { \
		if(dda.count >= dda.cap){ \
			if(dda.cap == 0) dda.cap = _DDA_DEFAULT_CAP; \
			else dda.cap *= 2; \
			dda.data = realloc(dda.data, dda.cap * sizeof(*dda.data)) \
		} \
		dda.data[count++] = val \
	while(0);

