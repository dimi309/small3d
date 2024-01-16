#include "Model.hpp"
#include "GlbFile.hpp"
#include "WavefrontFile.hpp"
#include "BinaryFile.hpp"
#include "Sound.hpp"

using namespace small3d;


bool isSound = false;

int main(int argc, char** argv) {
  try {
    if (argc > 2) {

      std::string modelpath = (argv[1]);
      std::string binpath = (argv[2]);

      Model model;
      Sound sound;

      try {
        Model m(GlbFile(modelpath), "");
        model = m;
      }
      catch (const std::exception& ex) {
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
          catch (const std::exception& ex2) {
            throw std::runtime_error("Could not parse " + modelpath + ".");
          }
        }
      }

      if (!isSound) {
        model.saveBinary(binpath);
        
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
      catch (const std::exception& ex) {
        std::cout << "Something went wrong while testing the file " << binpath << ": " << ex.what() << std::endl;
      }
    }
    else {
      std::cout << "Please provide source and target filename / path." << std::endl;
    }
  }
  catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    return 1;
  }
  
  return 0;
}
