/**
 * @file  GlbFile.hpp
 * @brief GLB file parser
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
   * @brief GLB file parser class
   */
  class GlbFile : public File {
  public:
    /**
     * @brief Types of value for the tokens parsed from the GLB file.
     */
    enum class ValueType { number = 0, charstring, character, MARKER };

    /**
     * @brief This token structure is used for storing the parsed GLB file data.
     */
    struct Token {
      ValueType valueType = ValueType::charstring;
      std::string value;
      std::shared_ptr<Token> next;
      std::string name;
    };

    /**
     * @brief glTF node
     */
    struct Node {
      std::string name = "";
      glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
      glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
      glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
      uint32_t skin = 0;
      bool noSkin = false;
      uint32_t mesh = 0;
      std::vector<uint32_t> children;
    };

    /**
     * @brief glTF skin
     */
    struct Skin {
      std::string name;
      uint32_t inverseBindMatrices = 0;
      std::vector<uint32_t> joints;
    };

    /**
     * @brief glTF animation sampler
     */
    struct AnimationSampler {
      uint32_t input = 0;
      std::string interpolation;
      uint32_t output = 0;
    };

    /**
     * @brief glTF animation channel target
     */
    struct ChannelTarget {
      uint32_t node = 0;
      std::string path;
    };

    /**
     * @brief glTF animation channel
     */
    struct AnimationChannel {
      uint32_t sampler = 0;
      ChannelTarget target;
    };

    /**
     * @brief glTF animation
     */
    struct Animation {
      std::string name;
      std::vector<AnimationChannel> channels;
      std::vector<AnimationSampler> samplers;
    };

  private:

    const uint32_t CHUNK_TYPE_JSON = 0x4E4F534A;
    const uint32_t CHUNK_TYPE_BIN = 0x004E4942;

    std::shared_ptr<Token> jsonRootToken;
    std::vector<char> binBuffer;

    // The result of parsing json is a series of token queues,
    // each queue being one of the found maps or lists. Some
    // of the nodes in the queues are references to other
    // queues.
    std::vector<std::shared_ptr<Token>> token_queues;

    // Forbid default and copy constructors
    GlbFile();
    GlbFile(const GlbFile&);

    std::shared_ptr<Token> getToken(const std::string&, size_t);
    void printTokensRecursive(std::shared_ptr<Token>);
    void lexJson(const std::vector<char>&, uint32_t);
    std::shared_ptr<Token> createToken(ValueType, const std::string&);
    void parseJson(std::shared_ptr<Token>);
    std::vector<std::shared_ptr<Token>> getTokens(uint32_t);

  public:

    /**
     * @brief Constructor of the GlbFile class.
     * @param filename The name (and path) of the GLB file to be parsed
     */
    GlbFile(const std::string& filename);

    /**
    * @brief Print a token
    * @param token The token to be printed
    */
    void printToken(const std::shared_ptr<Token>& token);

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
     * @brief Get a token by name
     * @param name The name of the token
     * @return Shared pointer to the token
     */
    std::shared_ptr<Token> getToken(const std::string& name);

    /**
     * @brief Get the child tokens of a token
     * @param token The token the children of which will be retrieved
     * @return Vector of shared pointers to the retrieved tokens
     */
    std::vector<std::shared_ptr<Token>> getChildTokens(const std::shared_ptr<Token>& token);

    /**
     * @brief Get a child token of a token by name
     * @param token The token the child of which will be retrieved
     * @param token The name of the child which will be retrieved
     * @return Shared pointer to the retrieved token
     */
    std::shared_ptr<Token> getChildToken(const std::shared_ptr<GlbFile::Token>& token, const std::string& name);

    /**
     * @brief Get the data of a buffer from the binary part of the file, using the buffer view index to locate it
     * @param index The buffer view index
     * @return Vector of the retrieved bytes (chars)
     */
    std::vector<char> getBufferByView(const size_t index);

    /**
     * @brief Get the data of a buffer from the binary part of the file, using the accessor view index to locate it
     *        This method can be preferable to getBufferByView because the accessor allows the reading of only part
     *        of the data described by the view, something which is sometimes required.
     * @param index The buffer accessor index
     * @return Vector of the retrieved bytes (chars)
     */
    std::vector<char> getBufferByAccessor(const size_t index);

    /**
     * @brief Does a node with the given index exist?
     * @param index The index of the node
     * @return True if the node exists, false otherwise
     */
    bool existNode(const uint32_t index);

    /**
     * @brief  Get a glTF node by index
     * @param  index The index of the node in the file.
     * @return The node
     */
    Node getNode(const uint32_t index);

    /**
     * @brief Does a node with the given name exist?
     * @param name The name of the node
     * @return True if the node exists, false otherwise
     */
    bool existNode(const std::string& name);

    /**
     * @brief  Get a glTF node by name
     * @param  name The name of the node in the file.
     * @return The node
     */
    Node getNode(const std::string& name);

    /**
     * @brief Does a skin with the given index exist?
     * @param index The index of the skin
     * @return True if the skin exists, false otherwise
     */
    bool existSkin(const uint32_t index);

    /**
     * @brief Get a skin by index
     * @param index The index of the skin in the file
     * @return The skin
     */
    Skin getSkin(const uint32_t index);

    /**
     * @brief Does a skin with the given name exist?
     * @param name The name of the skin
     * @return True if the skin exists, false otherwise
     */
    bool existSkin(const std::string& name);

    /**
     * @brief  Get a skin by name
     * @param  name The name of the skin in the file.
     * @return The skin
     */
    Skin getSkin(const std::string& name);

    /**
     * @brief Does an animation with the given index exist?
     * @param index The index of the animation
     * @return True if the animation exists, false otherwise
     */
    bool existAnimation(const uint32_t index);

    /**
     * @brief Get an animation by index
     * @param index The index of the animation in the file
     * @return The animation
     */
    Animation getAnimation(const uint32_t index);

    /**
     * @brief Get an animation by name
     * @param name The name of the animation in the file
     * @return The animation
     */
    Animation getAnimation(const std::string& name);

    void load(Model& model);

  };

}
