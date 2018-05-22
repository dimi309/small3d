/**
 * @file  Image.hpp
 * @brief Header of the Image class
 *
 * Created on: 2014/10/18
 *     Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include "Logger.hpp"
#include <png.h>

namespace small3d {

  /**
   * @class Image
   *
   * @brief An image, loaded from a png file, which can be used for
   *        generating textures.
   *
   */

  class Image {
  private:

    unsigned long width, height;
    std::vector<float> imageData;
    unsigned long imageDataSize;
    void loadFromFile(const std::string fileLocation);

  public:

    /**
     * @brief Default constructor
     *
     * @param fileLocation Location of the png image file
     */
    Image(const std::string fileLocation = "");

    /**
     * @brief Destructor
     */
    ~Image() = default;

    /**
     * @brief Get the image width
     * @return The image width
     */
    unsigned long getWidth() const;

    /**
     * @brief Get the image height
     * @return The image height
     */
    unsigned long getHeight() const;

    /**
     * @brief Get the size of the image, in bytes
     * @return Size of the image, in bytes
     */
    unsigned long getByteSize() const;

    /**
     * @brief Get the image data
     * @return The image data
     */
    const float* getData() const;

  };

}
