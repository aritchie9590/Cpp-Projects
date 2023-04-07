# Makefile

icosahedron:	icosahedron.o
	$(CXX) -g -o icosahedron icosahedron.cc -L/usr/lib64 -lglut -lGLU -lGL

icosahedron.o:	icosahedron.cc
	$(CXX) -c -g icosahedron.cc

clean:
	@rm -f icosahedron icosahedron.o
