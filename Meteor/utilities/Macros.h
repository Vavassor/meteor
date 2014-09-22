#ifndef MACROS_H
#define MACROS_H

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#endif

#define ALLOC_STRUCT(Type) (Type*) malloc(sizeof(Type))
#define ZERO_STRUCT(pointer) memset((pointer), 0, sizeof(*(pointer)))

#define FOR_EACH(item,arr) for(auto item = (arr).First(), end = (arr).Last(); item != end; ++item)
#define for_each(iter,arr) for(auto iter = (arr).begin(), end = (arr).end(); iter != end; ++iter)

#endif
