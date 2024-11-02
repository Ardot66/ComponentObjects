#include "ComponentObjects.h"

// int ComponentInterfaceComparer(const void *a, const void *b)
// {
//     uintptr_t aInt = (uintptr_t)((ComponentInterface *)a)->Interface;
//     uintptr_t bInt = (uintptr_t)((ComponentInterface *)b)->Interface;

//     int comp = aInt > bInt;
//     return (aInt != bInt) * (comp + !comp * -1);
// }

// void ArraySort(void *array, const size_t elementSize, const size_t length, int (*comparer)(const void *a, const void *b))
// {
//     for(size_t x = 0; x < length - 1; x++)
//     {
//         char *element = ((char *)array) + x * elementSize;
//         char *bestFit = element;

//         for(size_t y = x + 1; y < length; y++)
//         {
//             char *currentElement = ((char*)array) + y * elementSize;
//             int comparison = comparer(bestFit, currentElement);

//             if(comparison > 0)
//                 bestFit = currentElement;
//         }

//         if(bestFit == element)
//             continue;

//         for(size_t y = 0; y < elementSize; y++)
//         {
//             char temp;
//             temp = element[y];
//             element[y] = bestFit[y];  
//             bestFit[y] = temp;          
//         }
//     }
// }

// int BinarySearch(const void *array, const size_t elemSize, const size_t length, const void *target, int (*comparer)(const void *a, const void *b), size_t *indexDest)
// {
//     const char *charArray = (char*)array;

//     size_t position = length / 2;
//     size_t step = length / 2;

//     int compareResult;

//     while (((compareResult = comparer(target, (void *)(charArray + (position * elemSize)))) != 0) & (step > 0))
//     {
//         position += (step / 2 + (step / 2 == 0)) * compareResult;
//         step -= step / 2 + (step == 1);
//     }
    
//     *indexDest = position;
//     return compareResult == 0;
// }

// void InitializeComponent(Component *component)
// {
//     ArraySort(component->Implements, sizeof(component->Implements), component->ImplementsCount, ComponentInterfaceComparer);
// }

size_t ObjectGetSize(const size_t componentCount, const Component **components)
{
    size_t size = sizeof(Object) + sizeof(void *);

    for(size_t x = 0; x < componentCount; x++)
        size += components[x]->Size + sizeof(ComponentData);

    return size;
}

void ObjectInititalize(Object *object, const size_t componentCount, const Component **components)
{
    object->ComponentCount = componentCount;
    char *currentComponent = (char *)object + sizeof(Object);

    for(size_t x = 0; x < componentCount; x++)
    {
        ComponentData componentData;
        componentData.Component = components[x];
        componentData.Object = object;

        *(ComponentData *)currentComponent = componentData;
        currentComponent += componentData.Component->Size + sizeof(ComponentData);
    }

    *(void **)currentComponent = NULL;
}

// Create a macro that handles this more efficiently
int ObjectIterateComponents(const Object *object, void **component)
{
    if(*component == NULL)
    {
        if(object->ComponentCount == 0)
            return 1;

        *component = (char *)object + sizeof(Object) + sizeof(ComponentData);
        return 0;
    }

    ComponentData *componentData = COMPONENT_DATA(component);

    if(componentData == NULL)
        return 1;

    *component = (char *)*component + sizeof(ComponentData) + componentData->Component->Size;
    return 0;
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

int ObjectGetFirstComponentOfType(const Object *object, const Interface *interface, void **componentDest, void **interfaceVTableDest)
{
    for(void *component = NULL; !ObjectIterateComponents(object, &component);)
    {
        if(ComponentCast(COMPONENT_DATA(component)->Component, interface, interfaceVTableDest))
            continue;

        *componentDest = component;
        return 0;
    }

    return 1;
}