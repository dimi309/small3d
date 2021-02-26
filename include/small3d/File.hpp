/**
 * @file  File.hpp
 * @brief File parser virtual class
 *
 *  Created on: 2021/01/30
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include <string>

namespace small3d {

  class Model;

  /**
   * @class File
   * @brief File parser class
   */
  class File {
  private:
    File(); // No default constructor

    // Forbid moving and copying
    File(File const&) = delete;
    void operator=(File const&) = delete;
    File(File&&) = delete;
    void operator=(File&&) = delete;

  protected:
    std::string fileLocation = "";

  public:
    /**
     * @brief Constructor
     * @param fileLocation Path to the file
     */
    File(const std::string& fileLocation);
    
    /**
    * @brief Load data from the file into a Model
    * @param model The model to load the data to
    * @param meshName The name of the mesh to load (unused in the case of Wavefront)
    */
    virtual void load(Model& model, const std::string& meshName) = 0;
    
  };
}
