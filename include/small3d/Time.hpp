/**
 * @file  Time.hpp
 * @brief Time functions
 *
 * Created on: 13 Sep 2022
 *     Author: Dimitri Kourkoulis
 *    License: BSD 3-Clause License (see LICENSE file)
 */
#pragma once

#include <string>

/**
 * @brief Helper function that returns the current time
 *        in seconds. On PC, the Renderer has to have been
 *        instantiated at least once (it is a singleton) for
 *        this function to work because getTimeInSeconds uses 
 *        glfwGetTime and the Renderer initialises glfw.
 */

double getTimeInSeconds();
