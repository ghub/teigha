CXX=g++ -g
CXXFLAGS= \
	  -Wall \
	  -Wno-comment \
	  -Wno-reorder \

INC = \
	-I. \
	-I../../OpenGL-tutorial_v0014_33/external/glew-1.9.0/include \
	-I../../OpenGL-tutorial_v0014_33/external/glfw-3.0.3/include/GLFW \
	-I../../Teigha_lnxX64_4.7dll/Core/Include \
	-I../../Teigha_lnxX64_4.7dll/Core/Extensions/ExServices \

LIB_DIR = \
	-L../../OpenGL-tutorial_v0014_33/build/external/glfw-3.0.3/src \
	-L../../OpenGL-tutorial_v0014_33/build/external \
	-L../../Teigha_lnxX64_4.7dll/bin/lnxX64_4.7dll \
	-L../../Teigha_lnxX64_4.7dll/lib/lnxX64_4.7dll \

LIB = \
	-lGL \
	-lGLEW_190 \
	-lglfw3 \
	-lpthread \
	-lX11 \
	-lXext \
	-lXi \
	-lXrandr \
	-lXxf86vm \
	\
	-lsisl \
	-lTD_Alloc \
	-lTD_Db \
	-lTD_DbRoot \
	-lTD_ExamplesCommon \
	-lTD_Ge \
	-lTD_Gi \
	-lTD_Key \
	-lTD_Root \
	-lTD_SpatialIndex \

OBJ = shader.o teigha.o

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(INC)

.PHONY: all
all: teigha

teigha: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIB_DIR) $(LIB)

.PHONY: test
test: all
	LD_LIBRARY_PATH=../../Teigha_lnxX64_4.7dll/bin/lnxX64_4.7dll \
		./teigha ~/Downloads/DWG/civil_example-imperial.dwg

.PHONY: clean
clean:
	rm -f $(OBJ) teigha
