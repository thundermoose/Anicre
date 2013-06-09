
CXXFLAGS = -Wall -Wconversion -O3 -lm -g

all: mfr



mfr: mfr_main.o mr_file_reader.o mr_base_reader.o
	$(CXX) $+ -o $@

clean:
	rm -rf *.o mfr