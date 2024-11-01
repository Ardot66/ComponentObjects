#include "ComponentObjects.h"
#include <stdlib.h>

int ComponentInterfaceComparer(const void *a, const void *b)
{
    uintptr_t aInt = (uintptr_t)((ComponentInterface *)a)->Interface;
    uintptr_t bInt = (uintptr_t)((ComponentInterface *)b)->Interface;

    int comp = aInt > bInt;
    return (aInt != bInt) * (comp + !comp * -1);
}

void ArraySort(void *array, const size_t elementSize, const size_t length, int (*comparer)(const void *a, const void *b))
{
    for(size_t x = 0; x < length - 1; x++)
    {
        char *element = ((char *)array)[x * elementSize];
        char *bestFit = element;

        for(size_t y = x + 1; y < length; y++)
        {
            char *currentElement = ((char*)array)[y * elementSize];
            int comparison = comparer(bestFit, currentElement);

            if(comparison > 0)
                bestFit = currentElement;
        }

        if(bestFit == element)
            continue;

        for(size_t y = 0; y < elementSize; y++)
        {
            char temp;
            temp = element[y];
            element[y] = bestFit[y];  
            bestFit[y] = temp;          
        }
    }
}

int BinarySearch(const void *array, const size_t elemSize, const size_t length, const void *target, int (*comparer)(const void *a, const void *b), size_t *indexDest)
{
    const char *charArray = (char*)array;

    size_t position = length / 2;
    size_t step = length / 2;

    int compareResult;

    while (((compareResult = comparer(target, (void *)(charArray + (position * elemSize)))) != 0) & (step > 0))
    {
        position += (step / 2 + (step / 2 == 0)) * compareResult;
        step -= step / 2 + (step == 1);
    }
    
    *indexDest = position;
    return compareResult == 0;
}

void InitializeComponent(Component *component)
{
    size_t componentImplementsCount = 0;
    while(component->Implements[componentImplementsCount].Interface != NULL)
        componentImplementsCount++;

    ArraySort(component->Implements, sizeof(component->Implements), componentImplementsCount, ComponentInterfaceComparer);
    component->ImplementsCount = componentImplementsCount;
}

