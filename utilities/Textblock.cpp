#include "Textblock.h"

#include "Macros.h"
#include "FileHandling.h"
#include "Conversion.h"
#include "Parsing.h"
#include "FileStream.h"

Attribute::Attribute():
	values(nullptr),
	numValues(0)
{}

Attribute::Attribute(const String& key, String* values, int numValues):
	key(key),
	values(values),
	numValues(numValues)
{}

Attribute::Attribute(const Attribute& attribute):
	key(attribute.key),
	values(attribute.values),
	numValues(attribute.numValues)
{}

Textblock::Textblock() {}

Textblock::~Textblock()
{
	FOR_ALL(attributes) delete[] it->values;
	FOR_ALL(children) delete (*it);
}

void Textblock::Add_Attribute(const String& name, String values[], int numValues)
{
	Attribute attribute;
	attribute.key = name;
	String* new_values = new String[numValues];
	COPY(values, new_values, numValues);
	attribute.values = new_values;
	attribute.numValues = numValues;
	attributes.Push(attribute);
}

Attribute* Textblock::Get_Attribute(const String& name) const
{
	FOR_ALL(attributes)
		if(it->key.Equals(name)) return it;

	return nullptr;
}

bool Textblock::Has_Attribute(const String& name) const
{
	return Get_Attribute(name) != nullptr;
}

bool Textblock::Get_Attribute_As_Bool(const String& name, bool* value) const
{
	Attribute* attribute = Get_Attribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		*value = attribute->values[0].Equals("true");
		return true;
	}
	return false;
}

bool Textblock::Get_Attribute_As_Int(const String& name, int* value) const
{
	Attribute* attribute = Get_Attribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		*value = string_to_int(attribute->values[0].Data());
		return true;
	}
	return false;
}

bool Textblock::Get_Attribute_As_Float(const String& name, float* value) const
{
	Attribute* attribute = Get_Attribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		*value = string_to_float(attribute->values[0].Data());
		return true;
	}
	return false;
}

bool Textblock::Get_Attribute_As_Double(const String& name, double* value) const
{
	Attribute* attribute = Get_Attribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		*value = string_to_double(attribute->values[0].Data());
		return true;
	}
	return false;
}

bool Textblock::Get_Attribute_As_Vec2(const String& name, vec2* value) const
{
	Attribute* attribute = Get_Attribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		String* values = attribute->values;
		*value = vec2(
			string_to_float(values[0].Data()),
			string_to_float(values[1].Data()));
		return true;
	}
	return false;
}

bool Textblock::Get_Attribute_As_Vec3(const String& name, vec3* value) const
{
	Attribute* attribute = Get_Attribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		String* values = attribute->values;
		*value = vec3(
			string_to_float(values[0].Data()),
			string_to_float(values[1].Data()),
			string_to_float(values[2].Data()));
		return true;
	}
	return false;
}

bool Textblock::Get_Attribute_As_Vec4(const String& name, vec4* value) const
{
	Attribute* attribute = Get_Attribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		String* values = attribute->values;
		*value = vec4(
			string_to_float(values[0].Data()),
			string_to_float(values[1].Data()),
			string_to_float(values[2].Data()),
			string_to_float(values[3].Data()));
		return true;
	}
	return false;
}

bool Textblock::Get_Attribute_As_Quaternion(const String& name, quaternion* value) const
{
	Attribute* attribute = Get_Attribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		String* values = attribute->values;
		*value = quaternion(
			string_to_float(values[0].Data()),
			string_to_float(values[1].Data()),
			string_to_float(values[2].Data()),
			string_to_float(values[3].Data()));
		return true;
	}
	return false;
}

bool Textblock::Get_Attribute_As_Strings(const String& name, String** values, int* numValues) const
{
	Attribute* attribute = Get_Attribute(name);
	if(attribute != nullptr && values != nullptr)
	{
		if(numValues != nullptr)
			*numValues = attribute->numValues;
		*values = attribute->values;
		return true;
	}
	return false;
}

void Textblock::Add_Child(Textblock* child)
{
	children.Push(child);
}

void Textblock::Remove_Child(const String& name)
{
	FOR_ALL(children)
	{
		Textblock* child = *it;
		if(child->name.Equals(name))
		{
			delete child;
			*it = nullptr;
		}
	}
}

bool Textblock::Has_Child(const String& name) const
{
	return Get_Child_By_Name(name) != nullptr;
}

Textblock* Textblock::Get_Child_By_Name(const String& name) const
{
	FOR_ALL(children)
	{
		Textblock* child = *it;
		if(child->name.Equals(name)) return child;
	}
	return nullptr;
}

static bool is_space(unsigned char c[6])
{
	return (*c == ' ')
	    || (*c - 9 <= 13 - 9)                               // \t\n\v\f\r
	    || (c[0] == 0xC2 && (c[1] == 0x85 || c[1] == 0xA0)) // next-line (NEL) and non-breaking space
	    || (c[0] == 0xEA && c[1] == 0x9A && c[2] == 0x80)   // Ogham space mark
	    || (c[0] == 0xE2 && c[1] == 0x80 && (
	           (c[2] - 0x80 <= 0xA) // sized space marks
	        || (c[2] == 0xA8)		// line separator
	        || (c[2] == 0xA9)       // paragraph separator
	        || (c[2] == 0xAF)       // narrow no-break space
	        )
	    )
	    || (c[0] == 0xE2 && c[1] == 0x81 && c[2] == 0x9F)   // medium mathematical space
	    || (c[0] == 0xE3 && c[1] == 0x80 && c[2] == 0x80);  // ideographic space
}

static bool is_line_break(unsigned char s[6])
{
	return (*s - 0xA <= 0xD - 0xA)                                           // \n\v\f\r
	    || (s[0] == 0xC2 && s[1] == 0x85)                                    // next-line (NEL)
	    || (s[0] == 0xE2 && s[1] == 0x80 && (s[2] == 0xA8 || s[2] == 0xA9)); // line separator
}

static bool parse(FileStream& in, Textblock* block)
{
	/*
	enum Mode { SET, SEQUENCE, QUOTED } mode;
	
	char data[128];
	size_t length = 0;
	while(length = in.Read(data, ARRAY_COUNT(data)))
	{
		switch(mode)
		{
			case SET:
			{
				char* start = data;
				char* end = data;


				break;
			}
			case SEQUENCE:
			{
				break;
			}
			case QUOTED:
			{
				break;
			}
		}
	}
	*/

	return true;
}

void Textblock::Load_From_File(const String& fileName, Textblock* block)
{
	FileStream stream(fileName.Data());
	parse(stream, block);
}
