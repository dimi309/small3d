/**
 * @file  GlbFile.cpp
 *
 *  Created on: 2021/01/30
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "GlbFile.hpp"
#include "Logger.hpp"

#ifdef __ANDROID__
#include "vkzos.h"
#include <streambuf>
#include <istream>

struct membuf : std::streambuf
{
  membuf(char* begin, char* end) {
    this->setg(begin, begin, end);
  }
};
#endif

namespace small3d {

  std::shared_ptr<GlbFile::Token> GlbFile::getToken(const std::string& name, size_t index) {
    std::shared_ptr<GlbFile::Token> foundToken = nullptr;

    std::shared_ptr<GlbFile::Token> currenttoken = token_queues[index];
    do {
      if (currenttoken->name == name) {
        foundToken = currenttoken;
        break;
      }
      else {
        currenttoken = currenttoken->next;
      }
      if (foundToken != nullptr) break;
    } while (currenttoken != nullptr);

    return foundToken;
  }

  void GlbFile::printTokensRecursive(std::shared_ptr<GlbFile::Token> head_token) {
    std::shared_ptr<GlbFile::Token> currenttoken = head_token;
    do {
      if (currenttoken->valueType == GlbFile::ValueType::MARKER) {
        if (!currenttoken->name.empty()) {
          printf("%s: ", currenttoken->name.c_str());
        }
        printTokensRecursive(token_queues[static_cast<uint32_t>(std::stoi(currenttoken->value))]);
      }
      else {
        printToken(currenttoken);
        printf(" ");
      }
      currenttoken = currenttoken->next;
    } while (currenttoken != nullptr);
    printf("\n\r");
  }

  void GlbFile::lexJson(const std::vector<char>& json, uint32_t length)
  {
    uint32_t charsLeft = length;

    std::string tokenString = "";
    std::string tokenNumber = "";

    bool inQuotes = false;
    char c;
    bool inNumber = false;

    jsonRootToken = std::shared_ptr<GlbFile::Token>(new GlbFile::Token);
    jsonRootToken->next = nullptr;
    jsonRootToken->valueType = GlbFile::ValueType::charstring;
    jsonRootToken->value = "root";

    std::shared_ptr<GlbFile::Token> current = jsonRootToken;

    while (charsLeft != 0) {
      c = json[length - charsLeft];

      // end number
      if (inNumber && strchr("0123456789.-e", c) == nullptr) {

        current->next = createToken(GlbFile::ValueType::number, tokenNumber);
        current = current->next;

        inNumber = false;
        tokenNumber = "";

      }

      switch (c) {
      case '{': // begin map

        current->next = createToken(GlbFile::ValueType::character, &c);
        current = current->next;

        break;
      case '}': // end map
        current->next = createToken(GlbFile::ValueType::character, &c);
        current = current->next;

        break;
      case '[': // begin list
        current->next = createToken(GlbFile::ValueType::character, &c);
        current = current->next;

        break;
      case ']': // end list
        current->next = createToken(GlbFile::ValueType::character, &c);
        current = current->next;

        break;
      case '"':
        inQuotes = !inQuotes;
        if (!inQuotes) { // end string

          current->next = createToken(GlbFile::ValueType::charstring, tokenString);
          current = current->next;

          tokenString = "";
        }
        break;

      default:
        if (inQuotes) { // begin string
          tokenString += c;
        }
        else {
          if (strchr("0123456789-", c) != nullptr ||
            (strchr(".e", c) != nullptr && inNumber)) { // begin number or in number
            inNumber = true;
            tokenNumber += c;

          }
        }
        break;
      }

      if (c == ':') {
        current->next = createToken(GlbFile::ValueType::character, &c);
        current = current->next;

      }

      if (c == ',') {
        current->next = createToken(GlbFile::ValueType::character, &c);
        current = current->next;

      }

      --charsLeft;
    }
  }

  std::shared_ptr<GlbFile::Token> GlbFile::createToken(GlbFile::ValueType valueType, const std::string& value) {
    std::shared_ptr<GlbFile::Token> newToken(new GlbFile::Token);
    newToken->next = nullptr;
    newToken->name = "";
    newToken->valueType = valueType;
    newToken->value = value;
    if (valueType == GlbFile::ValueType::character) newToken->value.resize(1);
    return newToken;
  }

  void GlbFile::parseJson(std::shared_ptr<GlbFile::Token> token) {

    std::stack<std::shared_ptr<GlbFile::Token>> st;

    std::shared_ptr<GlbFile::Token> poppedToken;
    std::shared_ptr<GlbFile::Token> prevToken;
    std::shared_ptr<GlbFile::Token> antePrevToken;
    std::shared_ptr<GlbFile::Token> origNextToken;

    char poppedTokenFirstChar;

    uint32_t markerPoint = 0;

    while (true) {

      // Commas are ignored
      if (token->value == ",") {
        token = token->next;
        continue;
      }

      // The next token before breaking
      // current token off from the
      // original tokenisation queue
      origNextToken = token->next;
      st.push(token);

      char matching_bracket = token->value == "}" ? '{' :
        token->value == "]" ? '[' : ' ';

      if (matching_bracket != ' ') {
        prevToken = nullptr;
        antePrevToken = nullptr;
        do {

          poppedToken = st.top();
          poppedTokenFirstChar = poppedToken->value[0];
          st.pop();
          bool namedToken = false;
          if (antePrevToken != nullptr) {
            if (prevToken->valueType == GlbFile::ValueType::character && prevToken->value == ":" && poppedToken->valueType == GlbFile::ValueType::charstring) {
              antePrevToken->name = poppedToken->value;
              prevToken = antePrevToken;
              antePrevToken = prevToken->next;
              namedToken = true;
            }
          }

          if (namedToken) {
            namedToken = false;
          }
          else {
            poppedToken->next = prevToken;
            antePrevToken = prevToken;
            prevToken = poppedToken;
          }
        } while (poppedTokenFirstChar != matching_bracket);

        token_queues.push_back(poppedToken);

        char tkn[4] = "000";
        sprintf(tkn, "%d", markerPoint);
        ++markerPoint;
        std::shared_ptr<GlbFile::Token> marker = createToken(GlbFile::ValueType::MARKER, tkn);
        marker->next = jsonRootToken;
        jsonRootToken = marker;
        st.push(marker);

      }

      if (origNextToken == nullptr) break;
      token = origNextToken;
    }
  }

  std::vector<std::shared_ptr<GlbFile::Token>> GlbFile::getTokens(uint32_t index) {
    std::vector<std::shared_ptr<GlbFile::Token>> ret;
    std::shared_ptr<GlbFile::Token> currentToken = token_queues[index];
    do {
      if (currentToken->value != "[" && currentToken->value != "]" &&
        currentToken->value != "{" && currentToken->value != "}") {
        ret.push_back(currentToken);
      }
      currentToken = currentToken->next;
    } while (currentToken != nullptr);
    return ret;
  }

  GlbFile::GlbFile(const std::string& filename) {

#ifdef __ANDROID__
    AAsset* asset = AAssetManager_open(vkz_android_app->activity->assetManager,
      filename.c_str(),
      AASSET_MODE_STREAMING);
    if (!asset) throw std::runtime_error("Opening asset " + filename +
      " has failed!");
    off_t assetLength;
    assetLength = AAsset_getLength(asset);
    const void* buffer = AAsset_getBuffer(asset);
    membuf sbuf((char*)buffer, (char*)buffer + sizeof(char) * assetLength);
    std::istream fileOnDisk(&sbuf);
    if (!fileOnDisk) {
      throw std::runtime_error("Reading file " + filename +
			       " has failed!");
    }
#else
    std::ifstream fileOnDisk;
    fileOnDisk.open(filename, std::ios::binary);
    if (!fileOnDisk.is_open()) {
      throw std::runtime_error("Could not open GLB file " + filename);
    }
#endif

    std::string magic(4, '\0'); // 4 + 1 for \0
    uint32_t version = 0, length = 0;
    fileOnDisk.read(&magic[0], 4);
    fileOnDisk.read(reinterpret_cast<char*>(&version), 4);

    if (magic != "glTF" || version != 2) {
      throw std::runtime_error(filename + " is not a glTF v2 GLB file!");
    }

    fileOnDisk.read(reinterpret_cast<char*>(&length), 4);

    bool doneReading = false;
    uint32_t chunkLength, chunkType, bytesLeft = length - 12;
    std::vector<char> bytes;

    while (!doneReading) {
      chunkLength = 0;
      chunkType = 0;
      fileOnDisk.read(reinterpret_cast<char*>(&chunkLength), 4);
      fileOnDisk.read(reinterpret_cast<char*>(&chunkType), 4);

      if (chunkType == CHUNK_TYPE_BIN) {
        binBuffer.resize(chunkLength, 0);
        fileOnDisk.read(&binBuffer[0], chunkLength);
      }
      else if (chunkType == CHUNK_TYPE_JSON) {
        bytes.resize(chunkLength, 0);
        fileOnDisk.read(&bytes[0], chunkLength);
        lexJson(bytes, chunkLength);
      }
      else {

        LOGDEBUG("Unknown GLB chunk type or padding reached. Done reading... \n\r");
        doneReading = true;
      }

      if (!doneReading) {
        bytesLeft -= chunkLength;
        if (fileOnDisk.eof() || bytesLeft <= 0) doneReading = true;
      }
    }

#ifdef __ANDROID__
    AAsset_close(asset);
#else
    fileOnDisk.close();
#endif

    parseJson(jsonRootToken);

  }

  void GlbFile::printToken(const std::shared_ptr<GlbFile::Token>& token) {
    if (!token->name.empty()) {
      printf("%s: ", token->name.c_str());
    }

    if (token->valueType == GlbFile::ValueType::MARKER) {
      printf("M/%s", token->value.c_str());
    }
    else printf("%s", token->value.c_str());
  }

  void GlbFile::printTokensRecursive() {
    printTokensRecursive(token_queues.back());
  }

  void GlbFile::printTokensSerial() {
    for (auto tokenQueue : token_queues) {
      std::shared_ptr<GlbFile::Token> currenttoken = tokenQueue;
      do {
        printToken(currenttoken);
        printf(" ");
        currenttoken = currenttoken->next;
      } while (currenttoken != nullptr);
      printf("\n\r");
    }
  }

  std::shared_ptr<GlbFile::Token> GlbFile::getToken(const std::string& name) {
    return getToken(name, token_queues.size() - 1);
  }

  std::vector<std::shared_ptr<GlbFile::Token>> GlbFile::getChildTokens(const std::shared_ptr<GlbFile::Token>& token) {
    return getTokens(std::stoi(token->value));
  }

  std::shared_ptr<GlbFile::Token> GlbFile::getChildToken(const std::shared_ptr<GlbFile::Token>& token, const std::string& name) {
    return getToken(name, std::stoi(token->value));
  }

  std::vector<char> GlbFile::getBufferByView(const size_t viewIndex) {
    std::vector<std::shared_ptr<GlbFile::Token>> bufferViews = getChildTokens(getToken("bufferViews"));

    uint32_t byteLength = std::stoi(getChildToken(bufferViews[viewIndex], "byteLength")->value);
    uint32_t byteOffset = std::stoi(getChildToken(bufferViews[viewIndex], "byteOffset")->value);

    return std::vector<char>(binBuffer.begin() + byteOffset, binBuffer.begin() + byteOffset + byteLength);
  }

  std::vector<char> GlbFile::getBufferByAccessor(const size_t index) {

    auto accessorToken = getChildTokens(getToken("accessors"))[index];
    auto bufferViewNumber = std::stoi(getChildToken(accessorToken, "bufferView")->value);

    auto byteOffsetToken = getChildToken(accessorToken, "byteOffset");
    auto countToken = getChildToken(accessorToken, "count");

    if (byteOffsetToken == nullptr || countToken == nullptr) {
      return getBufferByView(bufferViewNumber);
    }
    else {
      size_t cnt = 0;

      auto componentType = std::stoi(getChildToken(accessorToken, "componentType")->value);

      switch (componentType) {
      case 5120:  // byte
        cnt = 1;
        break;
      case 5121:  // unsigned byte
        cnt = 1;
        break;
      case 5122:  // short
        cnt = 2;
        break;
      case 5123:  // unsigned short
        cnt = 2;
        break;
      case 5125:  // unsigned int
        cnt = 4;
        break;
      case 5126:  // float
        cnt = 4;
        break;
      default:
        throw std::runtime_error("Unrecognised componentType in GLB file: " + std::to_string(componentType));
         
      }

      auto dataType = getChildToken(accessorToken, "type")->value;
      
      if (dataType.substr(0, 3) == "VEC") {
        cnt *= std::stoi(dataType.substr(3, 1));
      }

      cnt *= std::stoi(countToken->value);
      
      auto byteOffset = std::stoi(byteOffsetToken->value);

      auto data = getBufferByView(bufferViewNumber);
      return std::vector<char>(data.begin() + byteOffset, data.begin() + byteOffset + cnt);
    }

  }

  bool GlbFile::existNode(const uint32_t index) {
    return getChildTokens(getToken("nodes")).size() > index;
  }

  GlbFile::Node GlbFile::getNode(const uint32_t index) {
    auto nodeToken = getChildTokens(getToken("nodes"))[index];

    Node ret;

    auto propToken = getChildToken(nodeToken, "name");
    if (propToken != nullptr) {
      ret.name = propToken->value;
    }

    propToken = getChildToken(nodeToken, "rotation");
    if (propToken != nullptr) {
      auto values = getChildTokens(propToken);
      ret.rotation = glm::quat(std::stof(values[3]->value), std::stof(values[0]->value),
        std::stof(values[1]->value), std::stof(values[2]->value));
    }

    propToken = getChildToken(nodeToken, "scale");
    if (propToken != nullptr) {
      auto values = getChildTokens(propToken);
      ret.scale = glm::vec3(std::stof(values[0]->value), std::stof(values[1]->value),
        std::stof(values[2]->value));
    }

    propToken = getChildToken(nodeToken, "translation");
    if (propToken != nullptr) {
      auto values = getChildTokens(propToken);
      ret.translation = glm::vec3(std::stof(values[0]->value), std::stof(values[1]->value),
        std::stof(values[2]->value));
    }

    propToken = getChildToken(nodeToken, "children");
    if (propToken != nullptr) {
      auto values = getChildTokens(propToken);
      for (auto &val : values) {
        ret.children.push_back(std::stoi(val->value));
      }
    }

    propToken = getChildToken(nodeToken, "mesh");
    if (propToken != nullptr) {
      ret.mesh = std::stoi(propToken->value);
    }

    propToken = getChildToken(nodeToken, "skin");
    if (propToken != nullptr) {
      ret.skin = std::stoi(propToken->value);
    }

    return ret;
  }

  bool GlbFile::existNode(const std::string& name) {
    auto nodeTokens = getChildTokens(getToken("nodes"));
    for (auto& nodeToken : nodeTokens) {
      if (getChildToken(nodeToken, "name")->value == name) {

        return true;
      }
    }
    return false;
  }

  GlbFile::Node GlbFile::getNode(const std::string& name) {

    auto nodeTokens = getChildTokens(getToken("nodes"));

    bool found = false;
    uint32_t nodeIndex = 0;

    for (auto &nodeToken : nodeTokens) {
      if (getChildToken(nodeToken, "name")->value == name) {
        found = true;
        break;
      }
      ++nodeIndex;
    }

    if (!found) throw std::runtime_error("Node " + name + " not found.");

    return getNode(nodeIndex);

  }

  bool GlbFile::existSkin(const uint32_t index) {
    return getChildTokens(getToken("skins")).size() > index;
  }

  GlbFile::Skin GlbFile::getSkin(const uint32_t index) {
    auto skinToken = getChildTokens(getToken("skins"))[index];
    
    Skin ret;

    auto propToken = getChildToken(skinToken, "name");
    if (propToken != nullptr) {
      ret.name = propToken->value;
    }

    propToken = getChildToken(skinToken, "inverseBindMatrices");
    if (propToken != nullptr) {
      ret.inverseBindMatrices = std::stoi(propToken->value);
    }

    propToken = getChildToken(skinToken, "joints");
    if (propToken != nullptr) {
      auto jointTokens = getChildTokens(propToken);
      for (auto &jointToken : jointTokens) {
        ret.joints.push_back(std::stoi(jointToken->value));
      }
    }

    return ret;
  }

  bool GlbFile::existSkin(const std::string& name) {
    auto skinTokens = getChildTokens(getToken("skins"));
    for (auto& skinToken : skinTokens) {
      if (getChildToken(skinToken, "name")->value == name) {
        return true;
      }
    }
    return false;
  }

  GlbFile::Skin GlbFile::getSkin(const std::string& name) {

    auto skinTokens = getChildTokens(getToken("skins"));

    bool found = false;
    uint32_t nodeIndex = 0;

    for (auto& skinToken : skinTokens) {
      if (getChildToken(skinToken, "name")->value == name) {
        found = true;
        break;
      }
      ++nodeIndex;
    }

    if (!found) throw std::runtime_error("Skin " + name + " not found.");

    return getSkin(nodeIndex);

  }

  GlbFile::Animation GlbFile::getAnimation(const uint32_t index) {
    auto animationToken = getChildTokens(getToken("animations"))[index];

    GlbFile::Animation ret;

    auto propToken = getChildToken(animationToken, "name");
    if (propToken != nullptr) {
      ret.name = propToken->value;
    }

    propToken = getChildToken(animationToken, "channels");
    if (propToken != nullptr) {
      auto channelTokens = getChildTokens(propToken);
      for (auto& channelToken : channelTokens) {
        AnimationChannel channel;
        auto samplerToken = getChildToken(channelToken, "sampler");
        if (samplerToken != nullptr) {
          channel.sampler = stoi(samplerToken->value);
        }

        auto targetToken = getChildToken(channelToken, "target");
        if (targetToken != nullptr) {
          ChannelTarget target;
          auto nodeToken = getChildToken(targetToken, "node");
          if (nodeToken != nullptr) {
            target.node = std::stoi(nodeToken->value);
          }
          auto pathToken = getChildToken(targetToken, "path");
          if (pathToken != nullptr) {
            target.path = pathToken->value;
          }
          channel.target = target;
        }
        ret.channels.push_back(channel);
      }
    }

    propToken = getChildToken(animationToken, "samplers");
    if (propToken != nullptr) {
      auto samplerTokens = getChildTokens(propToken);
      for (auto& samplerToken : samplerTokens) {
        AnimationSampler sampler;
        auto inputToken = getChildToken(samplerToken, "input");
        if (inputToken != nullptr) {
          sampler.input = std::stoi(inputToken->value);
        }

        auto interpolationToken = getChildToken(samplerToken, "interpolation");
        if (interpolationToken != nullptr) {
          sampler.interpolation = interpolationToken->value;
        }

        auto outputToken = getChildToken(samplerToken, "output");
        if (outputToken != nullptr) {
          sampler.output = std::stoi(outputToken->value);
        }

        ret.samplers.push_back(sampler);

      }

    }
    
    return ret;
  }

  GlbFile::Animation GlbFile::getAnimation(const std::string& name) {
    auto animationTokens = getChildTokens(getToken("animations"));

    bool found = false;
    uint32_t nodeIndex = 0;

    for (auto& animationToken : animationTokens) {
      if (getChildToken(animationToken, "name")->value == name) {
        found = true;
        break;
      }
      ++nodeIndex;
    }

    if (!found) throw std::runtime_error("Animation " + name + " not found.");

    return getAnimation(nodeIndex);
  }

}
