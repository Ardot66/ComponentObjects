#ifndef __COMPONENT_OBJECTS__
#define __COMPONENT_OBJECTS__

#include <stddef.h>

#define COMPONENT_OBJECTS_TYPE_VAR(name) T_ ## name
#define COMPONENT_USES(component, dependentComponent) COMPONENT_USES_ ## component ## _ ## dependentComponent
#define COMPONENT_USES_ENUM(component) EU_ ## component
#define COMPONENT_INTERFACE(name, interface) COMPONENT_INTERFACE_ ## name ## _ ## interface
#define COMPONENT_INTERFACES_ENUM(name) EI_ ## name 
#define INTERFACE_USES(interface, usesInterface) COMPONENT_USES_ ## interface ## _ ## usesInterface
#define INTERFACE_USES_ENUM(interface) IU_ ## interface
 
#define COMPONENT_USES_DECLARE(component, usesComponent) COMPONENT_USES(component, usesComponent),
#define COMPONENT_USES_DEFINE(dependentComponent) TYPEOF(dependentComponent),

#define COMPONENT_IMPLEMENTS_DECLARE(name, interface) COMPONENT_INTERFACE(name, interface),
#define COMPONENT_IMPLEMENTS_DEFINE(interface, ...) (const InterfaceData **)(&(const interface){.InterfaceData = &COMPONENT_OBJECTS_TYPE_VAR(interface), ## __VA_ARGS__}),
 
#define INTERFACE_USES_DECLARE(interface, usesInterfaces) INTERFACE_USES(interface, usesInterface),
#define INTERFACE_USES_DEFINE(usesInterface) TYPEOF(usesInterface),

#define INTERFACE_DECLARE(name, uses, ...) \
extern InterfaceData COMPONENT_OBJECTS_TYPE_VAR(name);\
enum INTERFACE_USES_ENUM(name) {uses INTERFACE_USES(name, END)};\
typedef struct name name;\
struct name {InterfaceData *InterfaceData; __VA_ARGS__};

#define INTERFACE_DEFINE(name, uses)\
InterfaceData COMPONENT_OBJECTS_TYPE_VAR(name) = {.UsesCount = INTERFACE_USES(name, END), .Uses = (const InterfaceData *[INTERFACE_USES(name, END) + 1]){uses NULL}};

#define COMPONENT_DECLARE(name, interfaces, uses, ...)\
extern ComponentData COMPONENT_OBJECTS_TYPE_VAR(name);\
enum COMPONENT_INTERFACES_ENUM(name) {interfaces COMPONENT_INTERFACE(name, END)};\
enum COMPONENT_USES_ENUM(name) {uses COMPONENT_USES(name, END)};\
typedef struct name name;\
struct name {__VA_ARGS__};

#define COMPONENT_DEFINE(name, interfaces, uses)\
ComponentData COMPONENT_OBJECTS_TYPE_VAR(name) = \
{\
    .Size = sizeof(name), .ImplementsCount = COMPONENT_INTERFACE(name, END), \
    .Implements = (const InterfaceData**[COMPONENT_INTERFACE(name, END) + 1]){interfaces NULL}, \
    .UsesCount = COMPONENT_USES(name, END), \
    .Uses = (const InterfaceData*[COMPONENT_USES(name, END) + 1]){uses NULL}\
};\

#define POINTER_OFFSET(pointer, offset) ((void *)((char *)(pointer) + (uintptr_t)(offset)))
#define COMPONENT_GET_INTERFACE(component, interface) ((const interface *)COMPONENT_OBJECTS_TYPE_VAR(component).Implements[COMPONENT_INTERFACE(component, interface)])
#define COMPONENT_GET_USE(componentData, component, use) (componentData->Uses + COMPONENT_USES(component, use))
#define TYPEOF(name) (&COMPONENT_OBJECTS_TYPE_VAR(name))

#define COMPONENTS(...) ((const ComponentData*[]){__VA_ARGS__})

typedef struct InterfaceData InterfaceData;
struct InterfaceData
{
    const size_t UsesCount;
    const InterfaceData **Uses;
};

typedef struct ComponentData ComponentData;
struct ComponentData
{
    const size_t Size;
    const size_t ImplementsCount;
    const InterfaceData ***Implements;
    const size_t UsesCount;
    const InterfaceData **Uses;
};

typedef struct ObjectInterfaceUseData ObjectInterfaceUseData;
typedef struct ObjectComponentData ObjectComponentData;
typedef struct ObjectInterfaceInstanceData ObjectInterfaceInstanceData;
typedef struct ObjectInterfaceData ObjectInterfaceData;
typedef struct ObjectData ObjectData;

struct ObjectInterfaceUseData
{
    size_t ImplementsCount;
    ObjectInterfaceInstanceData *ImplementingComponents;
};

struct ObjectComponentData
{
    const ComponentData *Component;
    const ObjectData *ObjectData;
    size_t Offset;
    ObjectInterfaceUseData Uses[];
};

struct ObjectInterfaceInstanceData
{
    ObjectInterfaceData *Interface;
    ObjectComponentData *Component;
    void *VTable;
};

struct ObjectInterfaceData
{
    const InterfaceData *Interface;
    size_t ImplementingComponentsCount;
    ObjectInterfaceInstanceData *ImplementingComponents;  
    ObjectInterfaceUseData *Uses;
};

struct ObjectData
{
    size_t Size;
    size_t DataSize;
    size_t ComponentsCount;
    ObjectComponentData **Components;
    size_t InterfacesCount;
    ObjectInterfaceData *Interfaces;
};

int ObjectInitialize(ObjectData **objectDataDest, const size_t componentCount, const ComponentData **components);
int ComponentCast(const ComponentData *componentData, const InterfaceData *interfaceData, void **interfaceDest);
ObjectInterfaceData *ObjectGetInterface(const ObjectData *objectData, const InterfaceData *interfaceData);
ObjectComponentData *ObjectGetComponent(const ObjectData *objectData, const ComponentData *componentData);

#endif