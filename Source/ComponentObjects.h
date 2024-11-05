#ifndef __COMPONENT_OBJECTS__
#define __COMPONENT_OBJECTS__

#include <stddef.h>

#define TYPE_VAR(name) T_ ## name
#define COMPONENT_USES(component, dependentComponent) COMPONENT_USES_ ## component ## _ ## dependentComponent
#define COMPONENT_USES_ENUM(component) ED_ ## component
#define COMPONENT_INTERFACE(name, interface) COMPONENT_INTERFACE_ ## name ## _ ## interface
#define COMPONENT_INTERFACES_ENUM(name) EI_ ## name

#define COMPONENT_USES_DECLARE(component, usesComponent) COMPONENT_USES(component, usesComponent),
#define COMPONENT_USES_DEFINE(dependentComponent) TYPEOF(dependentComponent),

#define COMPONENT_IMPLEMENTS_DECLARE(name, interface) COMPONENT_INTERFACE(name, interface),
#define COMPONENT_IMPLEMENTS_DEFINE(interface, ...) (const Interface **)(&(const interface){.Interface = &TYPE_VAR(interface), ## __VA_ARGS__}),
 
#define INTERFACE_DECLARE(name, ...) \
extern Interface TYPE_VAR(name);\
typedef struct name name;\
struct name {Interface *Interface; __VA_ARGS__};

#define INTERFACE_DEFINE(name)\
Interface TYPE_VAR(name);

#define COMPONENT_DECLARE(name, interfaces, uses, ...)\
extern Component TYPE_VAR(name);\
enum COMPONENT_INTERFACES_ENUM(name) {interfaces COMPONENT_INTERFACE(name, END)};\
enum COMPONENT_USES_ENUM(name) {uses COMPONENT_USES(name, END)};\
typedef struct name name;\
struct name {__VA_ARGS__};

#define COMPONENT_DEFINE(name, interfaces, uses)\
Component TYPE_VAR(name) = \
{\
    .Size = sizeof(name), .ImplementsCount = COMPONENT_INTERFACE(name, END), \
    .Implements = (const Interface**[COMPONENT_INTERFACE(name, END) + 1]){interfaces NULL}, \
    .UsesCount = COMPONENT_USES(name, END), \
    .Uses = (const Interface*[COMPONENT_USES(name, END) + 1]){uses NULL}\
};\

#define COMPONENT_GET_INTERFACE(component, interface) ((const interface *)TYPE_VAR(component).Implements[COMPONENT_INTERFACE(component, interface)])
#define TYPEOF(name) (&TYPE_VAR(name))

#define FOR_EACH_COMPONENT(componentVariableName, object) for(void *componentVariableName = (char *)object + sizeof(ComponentData); COMPONENT_DATA(componentVariableName)->Component != NULL; componentVariableName = (char *)componentVariableName + COMPONENT_DATA(componentVariableName)->Component->Size + sizeof(ComponentData))

#define COMPONENTS(count, ...) ((const Component*[]){__VA_ARGS__})
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
    const size_t UsesCount;
    const Interface **Uses;
};

typedef struct ObjectComponentUse ObjectComponentUse;
typedef struct ObjectComponent ObjectComponent;
typedef struct ObjectComponentInterface ObjectComponentInterface;
typedef struct ObjectInterface ObjectInterface;
typedef struct ObjectData ObjectData;

struct ObjectComponentUse
{
    size_t ImplementsCount;
    ObjectComponentInterface *ImplementingComponents;
};

struct ObjectComponent
{
    const Component *Component;
    size_t Offset;
    ObjectComponentUse Uses[];
};

struct ObjectComponentInterface
{
    ObjectComponent *Component;
    void *VTable;
};

struct ObjectInterface
{
    const Interface *Interface;
    size_t ImplementingComponentsCount;
    ObjectComponentInterface *ImplementingComponents;  
};

struct ObjectData
{
    size_t Size;
    size_t DataSize;
    size_t ComponentsCount;
    ObjectComponent **Components;
    size_t InterfacesCount;
    ObjectInterface *Interfaces;
};

int ObjectInitialize(ObjectData **objectDataDest, const size_t componentCount, const Component **components);
int ComponentCast(const Component *component, const Interface *interface, void **interfaceVTableDest);

#endif