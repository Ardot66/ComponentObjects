#include "ComponentObjects.h"
#include <stdlib.h>
#include <string.h>

#define OBJECT_DATA_STACK_STARTING_SIZE 256

size_t ObjectDataStackSize = 0;
size_t ObjectDataStackCount = 0;
void *ObjectDataStack = NULL;

int ObjectDataStackPush(void **dataDest, const size_t dataSize)
{
    if(ObjectDataStack == NULL)
    {
        ObjectDataStack = malloc(OBJECT_DATA_STACK_STARTING_SIZE);

        if(ObjectDataStack == NULL)
            return errno;
    }

    if(ObjectDataStackSize < ObjectDataStackCount + dataSize)
    {
        size_t newStackSize = (ObjectDataStackCount + dataSize) * 2;
        void *temp = realloc(ObjectDataStack, newStackSize);

        if(temp == NULL)
            return errno;

        ObjectDataStackSize = newStackSize;
        ObjectDataStack = temp;
    }

    *dataDest = (char *)ObjectDataStack + ObjectDataStackCount;
    ObjectDataStackCount += dataSize;

    return 0;
}

void ObjectDataStackPop(const void *data)
{
    ObjectDataStackCount = (char *)data - (char *)ObjectDataStack;
}

int ObjectInitialize(ObjectData **objectDataDest, const size_t componentCount, const Component **components)
{
    int result;

    ObjectData objectData;
    objectData.TotalSize = sizeof(Object);
    if(result = ObjectDataStackPush(&objectData.Components, sizeof(objectData.Components) * componentCount)) return result;
    
    for(size_t x = 0; x < componentCount; x++)
    {
        ObjectComponent objectComponent;
        objectComponent.Component = components[x];
        objectComponent.Offset = objectData.TotalSize;

        objectData.TotalSize += objectComponent.Component->Size;
        
        ObjectComponent *stackObjectComponent;
        if(result = ObjectDataStackPush(&stackObjectComponent, sizeof(ObjectComponent) + sizeof(ObjectComponentInterface *) * objectComponent.Component->UsesCount)) return result;

        *stackObjectComponent = objectComponent;
    }

    

    for(size_t x = 0; x < componentCount; )

    ObjectData objectDataInit = {.TotalSize = totalSize, .Components = objectComponents, .Interfaces = objectInterfaces};

    ObjectData *objectData;
    ObjectDataStackPush((void **)&objectData, sizeof(*objectData));
    memcpy(objectData, &objectDataInit, sizeof(*objectData));

    *objectDataDest = objectData;
    return 0;
}

size_t ObjectGetSize(const size_t componentCount, const Component **components)
{
    size_t size = sizeof(Object);

    for(size_t x = 0; x < componentCount; x++)
        size += components[x]->Size;

    return size;
}

// void ObjectInititalize(void *object, const size_t componentCount, const Component **components)
// {
//     char *currentComponent = (char *)object;

//     for(size_t x = 0; x < componentCount; x++)
//     {
//         ComponentData componentData;
//         componentData.Component = components[x];

//         *(ComponentData *)currentComponent = componentData;
//         currentComponent += componentData.Component->Size + sizeof(ComponentData);
//     }

//     ObjectEnd objectEnd;
//     objectEnd.NullPointer = NULL;
//     objectEnd.Object = object;

//     *(ObjectEnd *)currentComponent = objectEnd;
// }

// void *ComponentGetObject(const void *component)
// {
//     void *currentComponent = (char *)component;
//     while(COMPONENT_DATA(currentComponent)->Component != NULL)
//         currentComponent = (char *)currentComponent + COMPONENT_DATA(currentComponent)->Component->Size + sizeof(ComponentData);

//     ObjectEnd *objectEnd = (ObjectEnd *)COMPONENT_DATA(currentComponent);
//     return objectEnd->Object;
// }

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