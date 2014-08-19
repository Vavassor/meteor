#include "TextBlock.h"

#include "Macros.h"
#include "FileHandling.h"
#include "Conversion.h"
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

bool Textblock::GetAttributeAsBool(const String& name) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr)
		return attribute->values[0] == "true";
	return false;
}

int Textblock::GetAttributeAsInt(const String& name) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr)
		return string_to_int(attribute->values[0].Data());
	return 0;
}

float Textblock::GetAttributeAsFloat(const String& name) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr)
		return string_to_float(attribute->values[0].Data());
	return 0.0f;
}

double Textblock::GetAttributeAsDouble(const String& name) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr)
		return string_to_double(attribute->values[0].Data());
	return 0.0;
}

vec2 Textblock::GetAttributeAsVec2(const String& name) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr)
	{
		String* values = attribute->values;
		return vec2(string_to_float(values[0].Data()),
					string_to_float(values[1].Data()));
	}
	return VEC2_ZERO;
}

vec3 Textblock::GetAttributeAsVec3(const String& name) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr)
	{
		String* values = attribute->values;
		return vec3(string_to_float(values[0].Data()),
					string_to_float(values[1].Data()),
					string_to_float(values[2].Data()));
	}
	return VEC3_ZERO;
}

vec4 Textblock::GetAttributeAsVec4(const String& name) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr)
	{
		String* values = attribute->values;
		return vec4(string_to_float(values[0].Data()),
					string_to_float(values[1].Data()),
					string_to_float(values[2].Data()),
					string_to_float(values[3].Data()));
	}
	return VEC4_ZERO;
}

quaternion Textblock::GetAttributeAsQuaternion(const String& name) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr)
	{
		String* values = attribute->values;
		return quaternion(
			string_to_float(values[0].Data()),
			string_to_float(values[1].Data()),
			string_to_float(values[2].Data()),
			string_to_float(values[3].Data()));
	}
	return QUAT_I;
}

String* Textblock::GetAttributeAsStrings(const String& name, int* numValues) const
{
	Attribute* attribute = GetAttribute(name);
	if(attribute != nullptr)
	{
		if(numValues != nullptr)
			*numValues = attribute->numValues;
		return attribute->values;
	}
	return nullptr;
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


	return true;
}

void Textblock::LoadFromFile(const String& fileName, Textblock* block)
{
	FileStream stream(fileName.Data());
	parse(block, stream);
}
