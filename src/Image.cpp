/*
 *  Image.cpp
 *
 *  Created on: 2014/10/18
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "Image.hpp"
#include <stdexcept>
#include <cstring>
#include "BasePath.hpp"

namespace small3d {

  const std::string Image::NOTRGBA = "Image format not recognised. Only RGB / RGBA png images are supported.";

  Image::Image(const std::string& fileLocation) : imageData() {
    initLogger();
    width = 0;
    height = 0;
    imageDataSize = 0;

    if (fileLocation != "") {
      std::vector<char> empty;
      this->load(getBasePath() + fileLocation, empty);
    }
  }

  void Image::readDataFromMemory(png_structp png_ptr, png_bytep outBytes,
    png_size_t byteCountToRead) {

    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    if (io_ptr == nullptr) {
      throw std::runtime_error("Could not get png reader from memory.");
    }

    memoryDataAndPos_& md = *reinterpret_cast<memoryDataAndPos_*>(io_ptr);

    if (byteCountToRead > md.data.size() - md.pos) {
      throw std::runtime_error("Tried to read more png bytes than those left in memory.");
    }

    memcpy(outBytes, &md.data[md.pos], byteCountToRead);
    md.pos += byteCountToRead;

  }

  Image::Image(std::vector<char>& data) {

    if (data.size() > 0) {
      this->load("", data);
    }
  }

  void Image::toColour(Vec4 colour) {
    width = 10;
    height = 10;
    imageDataSize = 400;
    imageData.resize(400);

    for (uint64_t i = 0; i < 100; ++i) {
      std::memcpy(&imageData[i * 4], &colour, 4 * sizeof(uint8_t));
    }
  }

  void Image::load(const std::string& fileLocation, std::vector<char>& data) {
    // Developed based on information and examples at
    // http://zarb.org/~gc/html/libpng.html
    // http://pulsarengine.com/2009/01/reading-png-images-from-memory/

    bool fromMemory = false;

    if (fileLocation == "") {
      if (data.size() == 0) {
        throw std::runtime_error("Image load function called without a file location and "
          " no memory data (one of the two must be used).");
      }
      fromMemory = true;
    }
    else if (data.size() != 0) {
      throw std::runtime_error("Image load function called with a file location AND "
        "memory data (either of the two can be used but not both).");
    }
    else {
      fromMemory = false;
    }

    FILE* fp = 0;

    if (!fromMemory) {

      fp = fopen(fileLocation.c_str(), "rb");
      if (!fp) {
        throw std::runtime_error("Could not open file " + fileLocation);
      }

    }
    else {
      std::copy(data.begin(), data.end(), std::back_inserter(memoryDataAndPos.data));
      memoryDataAndPos.pos = 0;
    }

    png_infop pngInformation = nullptr;
    png_structp pngStructure = nullptr;
    png_byte colorType;
    png_bytep* rowPointers = nullptr;

    unsigned char header[8]; // Using maximum size that can be checked

    if (!fromMemory) {

      fread(header, 1, 8, fp);

    }
    else {
      memcpy(header, &memoryDataAndPos.data[0], 8);
    }

    if (png_sig_cmp(header, 0, 8)) {

      if (!fromMemory) {

        fclose(fp);

      }

      throw std::runtime_error(
        "Image data not recognised as PNG.");
    }

    pngStructure = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      nullptr, nullptr, nullptr);

    if (!pngStructure) {

      if (!fromMemory) {


        fclose(fp);

      }

      throw std::runtime_error("Could not create PNG read structure.");
    }

    pngInformation = png_create_info_struct(pngStructure);

    if (!pngInformation) {
      png_destroy_read_struct(&pngStructure, nullptr, nullptr);

      if (!fromMemory) {


        fclose(fp);

      }

      throw std::runtime_error("Could not create PNG information structure.");
    }

    if (setjmp(png_jmpbuf(pngStructure))) {
      png_destroy_read_struct(&pngStructure, &pngInformation, nullptr);
      pngStructure = nullptr;
      pngInformation = nullptr;

      if (!fromMemory) {


        fclose(fp);

      }

      throw std::runtime_error("PNG read: Error calling setjmp. (1)");
    }

    if (!fromMemory) {

      png_init_io(pngStructure, fp);

    }
    else {
      png_set_read_fn(pngStructure, &memoryDataAndPos, &readDataFromMemory);
    }

    // Only when reading from a file because, when reading from memory, 
    // we start reading at byte 0.
    if (!fromMemory) {
      png_set_sig_bytes(pngStructure, 8);

    }

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
      throw std::runtime_error("PNG read: Error calling setjmp. (2)");
    }

    rowPointers = new png_bytep[sizeof(png_bytep) * height];

    for (unsigned long y = 0; y < height; y++) {
      rowPointers[y] = new png_byte[png_get_rowbytes(pngStructure,
        pngInformation)];
    }

    png_read_image(pngStructure, rowPointers);

    if (colorType != PNG_COLOR_TYPE_RGB && colorType != PNG_COLOR_TYPE_RGBA) {
      throw std::runtime_error(NOTRGBA);
    }

    unsigned int numComponents = colorType == PNG_COLOR_TYPE_RGB ? 3 : 4;

    imageDataSize = 4 * width * height;

    LOGDEBUG("Reading " + std::to_string(imageDataSize * sizeof(uint8_t)) + " bytes of image data. uint8_t size " +
      std::to_string(sizeof(uint8_t)) + ", dimensions " + std::to_string(width) + ", " + std::to_string(height));

    imageData.resize(imageDataSize);

    for (uint64_t y = 0; y < height; y++) {

      png_byte* row = rowPointers[y];

      for (uint64_t x = 0; x < width; x++) {

        png_byte const * const ptr = &(row[x * numComponents]);

        uint8_t rgb[4];

        rgb[0] = static_cast<uint8_t>(ptr[0]);
        rgb[1] = static_cast<uint8_t>(ptr[1]);
        rgb[2] = static_cast<uint8_t>(ptr[2]);
        rgb[3] = numComponents == 3 ? 255 : static_cast<uint8_t>(ptr[3]);

        imageData[y * width * 4 + x * 4] = rgb[0];
        imageData[y * width * 4 + x * 4 + 1] = rgb[1];
        imageData[y * width * 4 + x * 4 + 2] = rgb[2];
        imageData[y * width * 4 + x * 4 + 3] = rgb[3];

      }
    }

    if (!fromMemory) {

      fclose(fp);

    }
    else {
      memoryDataAndPos.data.clear();
      memoryDataAndPos.pos = 0;
    }

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

  const uint8_t* Image::getData() const {
    return imageData.data();
  }

}
