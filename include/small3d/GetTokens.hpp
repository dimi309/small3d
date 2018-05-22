/**
 *  @file  GetTokens.hpp
 *  @brief Declaration of the getTokens function
 *
 *  Created on: 2014/10/18
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include <string>
#include <vector>

namespace small3d {

  /**
   * @brief Separates a string into tokens, using the given character
   *        as a separator
   *
   * @param input           The input string
   * @param sep	            The separator
   * @param [in,out] tokens The tokens
   *
   * @return The number of tokens
   */

  int getTokens(const std::string &input, const char sep,
		std::vector<std::string> &tokens);
}
