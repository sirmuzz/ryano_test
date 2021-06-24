# FSG-Annunciator
# Last updated: 2021-Feb-12 
# 

NAME := ryano-test

CXXFLAGS = -g -Wall -Wextra -Wno-unused-variable -std=c++11 -DENABLE_STRING_LITERALS
OBJECTFLAGS = -c -fmessage-length=0 -fPIC -MMD -MP

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(addprefix obj/,$(notdir $(SOURCES:.cpp=.o)))

INCLUDES += -I./submodules/http 
LIB += -ldl 

LD_FLAGS += $(LIB)
LD_DEBUG = libs

TARGET = $(NAME)
all: $(SOURCES) $(NAME)

$(NAME): $(OBJECTS)
	@echo ' '
	@echo '### RYANO BUILD START ###'
	@echo 'Linking target: $@'
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJECTS) $(LIB)
	@echo 'TARGET BUILT: $@'
	@echo '### RYANO BUILD END ###'
	@echo ' '

obj/main.o: main.cpp
	@mkdir -p obj
	@echo 'Compiling target: $<'
	$(CXX) $(CXX_FLAGS) $(CXXFLAGS) $(TARGET_CFLAGS) $(OBJECTFLAGS) $(INCLUDES) -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<

obj/httpAccessPoint.o: httpAccessPoint.cpp
	@mkdir -p obj
	@echo 'Compiling target: $<'
	$(CXX) $(CXX_FLAGS) $(CXXFLAGS) $(TARGET_CFLAGS) $(OBJECTFLAGS) $(INCLUDES) -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<

clean:
	rm obj/* $(NAME)
