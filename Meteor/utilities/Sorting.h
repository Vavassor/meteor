#ifndef SORTING_H
#define SORTING_H

 /* Templated Sorting Algorithm Notes:
  *
  * templated ItemType proved to be faster than void* array and run-time size
  * calculation because it allowed the size calculation and element swapping
  * to be optimized at compile-time. Similarly, using a templated functor 
  * instead of a function pointer allows the functor comparisons to be inlined
  */

template<typename ItemType, typename IsLessFunctor>
void insertion_sort(ItemType* array, unsigned size, const IsLessFunctor& compare)
{
    for(unsigned i = 1; i < size; ++i)
    {
        ItemType val = array[i];
        unsigned j;
        for(j = i; j > 0 && compare(val, array[j - 1]); --j)
            array[j] = array[j - 1];
        array[j] = val;
    }
}

template<typename ItemType, typename IsLessFunctor>
void quick_sort_r(ItemType* array, unsigned left, unsigned right, const IsLessFunctor& compare)
{
    while(left + 16 < right)
    {
        ItemType v1 = array[left];
		ItemType v2 = array[right]; 
		ItemType v3 = array[(left + right) / 2];
        ItemType median =
            (compare(v1, v2)) ?
			(compare(v3, v1) ? v1 : (!compare(v3, v2) ? v2 : v3)) :
			(compare(v3, v2) ? v2 : (!compare(v3, v1) ? v1 : v3));

		unsigned i = left - 1;
		unsigned j = right + 1;
		unsigned m;
		while(true)
		{
			while(compare(median, array[--j]));
			while(compare(array[++i], median));
			if(i < j)
			{
				ItemType tmp = array[i];
				array[i] = array[j];
				array[j] = tmp;
			}
			else
			{
				m = j;
				break;
			}
		}
        quick_sort_r(array, left, m, compare);
        left = m + 1;
    }
}

/* Median Hybrid version of Quick Sort
 *
 * compare should be implemented as a functor with the following member function:
 *		int operator()(const ItemType& a, const ItemType& b) const
 * which should return 0 if the items are equal, -1 if a < b and 1 if a < b
 * 
 * Performance Note: this experimentally outperformed std::sort except in cases
 * where data contained many duplicate elements. This is because it uses
 * insertion sort instead of heap sort for the mostly-sorted array at the end.
 */
template<typename ItemType, typename IsLessFunctor>
void quick_sort(ItemType* array, unsigned size, const IsLessFunctor& compare)
{
    quick_sort_r(array, 0, size - 1, compare);
    insertion_sort(array, size, compare);
}

/* Merge Sort
 * requires a memory buffer of length \n\ to use as temporary swap space
 *
 * is_less should be implemented as a functor with the following member function:
 *		bool operator()(const ItemType& a, const ItemType& b) const
 *
 * since merge sort is a stable sort, equal items are not moved relative to one another, 
 * so the comparison function only needs to compare less-than/greater-than, 
 * which is why it returns a bool
 */
template<typename T, typename IsLessFunctor>
void merge_sort(T* array, T* buffer, size_t n, const IsLessFunctor& is_less)
{
	// Unfold the first pass
	for(size_t f = 1; f < n; f += 2)
	{
		if(is_less(array[f], array[f - 1]))
		{
			T temp = array[f];
			array[f] = array[f - 1];
			array[f - 1] = temp;
		}
	}

	// Now all sublists of p are already sorted.
	bool s = false;
	for(size_t p = 2; p != 0 && p < n; p <<= 1, s = !s)
	{
		T* z = buffer;
		for(size_t i = 0; i < n; i += p << 1)
		{
			T* x = array + i;
			T* y = x + p;
			size_t xn = (p < n - i)? p : n - i;
			size_t yn = (p << 1 < n - i)? p :
			            (p < n - i)? n - p - i :
			            0;
			if(xn > 0 && yn > 0 && is_less(*y, x[xn - 1]))
			{
				while(true)
				{
					if(is_less(*y, *x))
					{
						*z++ = *y++;
						if(--yn == 0) break;
					}
					else
					{
						*z++ = *x++;
						if(--xn == 0) break;
					}
				}
			}
			// Copy from *x first because of (S).
			while(xn > 0)
			{  
				*z++ = *x++;
				--xn;
			}
			while(yn > 0)
			{
				*z++ = *y++;  
				--yn;
			}
		}  
		z = array;
		array = buffer;
		buffer = z;
	}

	// Copy from buffer to result.
	if(s)
	{
		for (T *x = buffer, *y = array, *const x_end = buffer + n; x != x_end; ++x, ++y)
			*x = *y;
	}
}

#endif
