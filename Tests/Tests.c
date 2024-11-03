#include "ComponentObjects.h"
#include "Tests.h"
#include <stdio.h>

#define TEST(expression, onFail, ...) \
{\
    const char *expressionString = #expression; \
    TestsCount++; \
    if(expression) \
    {\
        printf("Test Passed: %s\n", expressionString); \
        TestsPassed ++;\
    } \
    else\
        printf("Test Failed: %s; " onFail " at %s line %d\n", expressionString, ## __VA_ARGS__, __FILE__, __LINE__);\
}

#define TESTEND printf("%llu out of %llu tests passed", TestsPassed, TestsCount)

size_t TestsCount = 0;
size_t TestsPassed = 0;

INTERFACE_DEFINE(Shape)

size_t Rectangle_GetArea(void *self)
{
    Rectangle *rectangle = self;

    return rectangle->X * rectangle->Y;
}

COMPONENT_DEFINE(Rectangle, 
    IMPLEMENT(Shape, .GetArea = Rectangle_GetArea)
)

size_t Trapezoid_GetArea(void *self)
{
    Trapezoid *trapezoid = self;

    return ((trapezoid->BottomLength + trapezoid->TopLength) * trapezoid->Height) / 2;
}

COMPONENT_DEFINE(Trapezoid,
    IMPLEMENT(Shape, .GetArea = Trapezoid_GetArea)
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
    
    shapes[0] = (struct ShapePtr){.Shape = &rectangle, .VTable = INTERFACE_GET(Rectangle, Shape)};
    shapes[1] = (struct ShapePtr){.Shape = &trapezoid, .VTable = INTERFACE_GET(Trapezoid, Shape)};

    for(size_t x = 0; x < shapesLength; x++)
    {
        void *shape = shapes[x].Shape;
        size_t area = shapes[x].VTable->GetArea(shape);

        if(x == 0)
            TEST(area == rectangle.X * rectangle.Y, "")
        else if(x == 1)
            TEST(area == ((trapezoid.BottomLength + trapezoid.TopLength) * trapezoid.Height) / 2, "")
    }
}

void TestObjects()
{
    const size_t componentCount = 2;
    const Component **objectComponents = COMPONENTS(TYPEOF(Rectangle), TYPEOF(Trapezoid));
    
    char objectData[ObjectGetSize(2, objectComponents)];
    void *object = (void *)objectData;

    ObjectInititalize(object, componentCount, objectComponents);

    Rectangle *rectangle;
    Trapezoid *trapezoid;
    
    FOR_EACH_COMPONENT(component, object)
    {
        if(COMPONENT_DATA(component)->Component == TYPEOF(Rectangle))
        {
            rectangle = component;
            rectangle->X = 10;
            rectangle->Y = 5;
        }
        else if(COMPONENT_DATA(component)->Component == TYPEOF(Trapezoid))
        {
            trapezoid = component;
            trapezoid->BottomLength = 5;
            trapezoid->TopLength = 3;
            trapezoid->Height = 2;
        }
    }

    TEST(ComponentGetObject(rectangle) == object, "ComponentGetObject is %p, but should be %p", ComponentGetObject(rectangle), object)

    size_t totalArea = 0;

    FOR_EACH_COMPONENT(component, object)
    {
        Shape *shapeVTable;
        if(ComponentCast(COMPONENT_DATA(component)->Component, TYPEOF(Shape), (void **)&shapeVTable))
            continue;

        totalArea += shapeVTable->GetArea(component);
    }

    TEST(totalArea == Rectangle_GetArea(rectangle) + Trapezoid_GetArea(trapezoid), "")
}

int main (int argCount, char **argValues)
{
    INTERFACE_GET(Rectangle, Shape);

    

    TestComponents();
    TestObjects();

    TESTEND;
}