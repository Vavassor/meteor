#ifndef MACROS_H
#define MACROS_H

#define ARRAY_COUNT(x) \
	((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define FILL(pointer, count, value) \
	memset((pointer), (value), sizeof *(pointer) * ((size_t)(count)))

#define ALLOC_STRUCT(Type) \
	((Type*) ::operator new(sizeof(Type)))

#define ZERO_STRUCT(pointer) \
	memset((pointer), 0, sizeof(*(pointer)))

#define FOR_EACH(item,arr) \
	for(auto item = (arr).First(), end = (arr).Last(); item != end; ++item)

#endif
