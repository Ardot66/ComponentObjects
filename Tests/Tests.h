#ifndef __TESTS__
#define __TESTS__

#include "ComponentObjects.h"

INTERFACE_DECLARE(Shape, 
    USES_DECLARE(Shape, Shape),
    size_t (*GetArea)(void *object, const ObjectComponentData *componentData);
)

COMPONENT_DECLARE(MultiShape, ,
    USES_DECLARE(MultiShape, Shape)
)

COMPONENT_DECLARE(Rectangle, 
    COMPONENT_IMPLEMENTS_DECLARE(Rectangle, Shape), 
    USES_DECLARE(Rectangle, Shape), 
    size_t X;
    size_t Y;
)

COMPONENT_DECLARE(Trapezoid, 
    COMPONENT_IMPLEMENTS_DECLARE(Trapezoid, Shape), 
    USES_DECLARE(Trapezoid, Rectangle),
    size_t Height;
    size_t TopLength;
    size_t BottomLength;
)


#endif