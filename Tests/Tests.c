#include "ComponentObjects.h"
#include <stdio.h>

INTERFACE_DECLARE(Shape, 
    void (*Function)(void *);
)

INTERFACE_DEFINE(Shape)

COMPONENT_DECLARE(Square,
    size_t Amogus;
)

void Test(void *_this)
{
    
}

COMPONENT_DEFINE(Square, IMPLEMENT(Shape, .Function = Test))

int main (int argCount, char **argValues)
{
    printf("%p, %p", ((Component *)Test), &Test);
}