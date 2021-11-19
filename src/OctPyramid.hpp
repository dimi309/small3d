/*
 * OctPyramid.hpp
 *
 *  Created on: 9 Oct 2021
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */
#pragma once
#include <small3d/Model.hpp>
class OctPyramid : public small3d::Model {

private:
  float height;
  float radius;
  glm::vec3 normalBetween(uint64_t triangle1, uint64_t triangle2);

public:
  OctPyramid(const float height = 3.9f, const float radius = 1.5f);

};

