EXEC := bin/IOSync_Linux												# Executable name
CXX := g++																# Compiler
CXXFLAGS := -g -std=c++11 -MMD											# Compiler-flags -Wall -Wunused-variable
CPP_FILES := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)				# Retrieve every CPP file.
OBJECTS = $(patsubst src/%.cpp,obj/linux/%.o,$(CPP_FILES))				# Object-files. OBJECTS := $(addprefix obj/linux/, $(CPP_FILES:.cpp=.o))
DEPENDS := ${OBJECTS:.o=.d}												# Substitutes ".o" with ".d"

${EXEC} : ${OBJECTS}													# Link-step
	${CXX} ${OBJECTS} linux_libs/quicksock.o -o ${EXEC}										# linux_libs/quicksock.o

obj/linux/%.o : src/%.cpp
	mkdir -p $(dir $@)
	${CXX} ${CXXFLAGS} -c -o $@ $^

-include ${DEPENDS}														# Copies 'd' files (If they exist).