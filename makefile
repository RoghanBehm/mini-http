# compiler flags
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 -Iinclude
LDFLAGS := -lz

# dirs
SRCDIR := src
OBJDIR := build
TARGET := server

# Source and object files
SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

# default
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# compile each .cpp to .o in build/
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# create build dir
$(OBJDIR):
	mkdir -p $(OBJDIR)


#
clean:
	rm -rf $(OBJDIR) $(TARGET)
