#ifndef MACROS_H
#define MACROS_H

#define ARRAY_COUNT(x) \
	((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define FILL(pointer, count, value) \
	memset((pointer), (value), sizeof *(pointer) * ((size_t)(count)))

#define COPY(from, to, count)        \
	for(int i = 0; i < (count); ++i) \
		(to)[i] = (from)[i];


#define ALLOCATE_STRUCT(Type) \
	(((Type)*) ::operator new(sizeof(Type)))

#define ZERO_STRUCT(pointer) \
	memset((pointer), 0, sizeof(*(pointer)))

#define FREE_STRUCT(pointer) \
	::operator delete(pointer)


#define FOR_EACH(item,arr) \
	for(auto item = (arr).First(), end = (arr).Last(); item != end; ++item)

#define FOR_ALL(arr) \
	for(auto it = (arr).First(), end = (arr).Last(); it != end; ++it)

#define LOOP(n) \
	for(int i = 0; i < n; ++i)

#endif
