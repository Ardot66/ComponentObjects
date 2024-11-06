BIN = $(BASEDIR)/Bin
SOURCE = Source
TEMP = $(BASEDIR)/Temp

COMPONENT_OBJECTS_NAME = ComponentObjects
COMPONENT_OBJECTS_DLL = $(BIN)/lib$(COMPONENT_OBJECTS_NAME).dll

All: $(COMPONENT_OBJECTS_DLL)
HEADERS := $(abspath $(wildcard */*.h))
	
$(COMPONENT_OBJECTS_DLL): $(HEADERS) $(SOURCE)/$(COMPONENT_OBJECTS_NAME).c
	gcc -c -fPIC $(SOURCE)/$(COMPONENT_OBJECTS_NAME).c -o $(TEMP)/$(COMPONENT_OBJECTS_NAME).o
	gcc -shared $(TEMP)/$(COMPONENT_OBJECTS_NAME).o -o $(COMPONENT_OBJECTS_DLL)