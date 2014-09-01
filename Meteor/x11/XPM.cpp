#include "XPM.h"

#include "XPMNamedColor.h"

#include "utilities/Logging.h"
#include "utilities/Parsing.h"
#include "utilities/Macros.h"

#include <cstring>
#include <cctype>
#include <cstdlib>

#define XPM_MAX_COLOR_LINE_SIZE 128

/* We can use only 95 ASCII chars when coding colors [32-127]
 * but use 96 to speed up ariphmetic ((x << 6) + (x << 5))
 * Returns x * 96
 */
#define XPM_ASCII_RANGE(x) (((x) << 6) + ((x) << 5))

namespace x_pixmap {

namespace
{
	typedef unsigned long color_t;
	typedef unsigned char comp_t;

	enum ColorKey
	{
		KEY_MONO	= 0,
		KEY_GREY4	= 1,
		KEY_GRAY4	= 1,
		KEY_GREY	= 2,
		KEY_GRAY	= 2,
		KEY_COLOR	= 3,
		KEY_SYMBOL	= 4,
		KEY_UNKNOWN = 5,
	};

	struct Bitmap
	{
		unsigned int width, height;
		color_t* pixels;
	};

	struct MetaData
	{
		Bitmap* bitmap;
		unsigned int chpp; // number of characters per pixel
		unsigned int numColors;
		color_t* table;
		unsigned int tableSize;
		char* cids;					// array of color id's
	};
}

static color_t combine_rgba(comp_t r, comp_t g, comp_t b, comp_t a)
{
	return color_t(r) << 24 | color_t(g) << 16 | color_t(b) << 8 | a;
}

static color_t color_name_to_rgba(char *cname)
{
	NamedColor* cn;

	char* color = strdup(cname);
	if(color == nullptr)
	{
		DEBUG_LOG("Can't allocate memory for color name copy (%s)", cname);
		return -1;
	}
	int len = strlen(cname);

	/* Strip spaces and lowercase */
	char* tmp = color;
	while(*tmp)
	{
		if (' ' == *tmp)
		{
			memmove(tmp, tmp+1, color + len - tmp);
			continue;
		}
		else
		{
			*tmp = tolower(*tmp);
		}
		++tmp;
	}

	/* Check for transparent color */
	if(0 == strcmp(color, "none"))
	{
		free(color);
		/* Return black transparent color */
		return combine_rgba(0, 0, 0, 255);
	}

	/* check for "grey" */
	tmp = strstr(color, "grey");
	if(tmp)
	{
		tmp[2] = 'a';	/* Convert to "gray" */
	}

	/* Binary search in color names array */
	len = ARRAY_LENGTH(colorNames);

	int half = len >> 1;
	int rest = len - half;

	cn = colorNames + half;
	len = 1; /* Used as flag */

	while(half > 0)
	{
		half = rest >> 1;

		len = strcmp(tmp, cn->name);
		if(len < 0)
			cn -= half;
		else if (len > 0)
			cn += half;
		else break;

		rest -= half;
	}

	free(color);
	if(len == 0)
	{
		// Found
		return cn->rgba;
	}
	else
	{
		// Not found
		Log::Add(Log::INFO, "Color name '%s' not in colors database, returning transparent red", cname);
		// Return 'red' color like libXpm does
		return combine_rgba(255, 0, 0, 255);
	}
}

static void parse_cline(char* data, char** colors)
{
	// Clear colors
	colors[KEY_MONO] = nullptr;
	colors[KEY_GRAY4] = nullptr;
	colors[KEY_GRAY] = nullptr;
	colors[KEY_COLOR] = nullptr;

	char* e = data + strlen(data);
	char* color = nullptr;
	ColorKey key = KEY_UNKNOWN;
	ColorKey newkey = KEY_UNKNOWN;
	char* p = data;

	// Iterate over (<ckey> <color>) pairs
	int len = 0;
	while(p < e)
	{
		char* tmp = next_word(p, &p);
		if(tmp)
		{
			len = p - tmp;

			// If there is no color data and there is no key found
			// it is start of the color data
			if(color == nullptr && key != KEY_UNKNOWN)
				color = tmp;

			newkey = KEY_UNKNOWN;

			if(len == 1)
			{
				switch(tmp[0])
				{
					case 'c': newkey = KEY_COLOR;	break;
					case 'm': newkey = KEY_MONO;	break;
					case 'g': newkey = KEY_GRAY;	break;
					case 's': newkey = KEY_SYMBOL;	break;
				}
			}
			else if(len == 2 && tmp[0] == 'g' && tmp[1] == '4')
			{
				newkey = KEY_GRAY4;
			}
		}

		// Do we have found new key or end of string?
		if(newkey != KEY_UNKNOWN || tmp == nullptr || *p == '\0')
		{
			// If we have color and this is not symbolic key store it
			if(color != nullptr && key != KEY_SYMBOL)
			{
				// skip back key and zero-terminate the color name
				*(p - len - 1) = '\0';
				colors[key] = color;
				color = nullptr;
			}

			key = newkey;
		}

		if(tmp == nullptr) return;
	}
}

static bool parse_colors(char** xpm_data, MetaData* meta)
{
	// Array of colors in line
	char* colors[KEY_SYMBOL];
	// Color line buffer
	char line[XPM_MAX_COLOR_LINE_SIZE];

	int chpp = meta->chpp;
	char* cidptr = meta->cids;
	color_t* table = meta->table;

	for(char** data = xpm_data; data < xpm_data + meta->numColors; ++data)
	{
		/* Create temporary copy for parsing (w/o color id) */
		strncpy(line, *data + chpp, sizeof(line) - 1);
		line[sizeof(line)-1] = '\0';

		parse_cline(line, colors);

		// select color
		char* color = colors[KEY_COLOR];
		if(color == nullptr)
			color = colors[KEY_GRAY];
		if(color == nullptr)
			color = colors[KEY_GRAY4];
		if(color == nullptr)
			color = colors[KEY_MONO];

		if(color == nullptr)
		{
			LOG_ISSUE("Wrong XPM format: wrong colors line '%s'", *data);
			return false;
		}

		// Get rgba value from color hex or name
		color_t cval = (*color == '#') ? hex_string_to_rgba(color) : color_name_to_rgba(color);

		/* Store color value */
		/* NOTE: following conditions are mutually exclusive within
		 * single image so it's possible to move ctable pointer
		 */
		if(chpp <= 2)
		{
			/* Build colors lookup table */
			unsigned char c1 = (unsigned char) (*data)[0];
			unsigned char c2 = '\0';
			if(chpp == 2) c2 = (unsigned char) (*data)[1];

			if(c1 < 32 || c1 > 127 ||
				((chpp == 2) && (c2 < 32 || c2 > 127)))
			{
				LOG_ISSUE("Pixel char is out of range [32-127]");
			}
			else
			{
				c1 -= ' ';
				if(chpp == 1)
				{
					table[c1] = cval;
				}
				else
				{
					/* (y * 96 + x) */
					table[XPM_ASCII_RANGE(c1) + c2 - ' '] = cval;
				}
			}
		}
		else
		{
			// Build color id's array
			strncpy(cidptr, *data, chpp);
			cidptr += chpp;

			// Store color value
			*(table++) = cval;
		}
	}
	return true;
}

static bool parse_pixels(char** data, MetaData* meta)
{
	int chpp = meta->chpp;
	color_t* ctable = meta->table;
	color_t* pixptr = meta->bitmap->pixels;
	int cwidth = chpp * meta->bitmap->width;
	char* e = meta->cids + meta->numColors * chpp;

	for(char** d = data; d < data + meta->bitmap->height; ++d)
	{
		int dlen = strlen(*d);
		if(dlen != cwidth)
		{
			LOG_ISSUE("Wrong XPM format: pixel data length is not equal to width (%d != %d)",
				dlen, cwidth);
			return false;
		}

		/* Iterate over pixels (every chpp chars) */
		for(char* p = *d; p < *d + dlen; p += chpp)
		{
			/* NOTE: following conditions are mutually exclusive within
			* single image so it's possible to move ctable pointer
			*/
			if(chpp <= 2)
			{
				/* Use lookup table */
				unsigned char c1 = (unsigned char) *p;
				unsigned char c2 = '\0';
				if(2 == chpp) c2 = (unsigned char) *(p + 1);

				if(c1 < 32 || c1 > 127 ||
					(chpp == 2) && (c2 < 32 || c2 > 127))
				{
					LOG_ISSUE("Pixel char is out of range [32-127]");

					// Consider this pixel as transparent
					*pixptr = combine_rgba(0, 0, 0, 255);
				}
				else
				{
					c1 -= ' ';
					if(1 == chpp)
						*pixptr = ctable[c1];
					else
						*pixptr = ctable[XPM_ASCII_RANGE(c1) + c2 - ' '];
				}
			}
			else
			{
				// Search pixel
				bool found = 0;
				for(char* id = meta->cids; id < e; id += chpp, ctable++)
				{
					if(0 == strncmp(id, p, chpp))
					{
						*pixptr = *ctable;
						found = true;
						break;
					}
				}
				if(!found)
				{
					// Consider this pixel as transparent
					*pixptr = combine_rgba(0, 0, 0, 255);
				}
			}
			++pixptr;
		}
	}
	return true;
}

Bitmap* parse_bitmap(char** data, const int rows)
{
	if(data == nullptr) return nullptr;

	if(rows < 3)
	{
		LOG_ISSUE("XPM image array should have at least 3 rows!");
		return nullptr;
	}

	/* Parse image values */
	char* p;
	int width = next_int(data[0], &p);
	int height = next_int(p, &p);
	int ncolors = next_int(p, &p);
	int chpp = next_int(p, &p);

	if(width < 0 || height < 0 || ncolors < 0 || chpp < 0)
	{
		LOG_ISSUE("Wrong XPM format: wrong values (%d, %d, %d, %d)",
			width, height, ncolors, chpp);
		return nullptr;
	}

	if(rows != (1 + ncolors + height))
	{
		LOG_ISSUE("Wrong XPM format: passed and parsed sizes are not equal (%d != %d)",
			rows, 1 + ncolors + height);
		return nullptr;
	}

	if(ncolors > (1 << (8 * chpp)))
	{
		LOG_ISSUE("Wrong XPM format: there are more colors than char_per_pixel can serve (%d > %d)",
			ncolors, 1 << (8 * chpp));
		return nullptr;
	}

	/* Prepare return values */
	Bitmap* bitmap = new Bitmap;
	if(bitmap == nullptr)
	{
		LOG_ISSUE("Can't allocate memory for return values");
		return nullptr;
	}

	// Store values
	bitmap->width = width;
	bitmap->height = height;

	MetaData meta = {};
	meta.numColors = ncolors;
	meta.chpp = chpp;
	meta.bitmap = bitmap;

	// Allocate place for color values
	if(chpp == 1)
	{
		meta.tableSize = XPM_ASCII_RANGE(1); /* 96 */
		meta.cids = nullptr;
	}
	else if(chpp == 2)
	{
		meta.tableSize = XPM_ASCII_RANGE(meta.tableSize); /* (96 * 96) */
		meta.cids = nullptr;
	}
	else
	{
		/* Allocate place for color id's (ncolors * chpp chars)
		 * Only used when no lookup table is applicable
		 * NOTE: id's are stored w/o terminating '\0'
		 */
		meta.cids = new char[ncolors * chpp];
		meta.tableSize = ncolors;
	}

	// Parse colors data
	meta.table = new color_t[meta.tableSize];
	if(!parse_colors(data + 1, &meta))
	{
		LOG_ISSUE("Can't parse xpm colors");
		goto free_ctable;
	}

	// Parse pixels data
	bitmap->pixels = new color_t[width * height];
	if(!parse_pixels(data + 1 + ncolors, &meta))
	{
		LOG_ISSUE("Can't parse xpm pixels");
		goto free_ctable;
	}

free_ctable:
	delete[] meta.table;
	delete[] meta.cids;

	return bitmap;
}

unsigned long* parse_image(char** data, const int rows)
{
	Bitmap* bitmap = parse_bitmap(data, rows);

	color_t* images = new color_t[2 + bitmap->width * bitmap->height];
	images[0] = bitmap->width;
	images[1] = bitmap->height;
	memcpy(&images[2], bitmap->pixels, sizeof(color_t) * bitmap->width * bitmap->height);

	return images;
}

} // namespace x_pixmap
