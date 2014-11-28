#ifndef MACROS_H
#define MACROS_H

#include <cstring>

#define ARRAY_COUNT(x) \
	((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define FILL(pointer, count, value) \
	memset((pointer), (value), sizeof *(pointer) * ((size_t)(count)))
	
#define CLEAR(pointer, count) \
	memset((pointer), 0, sizeof *(pointer) * ((size_t)(count)))

#define COPY(from, to, count) \
	memcpy((to), (from), sizeof *(from) * ((size_t)(count)))


#define CLEAR_STRUCT(value) \
	memset(&(value), 0, sizeof (value))


#define FOR_EACH(item, arr) \
	for(auto item = (arr).First(), end = (arr).Last(); item != end; ++item)

#define FOR_ALL(arr) \
	for(auto it = (arr).First(), end = (arr).Last(); it != end; ++it)

#define LOOP(n) \
	for(int i = 0; i < n; ++i)

#endif
