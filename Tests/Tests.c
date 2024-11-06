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

INTERFACE_DEFINE(Shape)

size_t Rectangle_GetArea(void *self)
{
    Rectangle *rectangle = self;

    return rectangle->X * rectangle->Y;
}

COMPONENT_DEFINE(Rectangle, 
    COMPONENT_IMPLEMENTS_DEFINE(Shape, .GetArea = Rectangle_GetArea),
    COMPONENT_USES_DEFINE(Shape)
)

size_t Trapezoid_GetArea(void *self)
{
    Trapezoid *trapezoid = self;

    return ((trapezoid->BottomLength + trapezoid->TopLength) * trapezoid->Height) / 2;
}

COMPONENT_DEFINE(Trapezoid,
    COMPONENT_IMPLEMENTS_DEFINE(Shape, .GetArea = Trapezoid_GetArea),
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
        void *shape = shapes[x].Shape;
        size_t area = shapes[x].VTable->GetArea(shape);

        if(x == 0)
            TEST(area, ==, rectangle.X * rectangle.Y, "%llu, %llu")
        else if(x == 1)
            TEST(area, ==, ((trapezoid.BottomLength + trapezoid.TopLength) * trapezoid.Height) / 2, "%llu, %llu")
    }
}

void TestObjects()
{
    int result;

    const size_t componentCount = 2;
    const Component **objectComponents = COMPONENTS(componentCount, TYPEOF(Rectangle), TYPEOF(Trapezoid));
    
    ObjectData *objectData;
    if(result = ObjectInitialize(&objectData, componentCount, objectComponents)) exit(result);

    char objectChars[objectData->Size];
    void *object = (void *)objectChars;

    Rectangle *rectangle;
    ObjectComponent *rectangleObjectData;

    Trapezoid *trapezoid;
    ObjectComponent *trapezoidObjectData;
    
    for(size_t x = 0; x < objectData->ComponentsCount; x++)
    {
        ObjectComponent *objectComponent = objectData->Components[x];
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
    TEST(rectangleObjectData->Uses[0].ImplementsCount, ==, 2, "%llu, %llu")
    TEST(rectangleObjectData->Uses[0].ImplementingComponents->Component->Component, ==, TYPEOF(Rectangle), "%p, %p")


    ObjectInterface *objectShapeInterface = ObjectGetInterface(objectData, TYPEOF(Shape));
    TEST(objectShapeInterface, !=, NULL, "%p, %p", return;)

    size_t totalArea = 0;

    for(size_t x = 0; x < objectShapeInterface->ImplementingComponentsCount; x++)
    {
        ObjectComponentInterface *objectComponent = objectShapeInterface->ImplementingComponents + x;
        Shape *vTable = objectComponent->VTable;

        if(objectComponent->Component->Component == TYPEOF(Rectangle))
            TEST(vTable->GetArea, ==, Rectangle_GetArea, "%p, %p", continue;)
        else if(objectComponent->Component->Component == TYPEOF(Trapezoid))
            TEST(vTable->GetArea, ==, Trapezoid_GetArea, "%p, %p", continue;)

        totalArea += vTable->GetArea((char *)object + objectComponent->Component->Offset);
    }

    TEST(totalArea, ==, Rectangle_GetArea(rectangle) + Trapezoid_GetArea(trapezoid), "%llu, %llu")

    free(objectData);
}

int main (int argCount, char **argValues)
{
    TestComponents();
    TestObjects();

    TESTEND;
}