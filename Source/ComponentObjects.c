#include "ComponentObjects.h"
#include <stdlib.h>

int ComponentCast(const ComponentData *component, const InterfaceData *interfaceData, void **interfaceDest)
{
    for(size_t x = 0; x < component->ImplementsCount; x++)
    {
        if(*component->Implements[x] != interfaceData)
            continue;

        *interfaceDest = component->Implements[x];
        return 0;
    }
    
    return 1;
}

int ObjectInitialize(ObjectData **objectDataDest, const size_t componentCount, const ComponentData **components)
{
    ObjectData objectData;
    objectData.Size = 0;
    objectData.ComponentsCount = componentCount;
    objectData.InterfacesCount = 0;

    //Figuring out the total object size
    objectData.DataSize = sizeof(ObjectData) + sizeof(*objectData.Components) * componentCount;
    
    for(size_t x = 0; x < componentCount; x++)
        objectData.DataSize += sizeof(**objectData.Components) + sizeof(ObjectComponentUseData) * components[x]->UsesCount;

    //Also finding all unique interfaces
    size_t objectInterfacesLength = 4;
    ObjectInterfaceData *objectInterfaces = malloc(sizeof(*objectInterfaces) * objectInterfacesLength);

    if(objectInterfaces == NULL)
        return errno;

    for(size_t x = 0; x < componentCount; x++)
    {
        const ComponentData *componentData = components[x];

        for(size_t y = 0; y < componentData->ImplementsCount; y++)
        {
            const InterfaceData *interfaceData = *(componentData->Implements[y]);

            int interfaceAlreadyFound = 0;

            for(size_t z = 0; z < objectData.InterfacesCount; z++)
                interfaceAlreadyFound |= objectInterfaces[z].Interface == interfaceData;
            
            if(interfaceAlreadyFound)
                continue;

            if(objectInterfacesLength <= objectData.InterfacesCount)
            {
                size_t newObjectInterfacesLength = objectInterfacesLength * 2;
                ObjectInterfaceData *tempObjectInterfaces = realloc(objectInterfaces, sizeof(*objectInterfaces) * (newObjectInterfacesLength - objectInterfacesLength));

                if(tempObjectInterfaces == NULL)
                {
                    free(objectInterfaces);
                    return errno;
                }

                objectInterfaces = tempObjectInterfaces;
                objectInterfacesLength = newObjectInterfacesLength;
            }

            ObjectInterfaceData objectInterfaceData;
            objectInterfaceData.Interface = interfaceData;

            objectInterfaces[objectData.InterfacesCount] = objectInterfaceData;
            objectData.InterfacesCount++;

            objectData.DataSize += sizeof(*objectData.Interfaces);

            for(size_t z = 0; z < componentCount; z++)
            {
                const ComponentData *checkComponentData = components[z];

                for(size_t w = 0; w < checkComponentData->ImplementsCount; w++)
                    if(*(checkComponentData->Implements[w]) == interfaceData)
                        objectData.DataSize += sizeof(ObjectInterfaceInstanceData);
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

    objectData.Components = (ObjectComponentData **)objectDataStack;
    objectDataStack += sizeof(*objectData.Components) * componentCount;

    //Past this point is just setting up connections between different metadata objects

    for(size_t x = 0; x < componentCount; x++)
    {
        ObjectComponentData objectComponentData;
        objectComponentData.Component = components[x];
        objectComponentData.ObjectData = stackObjectData;
        objectComponentData.Offset = objectData.Size;

        objectData.Size += objectComponentData.Component->Size;
        
        ObjectComponentData *stackObjectComponentData = (ObjectComponentData *)objectDataStack;
        objectDataStack += sizeof(*stackObjectComponentData) + sizeof(*stackObjectComponentData->Uses) * objectComponentData.Component->UsesCount;

        *stackObjectComponentData = objectComponentData;
        objectData.Components[x] = stackObjectComponentData;
    }

    objectData.Interfaces = (ObjectInterfaceData *)objectDataStack;

    for(size_t x = 0; x < objectData.InterfacesCount; x++)
    {
        ObjectInterfaceData objectInterfaceData = objectInterfaces[x];

        ObjectInterfaceData *stackObjectInterfaceData = (ObjectInterfaceData *)objectDataStack;
        objectDataStack += sizeof(*stackObjectInterfaceData);

        *stackObjectInterfaceData = objectInterfaceData;
    }

    for(size_t x = 0; x < objectData.InterfacesCount; x++)
    {
        ObjectInterfaceData *objectInterfaceData = objectData.Interfaces + x;
        objectInterfaceData->ImplementingComponentsCount = 0;
        objectInterfaceData->ImplementingComponents = (ObjectInterfaceInstanceData *)objectDataStack;

        for(size_t y = 0; y < componentCount; y++)
        {
            ObjectInterfaceInstanceData objectInterfaceInstanceData;
            objectInterfaceInstanceData.Component = objectData.Components[y];

            if(ComponentCast(objectInterfaceInstanceData.Component->Component, objectInterfaceData->Interface, &objectInterfaceInstanceData.VTable))
                continue;

            objectInterfaceData->ImplementingComponentsCount += 1;

            ObjectInterfaceInstanceData *stackObjectInterfaceInstanceData = (ObjectInterfaceInstanceData *)objectDataStack;
            objectDataStack += sizeof(*stackObjectInterfaceInstanceData);
            *stackObjectInterfaceInstanceData = objectInterfaceInstanceData;
        }
    }

    free(objectInterfaces);

    for(size_t x = 0; x < componentCount; x++)
    {
        ObjectComponentData *objectComponentData = objectData.Components[x];

        for(size_t y = 0; y < objectComponentData->Component->UsesCount; y++)
        {
            const InterfaceData *useInterfaceData = objectComponentData->Component->Uses[y];
            ObjectComponentUseData objectComponentUseData;

            ObjectInterfaceData *foundObjectInterfaceData = NULL;

            for(size_t z = 0; z < objectData.InterfacesCount; z++)
                if(objectData.Interfaces[z].Interface == useInterfaceData)
                {
                    foundObjectInterfaceData = objectData.Interfaces + z;
                    break;
                }

            if(foundObjectInterfaceData != NULL)
            {
                objectComponentUseData.ImplementsCount = foundObjectInterfaceData->ImplementingComponentsCount;
                objectComponentUseData.ImplementingComponents = foundObjectInterfaceData->ImplementingComponents;
            }
            else
            {
                objectComponentUseData.ImplementsCount = 0;
                objectComponentUseData.ImplementingComponents = NULL;
            }

            objectComponentData->Uses[y] = objectComponentUseData;
        }
    }
    
    *stackObjectData = objectData;
    *objectDataDest = stackObjectData;

    return 0;
}

ObjectInterfaceData *ObjectGetInterface(const ObjectData *objectData, const InterfaceData *interface)
{
    for(size_t x = 0; x < objectData->InterfacesCount; x++)
    {
        ObjectInterfaceData *objectInterfaceData = objectData->Interfaces + x;

        if(objectInterfaceData->Interface == interface)
            return objectInterfaceData;
    }

    return NULL;
}

ObjectComponentData *ObjectGetComponent(const ObjectData *objectData, const ComponentData *component)
{
    for(size_t x = 0; x < objectData->ComponentsCount; x++)
    {
        ObjectComponentData *objectComponentData = objectData->Components[x];

        if(objectComponentData->Component == component)
            return objectComponentData;
    }

    return NULL;
}