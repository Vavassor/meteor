#include "LocalizationText.h"

/*

The MIT License

Copyright (c) 2007 Jonathan Blow (jon [at] number-none [dot] com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "FileHandling.h"

#include <cstring>

// This is just the common hashpjw routine, pasted in:

#define HASHWORDBITS 32

static inline unsigned long hashpjw(const char *str_param)
{
   unsigned long hval = 0;
   unsigned long g;
   const char *s = str_param;

   while(*s)
   {
       hval <<= 4;
       hval += (unsigned char) *s++;
       g = hval & ((unsigned long int) 0xf << (HASHWORDBITS - 4));
       if(g != 0)
       {
           hval ^= g >> (HASHWORDBITS - 8);
           hval ^= g;
       }
   }

   return hval;
}

// Swap the endianness of a 4-byte word.
// On some architectures you can replace my_swap4 with an inlined instruction.
static inline unsigned long my_swap4(unsigned long result)
{
    unsigned long c0 = (result >> 0) & 0xff;
    unsigned long c1 = (result >> 8) & 0xff;
    unsigned long c2 = (result >> 16) & 0xff;
    unsigned long c3 = (result >> 24) & 0xff;

    return (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
}

inline int Localization_Text::read4_from_offset(int offset)
{
    unsigned long *ptr = (unsigned long*)(((char*)mo_data) + offset);

    return (reversed)? my_swap4(*ptr) : *ptr;
}

inline char* Localization_Text::get_source_string(int index)
{
	int addr_offset = original_table_offset + 8 * index + 4;
	int string_offset = read4_from_offset(addr_offset);

	return ((char*)mo_data) + string_offset;
}

inline char* Localization_Text::get_translated_string(int index)
{
	int addr_offset = translated_table_offset + 8 * index + 4;
	int string_offset = read4_from_offset(addr_offset);

	return ((char*)mo_data) + string_offset;
}

bool Localization_Text::label_matches(char* s, int index)
{
    char* t = get_source_string(index);
    return strcmp(s, t) == 0;
}

inline int Localization_Text::get_target_index(char *s)
{
    unsigned long V = hashpjw(s);
    int S = hash_num_entries;

    int hash_cursor = V % S;
    int orig_hash_cursor = hash_cursor;
    int increment = 1 + (V % (S - 2));

    while(true)
    {
        unsigned int index = read4_from_offset(hash_offset + 4 * hash_cursor);
        if(index == 0) break;

        index--;  // Because entries in the table are stored +1 so that 0 means empty.

        if(label_matches(s, index)) return index;

        hash_cursor += increment;
        hash_cursor %= S;

        if(hash_cursor == orig_hash_cursor) break;
    }

    return -1;
}

Localization_Text::Localization_Text():
	mo_data(nullptr),
	reversed(0),
	num_strings(0),
	original_table_offset(0),
	translated_table_offset(0),
	hash_num_entries(0),
	hash_offset(0)
{}

Localization_Text::~Localization_Text()
{
    delete[] ((char*)mo_data);
}

bool Localization_Text::Load(const char* filename)
{
	void* data = nullptr;
	size_t length = load_binary_file(&data, filename);

	if(length < 0) return false;
	if(length < 24)
	{
		// There has to be at least this much in the header...
		Abort();
		return false;
	}

	mo_data = data;

	unsigned long* long_ptr = (unsigned long*)data;

	const unsigned long TARGET_MAGIC = 0x950412DE;
	const unsigned long TARGET_MAGIC_REVERSED = 0xDE120495;
	unsigned long magic = long_ptr[0];

	if(magic == TARGET_MAGIC)
	{
		reversed = 0;
	}
	else if(magic == TARGET_MAGIC_REVERSED)
	{
		reversed = 1;
	}
	else
	{
		Abort();
		return false;
	}

	num_strings = read4_from_offset(8);
	original_table_offset = read4_from_offset(12);
	translated_table_offset = read4_from_offset(16);
	hash_num_entries = read4_from_offset(20);
	hash_offset = read4_from_offset(24);

	if(hash_num_entries == 0)
	{
		// We expect a hash table to be there; if it's not, bail.
		Abort();
		return false;
	}

	return true;
}

void Localization_Text::Abort()
{
    delete[] (char*)mo_data;
    mo_data = NULL;
}

char* Localization_Text::LookupText(char* s)
{
	if(!mo_data) return s;

	int target_index = get_target_index(s);
	if(target_index == -1)
	{
		// Maybe we want to log an error?
		return s;
	}

	return get_translated_string(target_index);
}
