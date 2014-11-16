#ifndef COMPONENT_REGISTRY_H
#define COMPONENT_REGISTRY_H

#include <string>
#include <map>

namespace detail
{
    typedef Component* (*CreateComponentFunc)();
    typedef std::map<std::wstring, CreateComponentFunc> ComponentRegistry;

    inline ComponentRegistry& GetComponentRegistry()
    {
        static ComponentRegistry reg;
        return reg;
    }

    template<class T>
    Component* CreateComponent() {
        return new T;
    }

	void DestroyComponent(Component* comp) {
		delete comp;
	}

    template<class T>
    struct RegistryEntry
    {
	public:
        static RegistryEntry<T>& Instance(const std::wstring& name)
        {
            static RegistryEntry<T> inst(name);
            return inst;
        }

    private:
        RegistryEntry(const std::wstring& name)
        {
            ComponentRegistry& reg = GetComponentRegistry();
            CreateComponentFunc func = CreateComponent<T>;

            std::pair<ComponentRegistry::iterator, bool> ret =
                reg.insert(ComponentRegistry::value_type(name, func));

            if (ret.second == false) {
                // This means there already is a component registered to
                // this name. You should handle this error as you see fit.
            }
        }
    };
}

#endif