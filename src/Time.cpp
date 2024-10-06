/*
 * Time.cpp
 *
 * Created on: 13 Sep 2022
 *     Author: Dimitri Kourkoulis
 *    License: BSD 3-Clause License (see LICENSE file)
 */

#include "Time.hpp"


#include <chrono>

double getTimeInSeconds() {

  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() / 1000.0;

}
