/**
 * @file  BinaryFile.hpp
 * @brief Binary file loader
 *
 *  Created on: 2023/06/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once
#include <vector>
#include "File.hpp"
#include "Model.hpp"

namespace small3d {

  /**
   * @class BinaryFile
   * @brief Native model file loader. The model file loaded has to have been
   *        produced by converting a file in a supported format to a small3d
   *        native binary file using the format converter program, s3dfc,
   *        produced by this project's build scripts, for example by running
   *        s3dfc car.glb car.bin
   *        (this would convert a car gltf (.glb) model file to a native
   *        binary model file)
   *        ATTENTION: The native binary file has to be generated in Windows
   *        (using s3dfc) if it is going to be loaded in Windows or in Android
   *        and in Linux or in MacOS if it is going to be loaded anywhere else.
   */
  class BinaryFile : public File {

  private:


    BinaryFile(); // No default constructor

    // Forbid moving and copying
    BinaryFile(BinaryFile const&) = delete;
    void operator=(BinaryFile const&) = delete;
    BinaryFile(BinaryFile&&) = delete;
    void operator=(BinaryFile&&) = delete;

  public:
    /**
     * @brief Constructor
     * @param fileLocation Path to wavefront file
     */
    BinaryFile(const std::string& fileLocation);

    /**
     * @brief Load data from the Wavefront file into a Model
     * @param model The model to load the data to
     * @param meshName The name of the mesh to load
     */
    void load(Model& model, const std::string& meshName) override;

    /**
     * @brief Get a list of the names of the meshes contained in the
     *        file.
     * @return The list of mesh names
     */
    std::vector<std::string> getMeshNames() override;

  };
}
