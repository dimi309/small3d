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

namespace small3d {

  /**
   * @class GlbFile
   * @brief GLB file parser class
   */
  class GlbFile {
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
     * @brief A glTF node
     */
    struct Node {
      std::string name;
      glm::quat rotation;
      glm::vec3 scale;
      glm::vec3 translation;
      std::vector<uint32_t> children;
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
     * @brief  Get a glTF node by index
     * @param  index The index of the node in the file.
     * @return The node
     */
    Node getNode(const uint32_t index);

    /**
     * @brief  Get a glTF node by name
     * @param  name The name of the node in the file.
     * @return The node
     */
    Node getNode(const std::string& name);

  };

}
