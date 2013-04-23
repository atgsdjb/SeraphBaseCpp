.PHONY : all
CPPLDFLG = -lstdc++
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
%.o : %.cpp
	g++ $< -c -o $@
all : t10
t10 : $(OBJS)
	gcc $^ $(CPPLDFLG)  -o $@
.PHONY : clean
clean :
	@echo "clean-----"
	rm *.o
