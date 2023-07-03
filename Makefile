RUN_SCOUT_RUN = run --rm -it \
				-v $(shell pwd):/wkspace \
				--user $(shell id -u):$(shell id -g) \
				registry.gitlab.steamos.cloud/steamrt/scout/sdk:latest bash -c

COMPILE_FLAGS = -std=c99 -march=x86-64 -m64 -msse -msse2 -msse3 -Werror=vla
COMPILE_FLAGS += -I./ext/glfw/include
LINK_FLAGS = -Wl,-rpath=. -L.
LINUX_FLAGS = -pthread
LINUX_LIBS = ./obj/ext/glfw/src/libglfw3.a -lX11 -lXinerama -lXrandr -lvulkan -lm -ldl

debug: obj/ext/glfw/src/libglfw3.a
	@echo "Compiling in debug mode using GCC"
	@mkdir -p build/
	gcc $(COMPILE_FLAGS) $(LINUX_FLAGS) -g3 src/main.c -o ./build/vk_template $(LINUX_LIBS)

obj/ext/glfw/src/libglfw3.a: obj/ext/glfw/Makefile
	docker $(RUN_SCOUT_RUN) "cd /wkspace/obj/ext/glfw && make -j8 CC=gcc-9"

GLFW_CMAKE_OPTS = \
	-DGLFW_BUILD_TYPE=Release \
	-DGLFW_BUILD_DOCS=OFF \
	-DGLFW_BUILD_EXAMPLES=OFF \
	-DGLFW_BUILD_TESTS=OFF \
	-DGLFW_INSTALL=OFF

obj/ext/glfw/Makefile:
	@echo "Generating GLFW Makefile with CMake"
	@mkdir -p obj/ext/glfw
	docker $(RUN_SCOUT_RUN) "cd /wkspace && cmake $(GLFW_CMAKE_OPTS) -S ext/glfw/ -B obj/ext/glfw/"

clean:
	rm -rf obj/
	rm -rf build/

