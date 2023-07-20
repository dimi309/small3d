#include "Model.hpp"
#include "GlbFile.hpp"
#include "WavefrontFile.hpp"
#include <cereal/archives/binary.hpp>
#include <sstream>
#include <ostream>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include "BinaryFile.hpp"
#include "Sound.hpp"
#include <zlib.h>

using namespace small3d;

const uint32_t CHUNK = 16384;
bool isSound = false;

int main(int argc, char** argv) {

  if (argc > 2) {

    std::string modelpath =  (argv[1]);
    std::string binpath =  (argv[2]);

    Model model;
    Sound sound;

    try {
      Model m(GlbFile(modelpath), "");
      model = m;
    }
    catch (std::exception &ex) {
      try {
        Model m1(WavefrontFile(modelpath), "");
        model = m1;
      }
      catch (std::exception& ex1) {
        try {

          Sound s1(modelpath);
          sound = s1;
          isSound = true;
        }
        catch (std::exception& ex2) {
          throw std::runtime_error("Could not parse " + modelpath + ".");
        }
      }
    }

    if (!isSound) {
      std::stringstream ss("", std::ios::out | std::ios::binary);

      cereal::BinaryOutputArchive oarchive(ss);
      oarchive(model);

      unsigned char out[CHUNK];
      uint32_t have = 0;
      z_stream strm;
      int flush;

      strm.zalloc = Z_NULL;
      strm.zfree = Z_NULL;
      strm.opaque = Z_NULL;

      if (deflateInit(&strm, Z_DEFAULT_COMPRESSION) != Z_OK) {
        std::cout << "Failed to initialise deflate stream." << std::endl;;
        return 1;
      }

      auto strBuffer = ss.str();

      strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(strBuffer.c_str()));
      strm.avail_in = strBuffer.length();
      std::string compressedData = "";
      int defRet = 0;
      do {
        strm.avail_out = CHUNK;
        strm.next_out = out;

        defRet = deflate(&strm, strm.avail_in > 0 ? Z_NO_FLUSH : Z_FINISH);
        if (defRet == Z_STREAM_ERROR) {
          LOGERROR("Stream error");
        }
        have = CHUNK - strm.avail_out;
        for (uint32_t idx = 0; idx < have; ++idx) {
          compressedData += out[idx];
        }
      } while (defRet != Z_STREAM_END);
      deflateEnd(&strm);

      std::ofstream ofstr(binpath, std::ios::out | std::ios::binary);
      ofstr.write(compressedData.c_str(), compressedData.length());
      ofstr.close();
    }
    else {
      sound.saveBinary(binpath);
    }

    try {
      if (!isSound) {
        Model m1(BinaryFile(binpath), "");
      }
      else {
        Sound s1(binpath);
      }
      std::cout << "ok" << std::endl;
    }
    catch (std::exception ex) {
      std::cout << "Something went wrong while testing the file " << binpath << ": " << ex.what() << std::endl;
    }
  }
  else {
    std::cout << "Please provide source and target filename / path." << std::endl;
  }
  
  return 0;
}
