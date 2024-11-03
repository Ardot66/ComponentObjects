#ifndef __COMPONENT_OBJECTS__
#define __COMPONENT_OBJECTS__

#include <stddef.h>

#define COMPONENT_VTABLE(name) V_ ## name
#define TYPE_VAR(name) T_ ## name

#define INTERFACE(interface) interface *interface;
#define IMPLEMENT(interface, ...) . interface = &(interface){.Interface = &TYPE_VAR(interface), ## __VA_ARGS__}

#define INTERFACE_DECLARE(name, ...) \
extern Interface TYPE_VAR(name);\
typedef struct name name;\
struct name {Interface * Interface; __VA_ARGS__};

#define INTERFACE_DEFINE(name)\
Interface TYPE_VAR(name);

#define COMPONENT_DECLARE(name, interfaces, ...)\
extern Component TYPE_VAR(name);\
typedef struct COMPONENT_VTABLE(name) COMPONENT_VTABLE(name);\
struct COMPONENT_VTABLE(name) {interfaces}; \
typedef struct name name;\
struct name {__VA_ARGS__};

#define COMPONENT_DEFINE(name, ...)\
Component TYPE_VAR(name) = {.Size = sizeof(name), .ImplementsCount = sizeof(COMPONENT_VTABLE(name)) / sizeof(void *), .VTable = &(COMPONENT_VTABLE(name)){__VA_ARGS__}};

#define VTABLE(name) ((COMPONENT_VTABLE(name) *)TYPE_VAR(name).VTable)
#define TYPEOF(name) (&TYPE_VAR(name))

#define FOR_EACH_COMPONENT(componentVariableName, object) for(void *componentVariableName = (char *)object + sizeof(ComponentData); COMPONENT_DATA(componentVariableName)->Component != NULL; componentVariableName = (char *)componentVariableName + COMPONENT_DATA(componentVariableName)->Component->Size + sizeof(ComponentData))

#define COMPONENTS(...) ((const Component*[]){__VA_ARGS__})
#define COMPONENT_DATA(component) ((ComponentData *)((char *)component - sizeof(ComponentData)))

typedef struct Interface Interface;
struct Interface
{
    const size_t Filler;
};

typedef struct Component Component;
struct Component
{
    const size_t Size;
    const size_t ImplementsCount;

    union
    {
        const Interface ***Implements;
        const void *VTable;
    };
};

typedef struct ComponentData ComponentData;
struct ComponentData
{
    const Component *Component;
};

typedef struct ObjectEnd ObjectEnd;
struct ObjectEnd
{
    void *NullPointer;
    void *Object;
};

size_t ObjectGetSize(const size_t componentCount, const Component **components);
void ObjectInititalize(void *object, const size_t componentCount, const Component **components);
int ComponentCast(const Component *component, const Interface *interface, void **interfaceVTableDest);
void *ComponentGetObject(const void *component);

#endif