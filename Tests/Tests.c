#include "ComponentObjects.h"
#include "Tests.h"
#include <stdio.h>
#include <stdlib.h>

#define TEST(a, comparer, b, pattern, ...) \
{\
    const char *expressionString = #a " " #comparer " " #b; \
    TestsCount++; \
    if((a) comparer (b)) \
    {\
        printf("Test Passed: %s\n", expressionString); \
        TestsPassed ++;\
    }\
    else\
    {\
        printf("Test Failed: %s; Values are " pattern "; at %s line %d\n", expressionString, a, b, __FILE__, __LINE__);\
        __VA_ARGS__\
    }\
}

#define TESTEND printf("%llu out of %llu tests passed\n", TestsPassed, TestsCount)

size_t TestsCount = 0;
size_t TestsPassed = 0;

INTERFACE_DEFINE(Shape,
    INTERFACE_USES_DEFINE(Shape)
)

size_t Rectangle_GetArea(void *object, const ObjectComponentData *componentData)
{
    Rectangle *rectangle = POINTER_OFFSET(object, componentData->Offset);
    return rectangle->X * rectangle->Y;
}

COMPONENT_DEFINE(Rectangle, 
    COMPONENT_IMPLEMENTS_DEFINE(Shape, .GetArea = Rectangle_GetArea),
)

size_t Trapezoid_GetArea(void *object, const ObjectComponentData *componentData)
{
    Trapezoid *trapezoid = POINTER_OFFSET(object, componentData->Offset);
    return ((trapezoid->BottomLength + trapezoid->TopLength) * trapezoid->Height) / 2;
}

COMPONENT_DEFINE(Trapezoid,
    COMPONENT_IMPLEMENTS_DEFINE(Shape, .GetArea = Trapezoid_GetArea),
)

size_t MultiShape_GetArea(void *object, const ObjectComponentData *componentData)
{
    const ObjectInterfaceUseData *shapeUses = COMPONENT_GET_USE(componentData, MultiShape, Shape);
    size_t area = 0;

    for(size_t x = 0; x < shapeUses->ImplementsCount; x++)
    {
        ObjectInterfaceInstanceData *implementingComponent = shapeUses->ImplementingComponents + x;
        Shape *shapeVTable = implementingComponent->VTable;

        if(implementingComponent->Component->Component == TYPEOF(Rectangle))
            TEST(shapeVTable->GetArea, ==, Rectangle_GetArea, "%p, %p", continue;)
        else if(implementingComponent->Component->Component == TYPEOF(Trapezoid))
            TEST(shapeVTable->GetArea, ==, Trapezoid_GetArea, "%p, %p", continue;)

        area += shapeVTable->GetArea(object, implementingComponent->Component);
    }

    return area;
}

COMPONENT_DEFINE(MultiShape, ,
    COMPONENT_USES_DEFINE(Shape)
)

void TestComponents()
{
    Rectangle rectangle;
    rectangle.X = 1;
    rectangle.Y = 2;

    Trapezoid trapezoid;
    trapezoid.BottomLength = 4;
    trapezoid.TopLength = 6;
    trapezoid.Height = 3;

    const size_t shapesLength = 2;
    struct ShapePtr {void *Shape; const Shape *VTable;};
    struct ShapePtr shapes[shapesLength];
    
    shapes[0] = (struct ShapePtr){.Shape = &rectangle, .VTable = COMPONENT_GET_INTERFACE(Rectangle, Shape)};
    shapes[1] = (struct ShapePtr){.Shape = &trapezoid, .VTable = COMPONENT_GET_INTERFACE(Trapezoid, Shape)};

    for(size_t x = 0; x < shapesLength; x++)
    {
        if(x == 0)
            TEST(shapes[x].VTable->GetArea, ==, Rectangle_GetArea, "%llu, %llu")
        else if(x == 1)
            TEST(shapes[x].VTable->GetArea, ==, Trapezoid_GetArea, "%llu, %llu")
    }
}

void TestObjects()
{
    int result;

    const size_t componentCount = 3;
    const ComponentData **objectComponents = COMPONENTS(TYPEOF(MultiShape), TYPEOF(Rectangle), TYPEOF(Trapezoid));
    
    ObjectData *objectData;
    if(result = ObjectInitialize(&objectData, componentCount, objectComponents)) exit(result);

    char objectChars[objectData->Size];
    void *object = (void *)objectChars;

    Rectangle *rectangle;
    ObjectComponentData *rectangleObjectData;

    Trapezoid *trapezoid;
    ObjectComponentData *trapezoidObjectData;
    
    for(size_t x = 0; x < objectData->ComponentsCount; x++)
    {
        ObjectComponentData *objectComponent = objectData->Components[x];
        void *component = (char *)object + objectComponent->Offset;

        if(objectComponent->Component == TYPEOF(Rectangle))
        {
            rectangleObjectData = objectComponent;
            rectangle = component;
            rectangle->X = 10;
            rectangle->Y = 5;
        }
        else if(objectComponent->Component == TYPEOF(Trapezoid))
        {
            trapezoidObjectData = objectComponent;
            trapezoid = component;
            trapezoid->BottomLength = 5;
            trapezoid->TopLength = 3;
            trapezoid->Height = 2;
        }
    }

    TEST(rectangleObjectData->Component, ==, TYPEOF(Rectangle), "%p, %p")
    TEST(rectangleObjectData->Offset, ==, 0, "%llu, %llu")

    ObjectComponentData *multiShapeComponentData = ObjectGetComponent(objectData, TYPEOF(MultiShape));
    TEST(multiShapeComponentData, !=, NULL, "%p, %p", return;)

    TEST(multiShapeComponentData->Uses[0].ImplementsCount, ==, 2, "%llu, %llu")
    TEST(multiShapeComponentData->Uses[0].ImplementingComponents->Component->Component, ==, TYPEOF(Rectangle), "%p, %p")

    size_t totalArea = MultiShape_GetArea(object, multiShapeComponentData);

    TEST(totalArea, ==, Rectangle_GetArea(object, rectangleObjectData) + Trapezoid_GetArea(object, trapezoidObjectData), "%llu, %llu")
    free(objectData);
}

int main (int argCount, char **argValues)
{
    TestComponents();
    TestObjects();

    TESTEND;
}