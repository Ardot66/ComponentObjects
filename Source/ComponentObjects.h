#ifndef __COMPONENT_OBJECTS__
#define __COMPONENT_OBJECTS__

#include <stddef.h>

#define COMPONENT_OBJECTS_TYPE_VAR(name) T_ ## name
#define USES(component, dependentComponent) USES_ ## component ## _ ## dependentComponent
#define COMPONENT_USES_ENUM(component) EU_ ## component
#define COMPONENT_INTERFACE(name, interface) COMPONENT_INTERFACE_ ## name ## _ ## interface
#define COMPONENT_INTERFACES_ENUM(name) EI_ ## name 
#define INTERFACE_USES_ENUM(interface) IU_ ## interface
 
#define USES_DECLARE(this, uses) USES(this, uses),
#define USES_DEFINE(uses) {.Value = TYPEOF(uses)},

#define COMPONENT_IMPLEMENTS_DECLARE(name, interface) COMPONENT_INTERFACE(name, interface),
#define COMPONENT_IMPLEMENTS_DEFINE(interface, ...) (const InterfaceData **)(&(const interface){.InterfaceData = &COMPONENT_OBJECTS_TYPE_VAR(interface), ## __VA_ARGS__}),

#define INTERFACE_DECLARE(name, uses, ...) \
extern InterfaceData COMPONENT_OBJECTS_TYPE_VAR(name);\
enum INTERFACE_USES_ENUM(name) {uses USES(name, END)};\
typedef struct name name;\
struct name {InterfaceData *InterfaceData; __VA_ARGS__};

#define INTERFACE_DEFINE(name, uses)\
InterfaceData COMPONENT_OBJECTS_TYPE_VAR(name) = \
{\
    .DataType = USE_TYPE_INTERFACE,\
    .UsesCount = USES(name, END), \
    .Uses = (const UseData [USES(name, END) + 1]){uses {NULL}}\
};

#define COMPONENT_DECLARE(name, interfaces, uses, ...)\
extern ComponentData COMPONENT_OBJECTS_TYPE_VAR(name);\
enum COMPONENT_INTERFACES_ENUM(name) {interfaces COMPONENT_INTERFACE(name, END)};\
enum COMPONENT_USES_ENUM(name) {uses USES(name, END)};\
typedef struct name name;\
struct name {__VA_ARGS__};

#define COMPONENT_DEFINE(name, interfaces, uses)\
ComponentData COMPONENT_OBJECTS_TYPE_VAR(name) = \
{\
    .DataType = USE_TYPE_COMPONENT,\
    .Size = sizeof(name), .ImplementsCount = COMPONENT_INTERFACE(name, END), \
    .Implements = (const InterfaceData**[COMPONENT_INTERFACE(name, END) + 1]){interfaces NULL}, \
    .UsesCount = USES(name, END), \
    .Uses = (const UseData[USES(name, END) + 1]){uses {NULL}}\
};\

#define POINTER_OFFSET(pointer, offset) ((void *)((char *)(pointer) + (uintptr_t)(offset)))
#define COMPONENT_GET_INTERFACE(component, interface) ((const interface *)COMPONENT_OBJECTS_TYPE_VAR(component).Implements[COMPONENT_INTERFACE(component, interface)])
#define COMPONENT_GET_USE(componentData, component, use) (componentData->Uses + USES(component, use))
#define INTERFACE_GET_USE(interfaceData, interface, use) (interfaceData->Uses + USES(interface, use))
#define TYPEOF(name) (&COMPONENT_OBJECTS_TYPE_VAR(name))

#define COMPONENTS(...) ((const ComponentData*[]){__VA_ARGS__})

enum {USE_TYPE_COMPONENT, USE_TYPE_INTERFACE};

typedef struct InterfaceData InterfaceData;
typedef struct ComponentData ComponentData;
typedef struct UseData UseData;

struct UseData
{
    union
    {
        int *DataType;
        InterfaceData *Interface;
        ComponentData *Component;
        void *Value;
    };
};

struct InterfaceData
{
    const int DataType;
    const size_t UsesCount;
    const UseData *Uses;
};

struct ComponentData
{
    const int DataType;
    const size_t Size;
    const size_t ImplementsCount;
    const InterfaceData ***Implements;
    const size_t UsesCount;
    const UseData *Uses;
};

typedef struct ObjectUseData ObjectUseData;
typedef struct ObjectComponentData ObjectComponentData;
typedef struct ObjectInterfaceInstanceData ObjectInterfaceInstanceData;
typedef struct ObjectInterfaceData ObjectInterfaceData;
typedef struct ObjectData ObjectData;


struct ObjectUseData
{
    union
    {
        struct
        {
            size_t ImplementsCount;
            ObjectInterfaceInstanceData *ImplementingComponents;
        };

        const ObjectComponentData *Component;
    };
};

struct ObjectComponentData
{
    const ComponentData *Component;
    const ObjectData *ObjectData;
    size_t Offset;
    ObjectUseData Uses[];
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
    ObjectUseData *Uses;
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