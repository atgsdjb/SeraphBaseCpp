SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
CXXFLAGS := -lstdc++
.PHONY :all
all := base
base : $(OBJS)
	g++ $< $(CXXFLGS) -o $@
