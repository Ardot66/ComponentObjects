#include "ComponentObjects.h"

size_t ObjectGetSize(const size_t componentCount, const Component **components)
{
    size_t size = sizeof(ObjectEnd);

    for(size_t x = 0; x < componentCount; x++)
        size += components[x]->Size + sizeof(ComponentData);

    return size;
}

void ObjectInititalize(void *object, const size_t componentCount, const Component **components)
{
    char *currentComponent = (char *)object;

    for(size_t x = 0; x < componentCount; x++)
    {
        ComponentData componentData;
        componentData.Component = components[x];

        *(ComponentData *)currentComponent = componentData;
        currentComponent += componentData.Component->Size + sizeof(ComponentData);
    }

    ObjectEnd objectEnd;
    objectEnd.NullPointer = NULL;
    objectEnd.Object = object;

    *(ObjectEnd *)currentComponent = objectEnd;
}

void *ComponentGetObject(const void *component)
{
    void *currentComponent = (char *)component;
    while(COMPONENT_DATA(currentComponent)->Component != NULL)
        currentComponent = (char *)currentComponent + COMPONENT_DATA(currentComponent)->Component->Size + sizeof(ComponentData);

    ObjectEnd *objectEnd = (ObjectEnd *)COMPONENT_DATA(currentComponent);
    return objectEnd->Object;
}

int ComponentCast(const Component *component, const Interface *interface, void **interfaceVTableDest)
{
    for(size_t x = 0; x < component->ImplementsCount; x++)
    {
        if(*component->Implements[x] != interface)
            continue;

        *interfaceVTableDest = component->Implements[x];
        return 0;
    }
    
    return 1;
}