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

  int Windowing::realScreenWidth;
  int Windowing::realScreenHeight;

  int Windowing::getScreenWidth() {
    return realScreenWidth;
  }

  int Windowing::getScreenHeight() {
    return realScreenHeight;
  }

  void Windowing::swapBuffers()
  {
    glfwSwapBuffers(window);
  }

  void Windowing::terminate()
  {
    //This was causing crashes on MacOS
    //glfwTerminate();
  }

#ifdef _WIN32
  HWND Windowing::getWin32Window()
  {
    return glfwGetWin32Window(window);
  }
#endif

  void Windowing::framebufferSizeCallback(GLFWwindow* window, int width,
    int height) {
    realScreenWidth = width;
    realScreenHeight = height;

    glViewport(0, 0, static_cast<GLsizei>(realScreenWidth),
      static_cast<GLsizei>(realScreenHeight));

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
    }

    window = glfwCreateWindow(width, height, windowTitle.c_str(), monitor,
      nullptr);

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
