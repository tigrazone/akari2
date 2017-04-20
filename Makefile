.DEFAULT_GOAL := all
all:
	g++ -Ilibs/ -o akari2  main.cpp bmpexporter.cpp -O4 -march=native -funroll-loops -mfpmath=sse -msse2 -msse3 -mssse3 -ffast-math -fpermissive  -Wno-format-y2k  -fno-strict-aliasing -s -fopenmp -std=c++0x  -static-libgcc -static-libstdc++ -static
