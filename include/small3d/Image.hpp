/**
 * @file  Image.hpp
 * @brief Image loading and manipulation
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
#include <glm/vec4.hpp>

namespace small3d {

  /**
   * @class Image
   *
   * @brief An image, loaded from a .png file, which can be used for
   *        generating textures.
   *
   */

  class Image {
  private:

    struct memoryDataAndPos_ {
      std::vector<char> data;
      uint64_t pos = 0;
    };

    memoryDataAndPos_ memoryDataAndPos;

    unsigned long width = 0, height = 0;
    std::vector<float> imageData;
    unsigned long imageDataSize = 0;
    void load(const std::string fileLocation, std::vector<char>& data);
    static void readDataFromMemory(png_structp png_ptr, png_bytep outBytes,
      png_size_t byteCountToRead);

  public:

    /**
     * @brief String saying that the colour encoding of the image being read is not RGB/RGBA
     *
     */
    static const std::string NOTRGBA;

    /**
     * @brief File-reading constructor
     *
     * @param fileLocation Location of the png image file
     */
    Image(const std::string fileLocation = "");

    /**
     * @brief Memory-based constructor
     * 
     * @param data PNG bytes already read into memory
     */
    Image(std::vector<char>& data);

    /**
     * @brief Destructor
     */
    ~Image() = default;

    /**
     * @brief Convert to a coloured 10 x 10 pixel image
     * @param colour The colour of the image
     */
    void toColour(glm::vec4 colour);

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
