#ifndef __COMPONENT_OBJECTS__
#define __COMPONENT_OBJECTS__

#include <stddef.h>

#define TYPE_VAR(name) T_ ## name
#define COMPONENT_INTERFACES_ENUM(name) E_ ## name
#define COMPONENT_INTERFACE(name, interface) COMPONENT_INTERFACE_ ## name ## _ ## interface

#define INTERFACE(name, interface) COMPONENT_INTERFACE(name, interface),
#define IMPLEMENT(interface, ...) (const Interface **)(&(const interface){.Interface = &TYPE_VAR(interface), ## __VA_ARGS__}),

#define INTERFACE_DECLARE(name, ...) \
extern Interface TYPE_VAR(name);\
typedef struct name name;\
struct name {Interface *Interface; __VA_ARGS__};

#define INTERFACE_DEFINE(name)\
Interface TYPE_VAR(name);

#define COMPONENT_DECLARE(name, interfaces, ...)\
extern Component TYPE_VAR(name);\
enum COMPONENT_INTERFACES_ENUM(name) {interfaces COMPONENT_INTERFACE(name, END)};\
typedef struct name name;\
struct name {__VA_ARGS__};

#define COMPONENT_DEFINE(name, ...)\
Component TYPE_VAR(name) = {.Size = sizeof(name), .ImplementsCount = COMPONENT_INTERFACE(name, END), .Implements = (const Interface**[COMPONENT_INTERFACE(name, END) + 1]){__VA_ARGS__ NULL}};

#define INTERFACE_GET(component, interface) ((const interface *)TYPE_VAR(component).Implements[COMPONENT_INTERFACE(component, interface)])
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
    const Interface ***Implements;
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