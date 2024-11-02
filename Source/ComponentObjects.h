#ifndef __COMPONENT_OBJECTS__
#define __COMPONENT_OBJECTS__

#include <stddef.h>

#define INTERFACE_VAR(name) I_ ## name
#define COMPONENT_VTABLE(name) V_ ## name
#define COMPONENT_VAR(name) C_ ## name

#define INTERFACE(interface) interface *interface;
#define IMPLEMENT(interface, ...) . interface = &(interface){.Interface = &INTERFACE_VAR(interface), ## __VA_ARGS__}

#define INTERFACE_DECLARE(name, ...) \
extern Interface INTERFACE_VAR(name);\
typedef struct name name;\
struct name {Interface * Interface; __VA_ARGS__};

#define INTERFACE_DEFINE(name)\
Interface INTERFACE_VAR(name);

#define COMPONENT_DECLARE(name, interfaces, ...)\
extern Component COMPONENT_VAR(name);\
typedef struct COMPONENT_VTABLE(name) COMPONENT_VTABLE(name);\
struct COMPONENT_VTABLE(name) {interfaces}; \
typedef struct name name;\
struct name {__VA_ARGS__};

#define COMPONENT_DEFINE(name, ...)\
Component COMPONENT_VAR(name) = {.Size = sizeof(name), .ImplementsCount = sizeof(COMPONENT_VTABLE(name)) / sizeof(void *), .VTable = &(COMPONENT_VTABLE(name)){__VA_ARGS__}};

#define VTABLE(name) ((COMPONENT_VTABLE(name) *)COMPONENT_VAR(name).VTable)
#define TYPEOF(name) ((Component *)&COMPONENT_VAR(name))

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

typedef struct Object Object;
struct Object
{
    size_t ComponentCount;
};

typedef struct ComponentData ComponentData;
struct ComponentData
{
    const Object *Object;
    const Component *Component;
};

size_t ObjectGetSize(const size_t componentCount, const Component **components);
void ObjectInititalize(Object *object, const size_t componentCount, const Component **components);
int ObjectIterateComponents(const Object *object, void **component);
int ComponentCast(const Component *component, const Interface *interface, void **interfaceVTableDest);
int ObjectGetFirstComponentOfType(const Object *object, const Interface *interface, void **componentDest, void **interfaceVTableDest);

#endif