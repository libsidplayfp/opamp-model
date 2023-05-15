.PHONY: all clean

# Uncomment to enable parallel processing
#FLAG_OPENMP = -fopenmp

CXXFLAGS = -march=native -O3

all: clean opamp

clean:
	$(RM) opamp

%: %.cpp
	$(CXX) $(CXXFLAGS) $(FLAG_OPENMP) -std=c++17 $< -o $@
