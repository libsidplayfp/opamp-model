.PHONY: all clean

# Uncomment to enable parallel processing
#FLAG_OPENMP = -fopenmp

SRC       = $(wildcard *.cpp)
EXE       = $(subst .cpp,,$(SRC))

CXXFLAGS = -march=native -O3

all: $(EXE)

clean:
	$(RM) $(EXE)

%: %.cpp
	$(CXX) $(CXXFLAGS) $(FLAG_OPENMP) -std=c++17 $< -o $@
