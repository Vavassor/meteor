#ifndef COMPONENT_H
#define COMPONENT_H

class Component;
#include "ComponentRegistry.h"

#include "utilities/Textblock.h"

#include <string>

class Component
{
public:
	static Component* Create(const std::wstring& className);
	static void Destroy(Component* component);

	Component();
	virtual ~Component();
	virtual void Load(Textblock* block);
	virtual void Save(Textblock* block) const;
};

#define COMPONENT_REGISTER(TYPE, NAME)                                        \
    namespace detail {                                                        \
    namespace                                                                 \
    {                                                                         \
        template<class T>                                                     \
        class ComponentRegistration;                                          \
                                                                              \
        template<>                                                            \
        class ComponentRegistration<TYPE>                                     \
        {                                                                     \
            static const ::detail::RegistryEntry<TYPE>& reg;				  \
        };                                                                    \
                                                                              \
        const ::detail::RegistryEntry<TYPE>&								  \
            ComponentRegistration<TYPE>::reg =                                \
                ::detail::RegistryEntry<TYPE>::Instance(NAME);				  \
    }}

#endif
