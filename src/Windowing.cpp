/*
 *  Windowing.cpp
 *
 *  Created on: 2024/11/27
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "Windowing.hpp"
#include "Logger.hpp"

namespace small3d {

  static void error_callback(int error, const char* description)
  {
    LOGERROR(std::string(description));
  }

  int Windowing::realWindowWidth;
  int Windowing::realWindowHeight;

  void Windowing::swapBuffers()
  {
    glfwSwapBuffers(window);
  }

  void Windowing::terminate()
  {
    //This was causing crashes on MacOS
    //glfwTerminate();
  }

  GLFWwindow* Windowing::getWindow()
  {
    return window;
  }

#ifdef _WIN32
  HWND Windowing::getWin32Window()
  {
    return glfwGetWin32Window(window);
  }
#endif

  void Windowing::framebufferSizeCallback(GLFWwindow* window, int width,
    int height) {
    realWindowWidth = width;
    realWindowHeight = height;

    glViewport(0, 0, static_cast<GLsizei>(realWindowWidth),
      static_cast<GLsizei>(realWindowHeight));

    LOGDEBUG("New framebuffer dimensions " + std::to_string(width) + " x " +
      std::to_string(height));

  }

  void Windowing::initWindow(int& width, int& height,
    const std::string& windowTitle) {

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
      throw std::runtime_error("Unable to initialise GLFW");
    }
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // This was used as a workaround for an issue I cannot remember
    // on Mojave but on Monterey it was making the window transparent
    // when the colour of a rendered mesh was transparent so I have
    // commented it out.
    // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWmonitor* monitor = nullptr; // If NOT null, a full-screen window will
    // be created.

    bool fullScreen = false;

    if ((width == 0 && height != 0) || (width != 0 && height == 0)) {
      throw std::runtime_error("Screen width and height both have to be equal "
        "or not equal to zero at the same time.");
    }
    else if (width == 0) {

      monitor = glfwGetPrimaryMonitor();

      const GLFWvidmode* mode = glfwGetVideoMode(monitor);
      width = mode->width;
      height = mode->height;

      LOGINFO("Detected screen width " + std::to_string(width) + " and height " +
        std::to_string(height));

      fullScreen = true;

#ifdef _WIN32
      // On some Windows machines, capturing the OpenGL back buffer can produce a skewed image when a game runs in
      // full screen mode. This "hack" ensures borderless window mode. It is used because the OS can "decide" to 
      // set a game to full screen mode even when it has not been launched as such. The slight screen width difference 
      // helps avoid that. Full screen mode can still be activated when a user manually maximises a game
      // window for a game running in windowed mode. But fortunately that will not be possible for a game that
      // has been launched in borderless window mode with this hack.
      width+=2;
#endif
    }

    window = glfwCreateWindow(width, height, windowTitle.c_str(), monitor,
      nullptr);
    
#ifdef _WIN32
    // Full screen hack continued (see above)
    if (fullScreen) {
      glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
      glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
    }
#endif
    
    realWindowWidth = width;
    realWindowHeight = height;
    

    if (!window) {
      throw std::runtime_error("Unable to create GLFW window");
    }

    if (fullScreen) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    glfwMakeContextCurrent(window);

    width = 0;
    height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    LOGINFO("Framebuffer width " + std::to_string(width) + " height " +
      std::to_string(height));

  }

}
