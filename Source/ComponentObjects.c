#include "ComponentObjects.h"
#include <stdlib.h>

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

    if(dataDest != NULL)
        *dataDest = (char *)ObjectDataStack + ObjectDataStackCount;
    ObjectDataStackCount += dataSize;

    return 0;
}

void ObjectDataStackPop(const size_t amount)
{
    ObjectDataStackCount -= amount;
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

int ObjectInitialize(ObjectData **objectDataDest, const size_t componentCount, const Component **components)
{
    int result;

    ObjectData objectData;
    objectData.TotalSize = sizeof(Object);
    objectData.ComponentsCount = componentCount;
    objectData.InterfacesCount = 0;
    if(result = ObjectDataStackPush(&objectData.Components, sizeof(objectData.Components) * componentCount)) return result;
    
    //Figuring out the total object size

    

    for(size_t x = 0; x < componentCount; x++)
    {
        ObjectComponent objectComponent;
        objectComponent.Component = components[x];
        objectComponent.Offset = objectData.TotalSize;

        objectData.TotalSize += objectComponent.Component->Size;
        
        ObjectComponent *stackObjectComponent;
        if(result = ObjectDataStackPush(&stackObjectComponent, sizeof(objectComponent) + sizeof(*objectComponent.Uses) * objectComponent.Component->UsesCount)) return result;

        *stackObjectComponent = objectComponent;
        objectData.Components[x] = stackObjectComponent;
    }

    size_t objectDataInterfacesLength = 4;
    if(result = ObjectDataStackPush(&objectData.Interfaces, objectDataInterfacesLength * sizeof(ObjectInterface **))) return result;

    for(size_t x = 0; x < componentCount; x++)
    {
        Component *component = components[x];

        for(size_t y = 0; y < component->ImplementsCount; y++)
        {
            Interface *interface = component->Implements[y];

            int interfaceAlreadyFound = 0;

            for(size_t z = 0; z < objectData.InterfacesCount; z++)
                interfaceAlreadyFound |= objectData.Interfaces[z].Interface == interface;
            
            if(interfaceAlreadyFound)
                continue;

            if(objectDataInterfacesLength <= objectData.InterfacesCount)
            {
                size_t newObjectDataInterfacesSize = objectDataInterfacesLength * 2;
                if(result = ObjectDataStackPush(NULL, (newObjectDataInterfacesSize - objectDataInterfacesLength) * sizeof(*objectData.Interfaces))) return result;

                objectDataInterfacesLength = newObjectDataInterfacesSize;
            }

            ObjectInterface objectInterface;
            objectInterface.Interface = interface;

            objectData.Interfaces[objectData.InterfacesCount] = objectInterface;
            objectData.InterfacesCount++;
        }
    }

    ObjectDataStackPop((objectDataInterfacesLength - objectData.InterfacesCount) * sizeof(objectData.Interfaces));

    for(size_t x = 0; x < objectData.InterfacesCount; x++)
    {
        ObjectInterface *objectInterface = objectData.Interfaces + x;

        size_t objectInterfaceImplementingComponentsLength = 4;
        if(result = ObjectDataStackPush(&objectInterface->ImplementingComponents, objectInterfaceImplementingComponentsLength * sizeof(*objectInterface->ImplementingComponents))) return result;
        
        for(size_t y = 0; y < componentCount; y++)
        {
            ObjectComponent *objectComponent = objectData.Components[y];

            void *componentVTable;
            if(ComponentCast(objectComponent->Component, objectInterface->Interface, &componentVTable))
                continue;

            if(objectDataInterfacesLength <= objectInterface->ImplementingComponentsCount)
            {
                size_t newObjectDataInterfacesLength = objectDataInterfacesLength * 2;
                if(result = ObjectDataStackPush(NULL, (newObjectDataInterfacesLength - objectDataInterfacesLength) * sizeof(*objectInterface->ImplementingComponents))) return result;

                objectDataInterfacesLength = newObjectDataInterfacesLength;
            }

            ObjectComponentInterface objectComponentInterface;
            objectComponentInterface.Component = objectComponent;
            objectComponentInterface.VTable = componentVTable;

            objectInterface->ImplementingComponents[objectInterface->ImplementingComponentsCount] = objectComponentInterface;
            objectInterface->ImplementingComponentsCount++;
        }

        ObjectDataStackPop((objectInterfaceImplementingComponentsLength - objectInterface->ImplementingComponentsCount) * sizeof(*objectInterface->ImplementingComponents));
    }

    for(size_t x = 0; x < componentCount; x++)
    {
        ObjectComponent *objectComponent = objectData.Components + x;

        for(size_t y = 0; y < objectComponent->Component->UsesCount; y++)
        {
            Interface *useInterface = *(objectComponent->Component->Uses[y]);
            ObjectComponentUse objectComponentUse;

            ObjectInterface *foundInterface = NULL;
            for(size_t z = 0; z < objectData.InterfacesCount; z++)
            {
                if(objectData.Interfaces[z].Interface != useInterface)
                    continue;

                foundInterface = objectData.Interfaces + z;
            }

            if(foundInterface != NULL)
            {
                objectComponentUse.ImplementsCount = foundInterface->ImplementingComponentsCount
            }
            else
            {

            }
        }
    }

    ObjectData *stackObjectData;
    if(result = ObjectDataStackPush((void **)&stackObjectData, sizeof(objectData))) return result;
    *stackObjectData = objectData;
    *objectDataDest = stackObjectData;

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