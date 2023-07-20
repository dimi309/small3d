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
#include <zlib.h>

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

  std::string readData = "";
  const uint32_t CHUNK = 16384;

  char rd[CHUNK];
  while (!is.eof()) {
    memset(rd, 0, CHUNK);
    is.read(rd, CHUNK);
    for (uint32_t idx = 0; idx < is.gcount(); ++idx) {
      readData += rd[idx];
    }
  }
  is.close();

  unsigned char out[CHUNK];
  uint32_t have = 0;
  z_stream strm;
  int flush;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  if (inflateInit(&strm) != Z_OK) {
    throw std::runtime_error("Failed to initialise inflate stream.");
  }

  strm.avail_in = readData.length();
  strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(readData.c_str()));
  std::string uncompressedData = "";

  do {
    strm.avail_out = CHUNK;
    strm.next_out = out;
    if (inflate(&strm, Z_NO_FLUSH) == Z_STREAM_ERROR) {
      LOGERROR("Stream error");
    }
    have = CHUNK - strm.avail_out;
    for (uint32_t idx = 0; idx < have; ++idx) {
      uncompressedData += out[idx];
    }
  } while (strm.avail_out == 0);
  deflateEnd(&strm);
  
  std::istringstream iss(uncompressedData, std::ios::binary);

  cereal::BinaryInputArchive iarchive(iss);
  iarchive(model);
  iss.clear();
  uncompressedData.clear();

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

