/*! \page tutorial %small3d Tutorial

This is a tutorial on creating a ball in Blender, and then writing a C++ program 
that loads it and moves it around on the screen, using %small3d. Alternatively 
you can skip the ball creation and use a model of your own, if you have one in 
a glTF .glb file. The source code for this tutorial can be found 
here: https://github.com/dimi309/ball

# Getting small3d and Blender

The first thing to do is to download %small3d from GitHub: https://github.com/dimi309/small3d​​
Just follow the instructions in the README file to build it. If you have or can 
find a glTF .glb file containing a model somewhere, you can use that for the 
tutorial. Just skip to section "CMake Setup". Otherwise you need to install 
Blender since this tutorial begins with creating a model of a ball: https://www.blender.org/

# Creating a 3D Model with Blender

When you start Blender, you see a cube, and probably a camera:

![blender-start](img/tutorial01.png)

Press a to select them. If they are selected already, pressing a will de-select 
them. Press it again in that case. Then x to delete everything. You will be 
asked to confirm the deletion:

![confirm-deletion](img/tutorial02.png)

Just press enter to do so. Then, from the menu, select Add > Mesh > UV Sphere:

![add-uv-sphere](img/tutorial03.png)

This will create, as it says, a sphere:

![uv-sphere](img/tutorial04.png)

With the sphere selected (use the a key if it is not), select 
Object > Shade Smooth from the menu:

![select-smooth-shading](img/tutorial05.png)

This is not important but it will make the sphere look better.
We now need to create the Wavefront file. From the menu at the top, select 
File > Export > glTF 2.0 (.glb/.gltf):

![export-gltf](img/tutorial06.png)

Save the exported file as ball.obj (name it and click on the Export OBJ button):

![save-model](img/tutorial07.png)

# CMake Setup

Let's proceed to make our first program. Note that, instead of writing all the 
CMake code below, we could have just created a Visual Studio project for 
example, adding the %small3d library and dependencies to it. But using CMake as 
described will truly make our demo cross-platform, being able to compile and run 
on Windows, MacOS or Linux.

Create a directory, called ball. Then create another directory within it, 
called resources and place ball.obj in it. Also add the following code to a 
CMakeFiles.txt within the ball directory:

```
cmake_minimum_required(VERSION 3.0.2)

project(ball)

file(COPY "resources" DESTINATION "${PROJECT_BINARY_DIR}/bin")
file(COPY "deps/shaders" DESTINATION "${PROJECT_BINARY_DIR}/bin/resources")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/deps/cmake)

if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/bin")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/bin")
endif(MSVC)

set(DEPS_PATH "${CMAKE_SOURCE_DIR}/deps")
set(CMAKE_PREFIX_PATH ${DEPS_PATH})

find_package(SMALL3D REQUIRED)

subdirs(src)
```

Create a directory called src within the ball directory and, inside it, another CMakeLists.txt file:

```
add_executable(ball main.cpp)

target_include_directories(ball PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_include_directories(ball PUBLIC ${SMALL3D_INCLUDE_DIRS})

target_link_libraries(ball PUBLIC ${SMALL3D_LIBRARIES})

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
  set_target_properties(ball PROPERTIES LINK_FLAGS "-framework \
		AudioUnit -framework AudioToolbox -framework CoreAudio -framework Cocoa \
		-framework IOKit -framework CoreVideo")
endif()

if(MSVC)
  set_target_properties(ball PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
    "${PROJECT_BINARY_DIR}/bin")
endif()
```

# The Code

Inside ball/src, create the main.cpp file:

```
int main(int argc, char **argv) {

 return 0;
}
```

Include %small3d's Renderer and SceneObject classes:

```
#include <small3d/Renderer.hpp>
#include <small3d/SceneObject.hpp>
```

Now we need the GLFW header files:

```
#include <GLFW/glfw3.h>
```

We also need to be using the %small3d namespace, so this goes under our include 
statements:

```
using namespace small3d;
```

We also need to write the logic that will be detecting key presses:

```
bool downkey, leftkey, rightkey, upkey, esckey;

void keyCallback(GLFWwindow* window, int key, int scancode, int action,
 int mods)
{
if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
     downkey = true;
if (key == GLFW_KEY_UP && action == GLFW_PRESS)
     upkey = true;
if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
     leftkey = true;
if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
     rightkey = true;
if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
     esckey = true;
if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
     downkey = false;
if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
     upkey = false;
if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
     leftkey = false;
if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
     rightkey = false;
if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
     esckey = false;

}
```

And finally, we go to the main program, and we create the renderer. The renderer 
is a singleton, so it can only be retrieved via the getInstance method, and 
assigned to a pointer:

```
Renderer *renderer = &Renderer::getInstance("Ball demo");
```

We will later need to access the window of the application, in order to pick up 
key events:

```
GLFWwindow* window = renderer->getWindow();
```

We create the ball:

```
SceneObject ball("ball", "resources/ball.glb", "");
```

%small3d uses vectors a lot as parameters for convenience. When positioning the 
ball, the components are in order, x (-left, +right), y(+up, -down), and z 
(-away from the camera, +towards the camera):

```
ball.position = glm::vec3(0.0f, -1.0f, -8.0f);
```

So let's start our main loop now. %small3d uses GLFW and you can use it too! First we need to declare the callback function, which will be the keyCallback
method we wrote above.

```
glfwSetKeyCallback(window, keyCallback);
```

Now in every iteration, we need to check whether we want to exit the program. 
Let's say that we'll be doing that with the Esc key:

```
while (!glfwWindowShouldClose(window) && !esckey) {

glfwPollEvents();
 if (esckey)
  break;
If after that we are still in the loop (so, no Esc key pressed), we will want to move the ball around with the keyboard.
We will have the up arrow move the ball away from the camera. Down will do the opposite. Left and right move the ball as implied.
if (upkey)
  ball.position.z -= 0.001f;
else if (downkey)
  ball.position.z += 0.001f;
else if (leftkey)
  ball.position.x -= 0.001f;
else if (rightkey)
  ball.position.x += 0.001f;
```

Ok, the ball is positioned. Now we need to actually render it. The second 
parameter is the colour. Let's say it's yellow (the vector below symbolises an 
rgb colour, together with the alpha channel):

```
renderer->render(ball, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
```

We are using a buffered system (we draw on one buffer, while the user is looking 
at the other one), so we also need to swap the buffers:

```
renderer->swapBuffers();
```

And we close the loop :)

```
}
```

That's it!

Let's try it out. Create a ball/deps directory and from the built %small3d 
library copy the cmake, build/include, build/lib and build/shaders directories 
to this deps directory. Then, back from the root ball directory execute:

```
mkdir build
cd build
cmake ..
cmake --build .
cd bin
./ball
```

On Windows, you need to execute cmake .. -G"MinGW Makefiles", or with the 
preferred Visual Studio configuration (e.g. cmake .. -G"Visual Studio 17 2022").

Note that you have to be inside the build/bin directory in order to execute the 
program, otherwise it will not find the necessary resource files (shaders, 
textures, etc).

There's our ball:

![ball](img/tutorial08.png)

Try moving it around with the arrows.
%small3d supports a lot more features than those presented here, playing sounds, 
collision detection and building with the conan build system which is a lot 
faster and easier than plain cmake. A good way to learn about all that is to 
read the documentation and also to explore the source code of the sample games.
