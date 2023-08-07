/**
 *  GlbFile.cpp
 *
 *  Created on: 2021/01/30
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "GlbFile.hpp"
#include "Logger.hpp"

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
    } while (currenttoken != nullptr);

    return foundToken;
  }

  void GlbFile::printTokensRecursive(std::shared_ptr<Token> head_token) {
    std::shared_ptr<GlbFile::Token> currenttoken = head_token;
    do {
      if (currenttoken->valueType == ValueType::MARKER) {
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
    bool inTrueOrFalse = false;

    jsonRootToken = std::make_shared<GlbFile::Token>();
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

      // end true or false
      if (inTrueOrFalse && strchr("truefals", c) == nullptr) {
        current->next = createToken(GlbFile::ValueType::charstring, tokenString);
        current = current->next;
        inTrueOrFalse = false;
        tokenString = "";
      }

      switch (c) {
      case '{': // begin map

        current->next = createToken(GlbFile::ValueType::character, std::string(1, c));
        current = current->next;

        break;
      case '}': // end map
        current->next = createToken(GlbFile::ValueType::character, std::string(1, c));
        current = current->next;

        break;
      case '[': // begin list
        current->next = createToken(GlbFile::ValueType::character, std::string(1, c));
        current = current->next;

        break;
      case ']': // end list
        current->next = createToken(GlbFile::ValueType::character, std::string(1, c));
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
        if (inQuotes) { // begin or continue string
          tokenString += c;
        }
        else if (strchr("0123456789-", c) != nullptr ||
          (strchr(".e", c) != nullptr && inNumber)) { // begin number or in number
          inNumber = true;
          tokenNumber += c;

        }
        else if (strchr("truefals", c) != nullptr) { // begin or continue true or false indicator
          inTrueOrFalse = true;
          tokenString += c;
        }

        break;
      }

      if (c == ':') {
        current->next = createToken(GlbFile::ValueType::character, std::string(1, c));
        current = current->next;

      }

      if (c == ',') {
        current->next = createToken(GlbFile::ValueType::character, std::string(1, c));
        current = current->next;

      }

      --charsLeft;
    }
  }

  std::shared_ptr<GlbFile::Token> GlbFile::createToken(GlbFile::ValueType valueType, const std::string& value) {
    auto newToken = std::make_shared<GlbFile::Token>();
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

        token_queues.emplace_back(poppedToken);
        std::shared_ptr<GlbFile::Token> marker = createToken(GlbFile::ValueType::MARKER, std::to_string(markerPoint));
        ++markerPoint;
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
        ret.emplace_back(currentToken);
      }
      currentToken = currentToken->next;
    } while (currentToken != nullptr);
    return ret;
  }

  GlbFile::GlbFile(const std::string& fileLocation) : File(fileLocation) {

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
    std::istream fileOnDisk(&sbuf);
    if (!fileOnDisk) {
      throw std::runtime_error("Reading file " + fullPath +
        " has failed!");
    }
#else
    std::ifstream fileOnDisk;
    fileOnDisk.open(fullPath, std::ios::binary);
    if (!fileOnDisk.is_open()) {
      throw std::runtime_error("Could not open .glb file " + fullPath);
    }
#endif

    std::string magic(4, '\0');
    uint32_t version = 0, length = 0;
    fileOnDisk.read(&magic[0], 4);
    fileOnDisk.read(reinterpret_cast<char*>(&version), 4);

    if (magic != "glTF" || version != 2) {
#ifdef __ANDROID__
      AAsset_close(asset);
#else
      fileOnDisk.close();
#endif
      throw std::runtime_error("Magic number found: '" + magic + "'. File " + fullPath + " cannot be read as glb.");
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

        // Unknown .glb chunk type or padding reached. Done reading.
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
    auto offsetToken = getChildToken(bufferViews[viewIndex], "byteOffset");
    uint32_t byteOffset = offsetToken == nullptr ? 0U : std::stoi(offsetToken->value);
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
      else if (dataType == "MAT4") {
        cnt *= 16;
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

    ret.index = index;

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
      for (auto& val : values) {
        ret.children.emplace_back(std::stoi(val->value));
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
    else {
      ret.noSkin = true;
    }

    propToken = getChildToken(nodeToken, "matrix");
    if (propToken != nullptr) {
      auto values = getChildTokens(propToken);
      uint32_t idx = 0;
      for (auto& val : values) {
        ret.transformation[idx / 4][idx % 4] = std::stof(val->value);
        ++idx;
      }
    }
    return ret;
  }

  bool GlbFile::existParentNode(const uint32_t index) {
    bool found = false;
    auto nodeTokens = getChildTokens(getToken("nodes"));
    for (auto& nodeToken : nodeTokens) {
      auto childrenToken = getChildToken(nodeToken, "children");
      if (childrenToken != nullptr) {
        auto children = getChildTokens(childrenToken);
        for (auto& childToken : children) {
          if (std::stoi(childToken->value) == index) {
            found = true;
            break;
          }
        }
      }
    }
    return found;
  }

  GlbFile::Node GlbFile::getParentNode(const uint32_t index) {

    bool found = false;
    GlbFile::Node ret;
    uint32_t nodeIdx = 0;
    auto nodeTokens = getChildTokens(getToken("nodes"));
    for (auto& nodeToken : nodeTokens) {
      auto childrenToken = getChildToken(nodeToken, "children");
      if (childrenToken != nullptr) {
        auto children = getChildTokens(childrenToken);
        for (auto& childToken : children) {
          if (std::stoi(childToken->value) == index) {
            found = true;
            ret = getNode(nodeIdx);
            break;
          }
        }
      }
      ++nodeIdx;
    }
    if (!found) {
      throw std::runtime_error("Parent node of node " + std::to_string(index) + " not found.");
    }
    return ret;
  }

  bool GlbFile::existNodeForMesh(const uint32_t meshIndex) {
    bool found = false;
    for (auto& nodeToken : getChildTokens(getToken("nodes"))) {
      auto nodeMeshToken = getChildToken(nodeToken, "mesh");
      if (nodeMeshToken != nullptr) {
        if (meshIndex == std::stof(nodeMeshToken->value)) {
          found = true;
          break;
        }
      }
    }
    return found;
  }

  GlbFile::Node GlbFile::getNodeForMesh(const uint32_t meshIndex) {
    Node nodeForMesh;
    uint32_t nodeIdx = 0;
    bool found = false;
    for (auto& nodeToken : getChildTokens(getToken("nodes"))) {
      auto nodeMeshToken = getChildToken(nodeToken, "mesh");
      if (nodeMeshToken != nullptr) {
        if (meshIndex == std::stof(nodeMeshToken->value)) {
          found = true;
          nodeForMesh = getNode(nodeIdx);
          break;
        }
      }
      ++nodeIdx;
    }
    if (!found) {
      throw std::runtime_error("Node for mesh " + std::to_string(meshIndex) + " not found.");
    }
    return nodeForMesh;
  }

  bool GlbFile::existNode(const std::string& name) {
    auto nodeTokens = getChildTokens(getToken("nodes"));
    for (auto& nodeToken : nodeTokens) {
      auto nameToken = getChildToken(nodeToken, "name");
      if (nameToken != nullptr && nameToken->value == name) {
        return true;
      }
    }
    return false;
  }

  GlbFile::Node GlbFile::getNode(const std::string& name) {

    auto nodeTokens = getChildTokens(getToken("nodes"));

    bool found = false;
    uint32_t nodeIndex = 0;

    for (auto& nodeToken : nodeTokens) {
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
    if (getToken("skins") == nullptr) return false;
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
      for (auto& jointToken : jointTokens) {
        ret.joints.emplace_back(std::stoi(jointToken->value));
      }
    }

    propToken = getChildToken(skinToken, "skeleton");
    if (propToken != nullptr) {
      ret.skeleton = std::stoi(propToken->value);
      ret.foundSkeleton = true;
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

  bool GlbFile::existAnimation(const uint32_t index) {
    if (getToken("animations") == nullptr) return false;
    return getChildTokens(getToken("animations")).size() > index;
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

        ret.channels.emplace_back(channel);
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

        ret.samplers.emplace_back(sampler);

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

  void GlbFile::load(Model& model, const std::string& meshName) {

    bool loaded = false;
    std::string actualName = "";
    uint32_t meshIndex = 0;
    for (auto& meshToken : getChildTokens(getToken("meshes"))) {

      actualName = getChildToken(meshToken, "name")->value;
      if (actualName == meshName || meshName == "") { // Just get the first mesh if no name is given.
        auto primitives = getChildTokens(
          getChildToken(meshToken, "primitives"));
        auto attributes = getChildTokens(getChildToken(primitives[0], "attributes"));

        for (auto attribute : attributes) {

          if (attribute->name == "POSITION") {

            auto data = getBufferByAccessor(std::stoi(attribute->value));

            auto numFloatsInData = data.size() / 4;

            model.vertexData.resize(numFloatsInData + numFloatsInData / 3); // Add 1 float for each 3 for 
            // the w component of each vector

            model.vertexDataByteSize = static_cast<uint32_t>(model.vertexData.size() * 4); // Each vertex component is 4 bytes
            size_t vertexPos = 0;

            for (size_t idx = 0; idx < numFloatsInData; ++idx) {
              memcpy(&model.vertexData[vertexPos], &data[idx * 4], 4);
              ++vertexPos;
              if (idx > 0 && (idx + 1) % 3 == 0) {
                model.vertexData[vertexPos] = 1.0f;
                ++vertexPos;
              }
            }
          }

          if (attribute->name == "NORMAL") {
            auto data = getBufferByAccessor(std::stoi(attribute->value));
            model.normalsData.resize(data.size() / 4);
            model.normalsDataByteSize = static_cast<uint32_t>(data.size());
            memcpy(&model.normalsData[0], &data[0], data.size());

          }

          if (attribute->name == "TEXCOORD_0") {
            auto data = getBufferByAccessor(std::stoi(attribute->value));
            model.textureCoordsData.resize(data.size() / 4);
            model.textureCoordsDataByteSize = static_cast<uint32_t>(data.size());
            memcpy(&model.textureCoordsData[0], &data[0], data.size());
          }

          if (attribute->name == "JOINTS_0") {
            auto data = getBufferByAccessor(std::stoi(attribute->value));
            model.jointData.resize(data.size());
            model.jointDataByteSize = static_cast<uint32_t>(data.size());
            memcpy(&model.jointData[0], &data[0], data.size());
          }

          if (attribute->name == "WEIGHTS_0") {
            auto data = getBufferByAccessor(std::stoi(attribute->value));
            model.weightData.resize(data.size() / 4);
            model.weightDataByteSize = static_cast<uint32_t>(data.size());
            memcpy(&model.weightData[0], &data[0], data.size());
          }

        }

        auto indicesToken = getChildToken(primitives[0], "indices");
        if (indicesToken == nullptr) {
          // Just create indices that serially output the vertices
          model.indexData.resize(model.vertexData.size() / 4);
          uint32_t cnt = 0;
          for (auto& point : model.indexData) {
            point = cnt;
            cnt++;
          }
          model.indexDataByteSize = static_cast<uint32_t>(model.indexData.size() * 4);
        }
        else {
          auto data = getBufferByAccessor(std::stoi(indicesToken->value));
          model.indexData.resize(data.size() / 2);
          model.indexDataByteSize =
            static_cast<uint32_t>(model.indexData.size() * 4); // 4 because, even though each index is read in 16 bits,
          // it is stored in 32 bits
          uint16_t indexBuf = 0;
          for (size_t idx = 0; idx < model.indexData.size(); ++idx) {
            memcpy(&indexBuf, &data[2 * idx], 2);
            model.indexData[idx] = static_cast<uint32_t>(indexBuf);
          }
        }
        auto materialToken = getChildToken(primitives[0], "material");
        if (materialToken != nullptr) {

          uint32_t materialIndex = std::stoi(materialToken->value);

          auto materialToken = getChildTokens(getToken("materials"))[materialIndex];

          auto metallicRoughnessToken = getChildToken(materialToken, "pbrMetallicRoughness");
          if (metallicRoughnessToken != nullptr) {
            auto baseColorTextureToken = getChildToken(metallicRoughnessToken, "baseColorTexture");
            if (baseColorTextureToken != nullptr) {
              uint32_t textureIndex = std::stoi(getChildToken(baseColorTextureToken, "index")->value);
              uint32_t sourceIndex = std::stoi(getChildToken(getChildTokens(getToken("textures"))[textureIndex], "source")->value);
              auto imageToken = getChildTokens(getToken("images"))[sourceIndex];
              if (getChildToken(imageToken, "mimeType")->value == "image/png") {
                auto imageData = getBufferByView(std::stoi(getChildToken(imageToken, "bufferView")->value));

                try {
                  model.defaultTextureImage = std::make_shared<Image>(imageData);
                }
                catch (std::runtime_error& e) {
                  if (e.what() == Image::NOTRGBA) {
                    LOGDEBUG(e.what());
                    LOGDEBUG("Texture ignored.");
                  }
                }
                imageData.clear();

              }
              else {
                LOGINFO("Warning! Only PNG images embedded in .glb files can be read. Texture ignored.");
              }
            }
          }
        }

        loaded = true;
        break;
      }
      ++meshIndex;
    }

    if (!loaded) throw std::runtime_error("Could not load mesh " + meshName + " from " + fullPath);

    LOGDEBUG("Loaded mesh " + actualName + " from " + fullPath);

    if (existNode(actualName) || existNodeForMesh(meshIndex)) {
      Node meshNode;
      if (existNode(actualName)) {
        meshNode = getNode(actualName);
      }
      else {
        meshNode = getNodeForMesh(meshIndex);
      }

      model.origTransformation = meshNode.transformation;
      model.origRotation = meshNode.rotation;
      model.origTranslation = meshNode.translation;
      model.origScale = meshNode.scale;

      auto tmpNode = meshNode;


      // Getting armature and z_up transformations if they exist
      uint32_t parentCount = 0;
      while (existParentNode(tmpNode.index)) {
        tmpNode = getParentNode(tmpNode.index);
        ++parentCount;
        LOGDEBUG("Parent of mesh found (" + std::to_string(parentCount) + "): " + tmpNode.name);
        model.origRotation = tmpNode.rotation * model.origRotation;
        model.origTranslation += tmpNode.translation;
        model.origScale.x *= tmpNode.scale.x;
        model.origScale.y *= tmpNode.scale.y;
        model.origScale.z *= tmpNode.scale.z;
        model.origTransformation = tmpNode.transformation * model.origTransformation;
      }

      if (!meshNode.noSkin && existSkin(meshNode.skin)) {

        auto skin = getSkin(meshNode.skin);

        /*if (existNode(skin.name)) {
          auto skinNode = getNode(skin.name);
          model.origRotation = skinNode.rotation;
          model.origTranslation = skinNode.translation;
        }*/

        if (skin.joints.size() > Model::MAX_JOINTS_SUPPORTED) {
          LOGDEBUG("Found more than the maximum of " +
            std::to_string(Model::MAX_JOINTS_SUPPORTED) + " supported joints. Ignoring all.");
          return;
        }

        auto inverseBindMatrices = getBufferByAccessor(skin.inverseBindMatrices);
        uint64_t idx = 0;

        for (auto jointIdx : skin.joints) {
          Model::Joint j;

          memcpy(&j.inverseBindMatrix, &inverseBindMatrices[idx * 16 * 4], 16 * 4);
          auto jointNode = getNode(jointIdx);
          j.node = jointIdx;
          j.name = jointNode.name;
          j.rotation = jointNode.rotation;
          j.scale = jointNode.scale;
          j.translation = jointNode.translation;
          j.children = jointNode.children;
          model.joints.emplace_back(j);

          ++idx;
        }




        bool inputStored = false;
        uint32_t storedInput = 0;
        //bool cubicSplineWarningEmitted = false;
        bool multipleInputWarningEmitted = false;

        if (existAnimation(0)) {
          auto animation = getAnimation(0);


          for (auto& channel : animation.channels) {

            auto sampler = animation.samplers[channel.sampler];
            /*if (sampler.interpolation == "CUBICSPLINE") {
              if (!cubicSplineWarningEmitted) {
                LOGDEBUG("Cubic spline interpolation samplers for animations ignored.");
                cubicSplineWarningEmitted = true;
              }
              continue;
            }*/

            if (!inputStored) {
              storedInput = sampler.input;
              inputStored = true;
            }

            if (sampler.input != storedInput) {
              if (!multipleInputWarningEmitted) {
                LOGDEBUG("WARNING: Multiple animation inputs not supported well.");
                multipleInputWarningEmitted = true;
              }
              //continue; // Uncomment to only read animations based on the first input found
            }

            auto input = getBufferByAccessor(sampler.input);
            std::vector<float>times(input.size() / 4);
            memcpy(&times[0], &input[0], input.size());

            auto output = getBufferByAccessor(sampler.output);

            for (auto& joint : model.joints) {
              if (joint.node == channel.target.node) {
                bool found = false;
                uint32_t animIndex = 0;
                for (auto& animation : joint.animations) {
                  if (animation.input == sampler.input) {
                    found = true;
                    break;
                    ++animIndex;
                  }
                }

                if (!found) {
                  joint.animations.emplace_back(Model::JointAnimation());
                  animIndex = joint.animations.size() - 1;
                  joint.animations[animIndex].times = times;
                  joint.animations[animIndex].input = sampler.input;
                }

                if (channel.target.path == "rotation") {
                  joint.animations[animIndex].rotationAnimation.resize(output.size() / sizeof(glm::quat));
                  memcpy(&joint.animations[animIndex].rotationAnimation[0], &output[0], output.size());
                  if (model.numPoses < joint.animations[animIndex].rotationAnimation.size())
                    model.numPoses = joint.animations[animIndex].rotationAnimation.size();
                }

                if (channel.target.path == "translation") {
                  joint.animations[animIndex].translationAnimation.resize(output.size() / sizeof(glm::vec3));
                  memcpy(&joint.animations[animIndex].translationAnimation[0], &output[0], output.size());
                  if (model.numPoses < joint.animations[animIndex].translationAnimation.size())
                    model.numPoses = joint.animations[animIndex].translationAnimation.size();
                }

                if (channel.target.path == "scale") {
                  joint.animations[animIndex].scaleAnimation.resize(output.size() / sizeof(glm::vec3));
                  memcpy(&joint.animations[animIndex].scaleAnimation[0], &output[0], output.size());
                  if (model.numPoses < joint.animations[animIndex].scaleAnimation.size())
                    model.numPoses = joint.animations[animIndex].scaleAnimation.size();
                }

              }
            }
          }
        }
      }
    }
  }

  std::vector<std::string> GlbFile::getMeshNames() {
    std::vector<std::string> names;

    for (auto& meshToken : getChildTokens(getToken("meshes"))) {
      auto nameToken = getChildToken(meshToken, "name");
      if (nameToken != nullptr) {
        names.emplace_back(nameToken->value);
      }
    }
    return names;
  }
}
