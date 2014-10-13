#ifndef TEXTBLOCK_H
#define TEXTBLOCK_H

#include "String.h"
#include "GLMath.h"

#include "collections/AutoArray.h"

class Attribute
{
public:
	String key;
	String* values;
	int numValues;

	Attribute();
	Attribute(const String& key, String* values, int numValues);
	Attribute(const Attribute& attribute);
};

class Textblock
{
public:
	String name;
	AutoArray<Attribute> attributes;
	AutoArray<Textblock*> children;

	static void LoadFromFile(const String& fileName, Textblock* block);

	Textblock();
	~Textblock();

	void AddAttribute(const String& name, String values[], int numValues);
	bool HasAttribute(const String& name) const;

	bool GetAttributeAsBool(const String& name, bool* value) const;
	bool GetAttributeAsInt(const String& name, int* value) const;
	bool GetAttributeAsFloat(const String& name, float* value) const;
	bool GetAttributeAsDouble(const String& name, double* value) const;
	bool GetAttributeAsVec2(const String& name, vec2* value) const;
	bool GetAttributeAsVec3(const String& name, vec3* value) const;
	bool GetAttributeAsVec4(const String& name, vec4* value) const;
	bool GetAttributeAsQuaternion(const String& name, quaternion* value) const;
	bool GetAttributeAsStrings(const String& name, String** values, int* numValues = nullptr) const;

	void AddChild(Textblock* child);
	void RemoveChild(const String& name);
	bool HasChild(const String& name) const;
	Textblock* GetChildByName(const String& name) const;

private:
	Attribute* GetAttribute(const String& name) const;
};

#endif
