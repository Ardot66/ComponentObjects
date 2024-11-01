#ifndef __COMPONENT_OBJECTS__
#define __COMPONENT_OBJECTS__

#include <stddef.h>

#define INTERFACE_VAR(name) I_ ## name
#define COMPONENT_VAR(name) C_ ## name

#define IMPLEMENT(interface, ...) &(interface){.InterfaceData = {.Interface = &INTERFACE_VAR(interface), .Component = NULL}, ## __VA_ARGS__},

#define INTERFACE_DECLARE(name, ...) \
extern Interface INTERFACE_VAR(name);\
typedef struct name name;\
struct name {ComponentInterface InterfaceData; __VA_ARGS__};

#define INTERFACE_DEFINE(name)\
Interface INTERFACE_VAR(name);

#define COMPONENT_DECLARE(name, ...)\
extern Component COMPONENT_VAR(name);\
typedef struct name name;\
struct name {__VA_ARGS__};

#define COMPONENT_DEFINE(name, ...)\
Component COMPONENT_VAR(name) = {.Size = sizeof(name), .ImplementsCount = 0, .Implements = (ComponentInterface[]){__VA_ARGS__ {0}}};

typedef struct Interface Interface;
struct Interface
{
    const size_t Filler;
};

typedef struct ComponentInterface ComponentInterface;
struct ComponentInterface
{
    const Interface *Interface;
    const Component *Component;
};

typedef struct Component Component;
struct Component
{
    const size_t Size;
    size_t ImplementsCount;
    ComponentInterface *Implements;
};

struct Object
{
    size_t Size;
};

#endif