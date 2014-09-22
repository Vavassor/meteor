#include "TextBlock.h"

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

Textblock::Textblock():
	attributes(16),
	children(8)
{}

Textblock::~Textblock()
{
	FOR_EACH(attribute, attributes)
		delete[] attribute->values;
	FOR_EACH(child, children)
		delete (*child);
}

void Textblock::AddAttribute(const String& name, String values[], int numValues)
{
	Attribute attribute;
	attribute.key = name;
	String* vals = new String[numValues];
	for(int i = 0; i < numValues; i++)
		vals[i] = values[i];
	attribute.values = vals;
	attribute.numValues = numValues;
	attributes.Push(attribute);
}

Attribute* Textblock::GetAttribute(const String& name) const
{
	FOR_EACH(attribute, attributes)
		if(attribute->key == name) return attribute;
	return nullptr;
}

bool Textblock::HasAttribute(const String& name) const
{
	return GetAttribute(name) != nullptr;
}

bool Textblock::GetAttributeAsBool(const String& name, bool* value) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		*value = attribute->values[0] == "true";
		return true;
	}
	return false;
}

bool Textblock::GetAttributeAsInt(const String& name, int* value) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		*value = string_to_int(attribute->values[0].Data());
		return true;
	}
	return false;
}

bool Textblock::GetAttributeAsFloat(const String& name, float* value) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		*value = string_to_float(attribute->values[0].Data());
		return true;
	}
	return false;
}

bool Textblock::GetAttributeAsDouble(const String& name, double* value) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr && value != nullptr)
	{
		*value = string_to_double(attribute->values[0].Data());
		return true;
	}
	return false;
}

bool Textblock::GetAttributeAsVec2(const String& name, vec2* value) const
{
	Attribute* attribute = GetAttribute(name);
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

bool Textblock::GetAttributeAsVec3(const String& name, vec3* value) const
{
	Attribute* attribute = GetAttribute(name);
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

bool Textblock::GetAttributeAsVec4(const String& name, vec4* value) const
{
	Attribute* attribute = GetAttribute(name);
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

bool Textblock::GetAttributeAsQuaternion(const String& name, quaternion* value) const
{
	Attribute* attribute = GetAttribute(name);
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

bool Textblock::GetAttributeAsStrings(const String& name, String** values, int* numValues) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr && values != nullptr)
	{
		if(numValues != nullptr)
			*numValues = attribute->numValues;
		*values = attribute->values;
		return true;
	}
	return false;
}

void Textblock::AddChild(Textblock* child)
{
	children.Push(child);
}

void Textblock::RemoveChild(const String& name)
{
	FOR_EACH(block, children)
	{
		Textblock* child = *block;
		if(child->name == name)
		{
			delete child;
			*block = nullptr;
		}
	}
}

bool Textblock::HasChild(const String& name) const
{
	return GetChildByName(name) != nullptr;
}

Textblock* Textblock::GetChildByName(const String& name) const
{
	FOR_EACH(block, children)
	{
		Textblock* child = *block;
		if(child->name == name) return child;
	}
	return nullptr;
}

bool parse(Textblock* block, FileStream& in)
{
	String line;

	// TODO: implement

	return true;
}

void Textblock::LoadFromFile(const String& fileName, Textblock* block)
{
	FileStream stream(fileName.Data());
	parse(block, stream);
}
