#include "Component.h"

using namespace std;

Component* Component::Create(const wstring& className)
{
	detail::ComponentRegistry& reg = detail::GetComponentRegistry();
    detail::ComponentRegistry::iterator it = reg.find(className);

    if (it != reg.end())
	{
		detail::CreateComponentFunc func = it->second;
		return func();
    }
	return nullptr;
}

void Component::Destroy(Component* component)
{
	detail::DestroyComponent(component);
}

Component::Component()
{
}

Component::~Component()
{
}

void Component::Load(Textblock* block)
{
}

void Component::Save(Textblock* block) const
{
}
