/**
 * @file Windowing.hpp
 * @brief Window management class
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

  /**
   * @class	Windowing
   *
   * @brief	Window management class
   */
  class Windowing {
  private:
    GLFWwindow* window = nullptr;

    static void framebufferSizeCallback(GLFWwindow* window, int width,
      int height);

  public:

    /**
     * @brief Real window width. It is exposed as public for speed, but it
     *        should be treated as read-only.
     */
    static int realWindowWidth;

    /**
     * @brief Real window height. It is exposed as public for speed, but it
     *        should be treated as read-only.
     */
    static int realWindowHeight;

    /**
     * @brief Initialise the application window
     * @param width Window width. Set to 0 for full screen and the full screen width will be returned here.
     * @param height Window height. Set to 0 for full screen and the full screen height will be returned here.
     * @param windowTitle The title of the window
     */
    void initWindow(int& width, int& height,
      const std::string& windowTitle = "");

    /**
     * @brief Swap buffers.
     */
    void swapBuffers();

    /**
     * @brief Perform any necessary cleanup actions.
     */
    void terminate();

    /**
     * @brief Get the GLFW window used
     * @return The GLFW window
     */
    GLFWwindow* getWindow();

#ifdef _WIN32
    HWND getWin32Window();
#endif


  };
}
