/**
 * @file  BasePath.hpp
 * @brief Single function to return the base path on any platform
 *
 * Created on: 31 Jan 2020
 *     Author: Dimitri Kourkoulis
 *    License: BSD 3-Clause License (see LICENSE file)
 */
#pragma once

#include <string>

/**
 * @brief Helper function that returns the path of the directory where the
 *        executable is located. It returns "" in all cases apart from when 
 *        running on MacOS / iOS.
 */

std::string getBasePath();
