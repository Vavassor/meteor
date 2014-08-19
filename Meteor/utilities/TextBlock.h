#ifndef TEXTBLOCK_H
#define TEXTBLOCK_H

#include "BString.h"
#include "AutoArray.h"
#include "GLMath.h"

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

	bool GetAttributeAsBool(const String& name) const;
	int GetAttributeAsInt(const String& name) const;
	float GetAttributeAsFloat(const String& name) const;
	double GetAttributeAsDouble(const String& name) const;
	vec2 GetAttributeAsVec2(const String& name) const;
	vec3 GetAttributeAsVec3(const String& name) const;
	vec4 GetAttributeAsVec4(const String& name) const;
	quaternion GetAttributeAsQuaternion(const String& name) const;
	String* GetAttributeAsStrings(const String& name, int* numValues = nullptr) const;

	void AddChild(Textblock* child);
	void RemoveChild(const String& name);
	bool HasChild(const String& name) const;
	Textblock* GetChildByName(const String& name) const;

private:
	Attribute* GetAttribute(const String& name) const;
};

#endif
