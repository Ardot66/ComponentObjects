#include "ComponentObjects.h"
#include <stdlib.h>
#include <stdio.h>

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

static void ObjectFindUses(const size_t componentsCount, ObjectComponentData **objectComponents, const size_t interfacesCount, const ObjectInterfaceData *objectInterfaces, const size_t usesCount, const UseData *uses, ObjectUseData *usesDest)
{
    for(size_t x = 0; x < usesCount; x++)
    {
        const UseData useData = uses[x];
        ObjectUseData objectUseData;

        switch(*useData.DataType)
        {
            case USE_TYPE_COMPONENT:
            {
                const ObjectComponentData *foundObjectComponentData = NULL;

                for(size_t y = 0; y < componentsCount; y++)
                {
                    if(objectComponents[y]->Component == useData.Component)
                    {
                        foundObjectComponentData = objectComponents[y];
                        break;
                    }
                }

                objectUseData.Component = foundObjectComponentData;
                break;
            }
            case USE_TYPE_INTERFACE:
            {
                const ObjectInterfaceData *foundObjectInterfaceData = NULL;

                for(size_t y = 0; y < interfacesCount; y++)
                    if(objectInterfaces[y].Interface == useData.Interface)
                    {
                        foundObjectInterfaceData = objectInterfaces + y;
                        break;
                    }

                if(foundObjectInterfaceData != NULL)
                {
                    objectUseData.ImplementsCount = foundObjectInterfaceData->ImplementingComponentsCount;
                    objectUseData.ImplementingComponents = foundObjectInterfaceData->ImplementingComponents;
                }
                else
                {
                    objectUseData.ImplementsCount = 0;
                    objectUseData.ImplementingComponents = NULL;
                }
                break;
            }
        }
        
        usesDest[x] = objectUseData;
    }
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
        objectData.DataSize += sizeof(**objectData.Components) + sizeof(ObjectUseData) * components[x]->UsesCount;

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

            objectData.DataSize += sizeof(*objectData.Interfaces) + sizeof(*objectData.Interfaces->Uses) * interfaceData->UsesCount;

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

    free(objectInterfaces);

    for(size_t x = 0; x < objectData.InterfacesCount; x++)
    {
        ObjectInterfaceData *objectInterfaceData = objectData.Interfaces + x;
        objectInterfaceData->ImplementingComponentsCount = 0;
        objectInterfaceData->ImplementingComponents = (ObjectInterfaceInstanceData *)objectDataStack;

        for(size_t y = 0; y < componentCount; y++)
        {
            ObjectInterfaceInstanceData objectInterfaceInstanceData;
            objectInterfaceInstanceData.Component = objectData.Components[y];
            objectInterfaceInstanceData.Interface = objectInterfaceData;

            if(ComponentCast(objectInterfaceInstanceData.Component->Component, objectInterfaceData->Interface, &objectInterfaceInstanceData.VTable))
                continue;

            objectInterfaceData->ImplementingComponentsCount += 1;

            ObjectInterfaceInstanceData *stackObjectInterfaceInstanceData = (ObjectInterfaceInstanceData *)objectDataStack;
            objectDataStack += sizeof(*stackObjectInterfaceInstanceData);
            *stackObjectInterfaceInstanceData = objectInterfaceInstanceData;
        }
    }

    for(size_t x = 0; x < objectData.InterfacesCount; x++)
    {
        ObjectInterfaceData *interface = objectData.Interfaces + x;
        ObjectUseData *interfaceUses = (ObjectUseData *)objectDataStack;

        ObjectFindUses(objectData.ComponentsCount, objectData.Components, objectData.InterfacesCount, objectData.Interfaces, interface->Interface->UsesCount, interface->Interface->Uses, interfaceUses);
        objectDataStack += sizeof(*interfaceUses) * interface->Interface->UsesCount;
        interface->Uses = interfaceUses;
    }

    for(size_t x = 0; x < componentCount; x++)
    {
        ObjectComponentData *objectComponentData = objectData.Components[x];
        ObjectFindUses(objectData.ComponentsCount, objectData.Components, objectData.InterfacesCount, objectData.Interfaces, objectComponentData->Component->UsesCount, objectComponentData->Component->Uses, objectComponentData->Uses);
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