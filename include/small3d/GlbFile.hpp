/**
 * @file  GlbFile.hpp
 * @brief .glb file parser
 *
 *  Created on: 2021/01/30
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include <fstream>
#include <string>
#include <stack>
#include <vector>
#include <memory>
#include <cstring>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
#include "Model.hpp"
#include "File.hpp"

namespace small3d {

  /**
   * @class GlbFile
   * @brief .glb (glTF) file parser class
   */
  class GlbFile : public File {

  public:
    /**
     * @brief String saying that what comes before it is not a .glb file
     */
    static const std::string NOTGLTF;

  private:

    enum class ValueType { number = 0, charstring, character, MARKER };

    struct Token {
      ValueType valueType = ValueType::charstring;
      std::string value;
      std::shared_ptr<Token> next;
      std::string name;
    };

    struct Node {
      uint32_t index = 0;
      std::string name = "";
      glm::mat4 transformation = glm::mat4(1.0f);
      glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
      glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
      glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
      uint32_t skin = 0;
      bool noSkin = false;
      uint32_t mesh = 0;
      std::vector<uint32_t> children;
    };

    struct Skin {
      std::string name;
      uint32_t inverseBindMatrices = 0;
      std::vector<uint32_t> joints;
    };

    struct AnimationSampler {
      uint32_t input = 0;
      std::string interpolation;
      uint32_t output = 0;
    };

    struct ChannelTarget {
      uint32_t node = 0;
      std::string path;
    };

    struct AnimationChannel {
      uint32_t sampler = 0;
      ChannelTarget target;
    };

    struct Animation {
      std::string name;
      std::vector<AnimationChannel> channels;
      std::vector<AnimationSampler> samplers;
    };

    const uint32_t CHUNK_TYPE_JSON = 0x4E4F534A;
    const uint32_t CHUNK_TYPE_BIN = 0x004E4942;

    std::shared_ptr<Token> jsonRootToken;
    std::vector<char> binBuffer;

    // The result of parsing json is a series of token queues,
    // each queue being one of the found maps or lists. Some
    // of the nodes in the queues are references to other
    // queues.
    std::vector<std::shared_ptr<Token>> token_queues;

    std::shared_ptr<Token> getToken(const std::string&, size_t);
    void printTokensRecursive(std::shared_ptr<Token>);
    void lexJson(const std::vector<char>&, uint32_t);
    std::shared_ptr<Token> createToken(ValueType, const std::string&);
    void parseJson(std::shared_ptr<Token>);
    std::vector<std::shared_ptr<Token>> getTokens(uint32_t);

    void printToken(const std::shared_ptr<Token>& token);

    std::shared_ptr<Token> getToken(const std::string& name);

    std::vector<std::shared_ptr<Token>> getChildTokens(const std::shared_ptr<Token>& token);

    std::shared_ptr<Token> getChildToken(const std::shared_ptr<GlbFile::Token>& token, const std::string& name);

    std::vector<char> getBufferByView(const size_t index);

    std::vector<char> getBufferByAccessor(const size_t index);

    bool existNode(const uint32_t index);

    Node getNode(const uint32_t index);

    bool existNode(const std::string& name);

    Node getNode(const std::string& name);

    bool existParentNode(const uint32_t index);

    Node getParentNode(const uint32_t index);

    bool existNodeForMesh(const uint32_t meshIndex);

    Node getNodeForMesh(const uint32_t meshIndex);

    bool existSkin(const uint32_t index);

    Skin getSkin(const uint32_t index);

    bool existSkin(const std::string& name);

    Skin getSkin(const std::string& name);

    bool existAnimation(const uint32_t index);

    Animation getAnimation(const uint32_t index);

    Animation getAnimation(const std::string& name);

    GlbFile(); // No default constructor

    // Forbid moving and copying
    GlbFile(GlbFile const&) = delete;
    void operator=(GlbFile const&) = delete;
    GlbFile(GlbFile&&) = delete;
    void operator=(GlbFile&&) = delete;

  public:

    /**
     * @brief Constructor of the GlbFile class.
     * @param fileLocation The name and path of the GLB file to be parsed
     */
    GlbFile(const std::string& fileLocation);

    /**
     * @brief Recursively print all tokens, starting from the head
     * of the first token queue, dereferencing queue references.
     */
    void printTokensRecursive();

    /**
     * @brief Print tokens as found in the token queues, without
     * dereferencing queue references.
     */
    void printTokensSerial();

    /**
     * @brief Load a mesh from the file into a Model
     * @param model The Model into which to load the data
     * @param meshName The name of the mesh to load
     * 
     */
    void load(Model& model, const std::string& meshName);

    /**
     * @brief Get a list of the names of the meshes contained in the
     *        file.
     * @return The list of mesh names
     */
    std::vector<std::string> getMeshNames();

  };

}
