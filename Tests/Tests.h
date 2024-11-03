#ifndef __TESTS__
#define __TESTS__

#include "ComponentObjects.h"

INTERFACE_DECLARE(Shape, 
    size_t (*GetArea)(void *self);
)

COMPONENT_DECLARE(Rectangle, INTERFACE(Rectangle, Shape), 
    size_t X;
    size_t Y;
)

COMPONENT_DECLARE(Trapezoid, INTERFACE(Trapezoid, Shape), 
    size_t Height;
    size_t TopLength;
    size_t BottomLength;
)


#endif