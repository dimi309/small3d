/**
 *  BinaryFile.cpp
 *
 *  Created on: 2023/06/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include <cereal/archives/binary.hpp>

#include <fstream>

#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

#include "BinaryFile.hpp"

#ifdef __ANDROID__
#include "small3d_android.h"
#include <streambuf>
#include <istream>

struct membuf : std::streambuf
{
  membuf(char* begin, char* end) {
    this->setg(begin, begin, end);
  }
};
#endif

using namespace small3d;
BinaryFile::BinaryFile(const std::string& fileLocation) : File(fileLocation) {

}


void BinaryFile::load(Model& model, const std::string& meshName) {

#ifdef __ANDROID__
  AAsset* asset = AAssetManager_open(small3d_android_app->activity->assetManager,
                                     fullPath.c_str(),
                                     AASSET_MODE_STREAMING);
  if (!asset) throw std::runtime_error("Opening asset " + fullPath +
                                       " has failed!");
  off_t assetLength;
  assetLength = AAsset_getLength(asset);
  const void* buffer = AAsset_getBuffer(asset);
  membuf sbuf((char*)buffer, (char*)buffer + sizeof(char) * assetLength);
  std::istream is(&sbuf);
  if (!is) {
    throw std::runtime_error("Could not open file " + fullPath);
  }
#else
  std::ifstream is;
  is.open(fullPath, std::ios::in | std::ios::binary);
  if (!is.is_open()) {
    throw std::runtime_error("Could not open file " + fullPath);
  }
#endif

  cereal::BinaryInputArchive iarchive(is);
  iarchive(model);
#ifdef __ANDROID__
  AAsset_close(asset);
#else
  is.close();
#endif
  LOGDEBUG("Loaded model from binary file " + fullPath);
}


std::vector<std::string> BinaryFile::getMeshNames() {

  std::vector<std::string> blank;
  return blank;

}

