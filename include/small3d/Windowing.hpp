/**
 * @file Windowing.hpp
 * @brief Windowing environment management class
 *
 * Created on: 2024/11/27
 *     Author: Dimitri Kourkoulis
 *    License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include <string>

namespace small3d {
  class Windowing {
  private:
    GLFWwindow* window = nullptr;
    static int realScreenWidth, realScreenHeight;

    static void framebufferSizeCallback(GLFWwindow* window, int width,
      int height);

  public:

    /**
     * @brief Initialise the application window
     * @param width Window width. Set to 0 for full screen and the full screen width will be returned here.
     * @param height Window height. Set to 0 for full screen and the full screen height will be returned here.
     * @param windowTitle The title of the window
     */
    void initWindow(int& width, int& height,
      const std::string& windowTitle = "");

    /**
     * @brief Get the real screen width
     * @return The screen width
     */
    int getScreenWidth();

    /**
     * @brief Get the real screen height
     * @return The screen height
     */
    int getScreenHeight();

    /**
     * @brief Swap buffers.
     */
    void swapBuffers();

    /**
     * @brief Perform any necessary cleanup actions.
     */
    void terminate();

#ifdef _WIN32
    HWND getWin32Window();
#endif
  };
}