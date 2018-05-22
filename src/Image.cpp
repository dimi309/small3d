/*
 *  Image.cpp
 *
 *  Created on: 2014/10/18
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "Image.hpp"
#include <stdexcept>

namespace small3d {

  Image::Image(const std::string fileLocation) : imageData() {
    initLogger();
    width = 0;
    height = 0;
    imageDataSize=0;

    if (fileLocation != "")
      this->loadFromFile(fileLocation);
  }

  void Image::loadFromFile(const std::string fileLocation) {
    // function developed based on example at
    // http://zarb.org/~gc/html/libpng.html
    FILE *fp = fopen(fileLocation.c_str(), "rb");

    if (!fp) {
      throw std::runtime_error("Could not open file " + fileLocation);
    }

    png_infop pngInformation = nullptr;
    png_structp pngStructure = nullptr;
    png_byte colorType;
    png_bytep *rowPointers = nullptr;

    unsigned char header[8]; // Using maximum size that can be checked

    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8)) {
      throw std::runtime_error(
        "File " + fileLocation
        + " is not recognised as a PNG file.");
    }

    pngStructure = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      nullptr, nullptr, nullptr);

    if (!pngStructure) {
      fclose(fp);
      throw std::runtime_error("Could not create PNG read structure.");
    }

    pngInformation = png_create_info_struct(pngStructure);

    if (!pngInformation) {
      png_destroy_read_struct(&pngStructure, nullptr, nullptr);
      fclose(fp);
      throw std::runtime_error("Could not create PNG information structure.");
    }

    if (setjmp(png_jmpbuf(pngStructure))) {
      png_destroy_read_struct(&pngStructure, &pngInformation, nullptr);
      pngStructure = nullptr;
      pngInformation = nullptr;
      fclose(fp);
      throw std::runtime_error("PNG read: Error calling setjmp. (1)");
    }

    png_init_io(pngStructure, fp);
    png_set_sig_bytes(pngStructure, 8);

    png_read_info(pngStructure, pngInformation);

    width = png_get_image_width(pngStructure, pngInformation);
    height = png_get_image_height(pngStructure, pngInformation);

    colorType = png_get_color_type(pngStructure, pngInformation);
    png_set_interlace_handling(pngStructure);

    png_read_update_info(pngStructure, pngInformation);

    if (setjmp(png_jmpbuf(pngStructure))) {
      png_destroy_read_struct(&pngStructure, &pngInformation, nullptr);
      pngStructure = nullptr;
      pngInformation = nullptr;
      fclose(fp);
      throw std::runtime_error("PNG read: Error calling setjmp. (2)");
    }

    rowPointers = new png_bytep[sizeof(png_bytep) * height];

    for (unsigned long y = 0; y < height; y++) {
      rowPointers[y] = new png_byte[png_get_rowbytes(pngStructure,
        pngInformation)];
    }

    png_read_image(pngStructure, rowPointers);

    if (colorType != PNG_COLOR_TYPE_RGB && colorType != PNG_COLOR_TYPE_RGBA) {
      throw std::runtime_error(
        "Image format not recognised. Only RGB / RGBA png images are "
	"supported.");
    }

    unsigned int numComponents = colorType == PNG_COLOR_TYPE_RGB ? 3 : 4;

    imageDataSize = 4 * width * height;

    imageData.resize(imageDataSize);

    for (unsigned long y = 0; y < height; y++) {

      png_byte *row = rowPointers[y];

      for (unsigned long x = 0; x < width; x++) {

        png_byte *ptr = &(row[x * numComponents]);

        float rgb[4];

        rgb[0] = static_cast<float>(ptr[0]);
        rgb[1] = static_cast<float>(ptr[1]);
        rgb[2] = static_cast<float>(ptr[2]);
        rgb[3] = numComponents == 3 ? 255.0f : static_cast<float>(ptr[3]);

        imageData[y * width * 4 + x * 4] = rgb[0] / 255.0f;
        imageData[y * width * 4 + x * 4 + 1] = rgb[1] / 255.0f;
        imageData[y * width * 4 + x * 4 + 2] = rgb[2] / 255.0f;
        imageData[y * width * 4 + x * 4 + 3] = rgb[3] / 255.0f;

      }
    }

    fclose(fp);

    for (unsigned long y = 0; y < height; y++) {
      delete[] rowPointers[y];
    }
    delete[] rowPointers;

    png_destroy_read_struct(&pngStructure, &pngInformation, nullptr);
    pngStructure = nullptr;
    pngInformation = nullptr;
  }

  unsigned long Image::getWidth() const {
    return width;
  }

  unsigned long Image::getHeight() const {
    return height;
  }

  unsigned long Image::getByteSize() const {
    return imageDataSize;
  }

  const float* Image::getData() const {
    return imageData.data();
  }

}
