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
using namespace small3d;

int main(int argc, char** argv) {
  if (argc > 2) {
    std::string modelpath(argv[1]);
    std::string binpath(argv[2]);

    Model model;

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
        throw std::runtime_error("Could not parse " + modelpath + ".");
      }
    }

    std::ofstream os;

    os.open(binpath, std::ios::out | std::ios::binary);

    if (!os.is_open()) {
      throw std::runtime_error("Could not open file " + binpath);
    }
    cereal::BinaryOutputArchive oarchive(os);
    oarchive(model);
    os.flush();
    os.close();

    try {
      Model m1(BinaryFile(binpath), "");
      std::cout << "ok" << std::endl;
    }
    catch (std::exception ex) {
      std::cout << "Something went wrong while testing the file " << binpath << ":" << ex.what() << std::endl;
    }

  }
  else {
    std::cout << "Please provide source and target filename / path." << std::endl;
  }
  
  return 0;
}
