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

	static void Load_From_File(const String& fileName, Textblock* block);

	Textblock();
	~Textblock();

	void Add_Attribute(const String& name, String values[], int numValues);
	bool Has_Attribute(const String& name) const;

	bool Get_Attribute_As_Bool(const String& name, bool* value) const;
	bool Get_Attribute_As_Int(const String& name, int* value) const;
	bool Get_Attribute_As_Float(const String& name, float* value) const;
	bool Get_Attribute_As_Double(const String& name, double* value) const;
	bool Get_Attribute_As_Vec2(const String& name, vec2* value) const;
	bool Get_Attribute_As_Vec3(const String& name, vec3* value) const;
	bool Get_Attribute_As_Vec4(const String& name, vec4* value) const;
	bool Get_Attribute_As_Quaternion(const String& name, quaternion* value) const;
	bool Get_Attribute_As_Strings(const String& name, String** values, int* numValues = nullptr) const;

	void Add_Child(Textblock* child);
	void Remove_Child(const String& name);
	bool Has_Child(const String& name) const;
	Textblock* Get_Child_By_Name(const String& name) const;

private:
	Attribute* Get_Attribute(const String& name) const;
};

#endif
