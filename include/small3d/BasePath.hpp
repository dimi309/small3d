/*
 * @file  BasePath.hpp
 *
 * Created on: 31 Jan 2020
 *     Author: Dimitri Kourkoulis
 *    License: BSD 3-Clause License (see LICENSE file)
 */
#pragma once

#include <string>

/**
 * @brief Helper function that returns the path where the executable
 *        is located. It returns "" in all cases appart from when running
 *        on Apple machines, in which case  the execution directory is not 
 *        generally the one where the executable is located when running it
 *        from the command line, and accessing files from paths of the format
 *        "directory/file.txt" can produce errors if not prefixed with the
 *        full path.
 */

std::string getBasePath();
