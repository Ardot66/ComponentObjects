#include "ComponentObjects.h"
#include <stdlib.h>

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
    ObjectData objectData;
    objectData.Size = 0;
    objectData.ComponentsCount = componentCount;
    objectData.InterfacesCount = 0;

    //Figuring out the total object size
    objectData.DataSize = sizeof(ObjectData) + sizeof(*objectData.Components) * componentCount;
    
    for(size_t x = 0; x < componentCount; x++)
        objectData.DataSize += sizeof(**objectData.Components) + sizeof(ObjectComponentUse) * components[x]->UsesCount;

    //Also finding all unique interfaces
    size_t objectInterfacesLength = 4;
    ObjectInterface *objectInterfaces = malloc(sizeof(*objectInterfaces) * objectInterfacesLength);

    if(objectInterfaces == NULL)
        return errno;

    for(size_t x = 0; x < componentCount; x++)
    {
        const Component *component = components[x];

        for(size_t y = 0; y < component->ImplementsCount; y++)
        {
            const Interface *interface = *(component->Implements[y]);

            int interfaceAlreadyFound = 0;

            for(size_t z = 0; z < objectData.InterfacesCount; z++)
                interfaceAlreadyFound |= objectInterfaces[z].Interface == interface;
            
            if(interfaceAlreadyFound)
                continue;

            if(objectInterfacesLength <= objectData.InterfacesCount)
            {
                size_t newObjectInterfacesLength = objectInterfacesLength * 2;
                ObjectInterface *temp = realloc(objectInterfaces, sizeof(*objectInterfaces) * (newObjectInterfacesLength - objectInterfacesLength));

                if(temp == NULL)
                {
                    free(objectInterfaces);
                    return errno;
                }

                objectInterfaces = temp;
                objectInterfacesLength = newObjectInterfacesLength;
            }

            ObjectInterface objectInterface;
            objectInterface.Interface = interface;

            objectInterfaces[objectData.InterfacesCount] = objectInterface;
            objectData.InterfacesCount++;

            objectData.DataSize += sizeof(*objectData.Interfaces);

            for(size_t z = 0; z < componentCount; z++)
            {
                const Component *interfaceCheckComponent = components[z];

                for(size_t w = 0; w < interfaceCheckComponent->ImplementsCount; w++)
                    if(*(interfaceCheckComponent->Implements[w]) == interface)
                        objectData.DataSize += sizeof(ObjectComponentInterface);
            }
        }
    }

    // Allocating actual ObjectData super-structure in heap

    ObjectData *stackObjectData = malloc(objectData.DataSize);
    
    if(stackObjectData == NULL)
    {
        free(objectInterfaces);
        return errno;
    }
    
    char *objectDataStack = (char *)stackObjectData + sizeof(*stackObjectData);

    objectData.Components = (ObjectComponent **)objectDataStack;
    objectDataStack += sizeof(*objectData.Components) * componentCount;

    //Past this point is just setting up connections between different metadata objects

    for(size_t x = 0; x < componentCount; x++)
    {
        ObjectComponent objectComponent;
        objectComponent.Component = components[x];
        objectComponent.ObjectData = stackObjectData;
        objectComponent.Offset = objectData.Size;

        objectData.Size += objectComponent.Component->Size;
        
        ObjectComponent *stackObjectComponent = (ObjectComponent *)objectDataStack;
        objectDataStack += sizeof(*stackObjectComponent) + sizeof(*stackObjectComponent->Uses) * objectComponent.Component->UsesCount;

        *stackObjectComponent = objectComponent;
        objectData.Components[x] = stackObjectComponent;
    }

    objectData.Interfaces = (ObjectInterface *)objectDataStack;

    for(size_t x = 0; x < objectData.InterfacesCount; x++)
    {
        ObjectInterface objectInterface = objectInterfaces[x];

        ObjectInterface *stackObjectInterface = (ObjectInterface *)objectDataStack;
        objectDataStack += sizeof(*stackObjectInterface);

        *stackObjectInterface = objectInterface;
    }

    for(size_t x = 0; x < objectData.InterfacesCount; x++)
    {
        ObjectInterface *objectInterface = objectData.Interfaces + x;
        objectInterface->ImplementingComponentsCount = 0;
        objectInterface->ImplementingComponents = (ObjectComponentInterface *)objectDataStack;

        for(size_t y = 0; y < componentCount; y++)
        {
            ObjectComponentInterface objectComponentInterface;
            objectComponentInterface.Component = objectData.Components[y];

            if(ComponentCast(objectComponentInterface.Component->Component, objectInterface->Interface, &objectComponentInterface.VTable))
                continue;

            objectInterface->ImplementingComponentsCount += 1;

            ObjectComponentInterface *stackObjectComponentInterface = (ObjectComponentInterface *)objectDataStack;
            objectDataStack += sizeof(*stackObjectComponentInterface);
            *stackObjectComponentInterface = objectComponentInterface;
        }
    }

    free(objectInterfaces);

    for(size_t x = 0; x < componentCount; x++)
    {
        ObjectComponent *objectComponent = objectData.Components[x];

        for(size_t y = 0; y < objectComponent->Component->UsesCount; y++)
        {
            const Interface *useInterface = objectComponent->Component->Uses[y];
            ObjectComponentUse objectComponentUse;

            ObjectInterface *foundInterface = NULL;

            for(size_t z = 0; z < objectData.InterfacesCount; z++)
                if(objectData.Interfaces[z].Interface == useInterface)
                {
                    foundInterface = objectData.Interfaces + z;
                    break;
                }

            if(foundInterface != NULL)
            {
                objectComponentUse.ImplementsCount = foundInterface->ImplementingComponentsCount;
                objectComponentUse.ImplementingComponents = foundInterface->ImplementingComponents;
            }
            else
            {
                objectComponentUse.ImplementsCount = 0;
                objectComponentUse.ImplementingComponents = NULL;
            }

            objectComponent->Uses[y] = objectComponentUse;
        }
    }
    
    *stackObjectData = objectData;
    *objectDataDest = stackObjectData;

    return 0;
}

ObjectInterface *ObjectGetInterface(const ObjectData *objectData, const Interface *interface)
{
    for(size_t x = 0; x < objectData->InterfacesCount; x++)
    {
        ObjectInterface *objectInterface = objectData->Interfaces + x;

        if(objectInterface->Interface == interface)
            return objectInterface;
    }

    return NULL;
}

ObjectComponent *ObjectGetComponent(const ObjectData *objectData, const Component *component)
{
    for(size_t x = 0; x < objectData->ComponentsCount; x++)
    {
        ObjectComponent *objectComponent = objectData->Components[x];

        if(objectComponent->Component == component)
            return objectComponent;
    }

    return NULL;
}