.PHONY: all clean

# Uncomment to enable parallel processing
#FLAG_OPENMP = -fopenmp
FLAGS_GLS = `pkg-config cflags gsl`
LIBS_GLS = `pkg-config --libs gsl`

SRC       = $(wildcard *.cpp)
EXE       = $(subst .cpp,,$(SRC))

CXXFLAGS = -march=native -O3

all: $(EXE)

clean:
	$(RM) $(EXE)

%: %.cpp
	$(CXX) $(CXXFLAGS) $(FLAG_OPENMP) $(FLAGS_GLS) $(LIBS_GLS) -std=c++17 $< -o $@
