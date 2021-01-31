/**
 * @file  GlbFile.cpp
 *
 *  Created on: 2021/01/30
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "GlbFile.hpp"
#include "Logger.hpp"

namespace small3d {

  std::shared_ptr<GlbFile::tokent> GlbFile::getToken(const std::string& name, size_t index) {
    std::shared_ptr<GlbFile::tokent> foundToken = nullptr;

    std::shared_ptr<GlbFile::tokent> currenttoken = token_queues[index];
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

  void GlbFile::printTokensRecursive(std::shared_ptr<GlbFile::tokent> head_token) {
    std::shared_ptr<GlbFile::tokent> currenttoken = head_token;
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

    jsonRootToken = std::shared_ptr<GlbFile::tokent>(new GlbFile::tokent);
    jsonRootToken->next = nullptr;
    jsonRootToken->valueType = GlbFile::ValueType::charstring;
    jsonRootToken->value = "root";

    std::shared_ptr<GlbFile::tokent> current = jsonRootToken;

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
          if (strchr("0123456789", c) != nullptr ||
            (strchr(".-e", c) != nullptr && inNumber)) { // begin number
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

  std::shared_ptr<GlbFile::tokent> GlbFile::createToken(GlbFile::ValueType valueType, const std::string& value) {
    std::shared_ptr<GlbFile::tokent> newToken(new GlbFile::tokent);
    newToken->next = nullptr;
    newToken->name = "";
    newToken->valueType = valueType;
    newToken->value = value;
    if (valueType == GlbFile::ValueType::character) newToken->value.resize(1);
    return newToken;
  }

  void GlbFile::parseJson(std::shared_ptr<GlbFile::tokent> token) {

    std::stack<std::shared_ptr<GlbFile::tokent>> st;

    std::shared_ptr<GlbFile::tokent> poppedToken;
    std::shared_ptr<GlbFile::tokent> prevToken;
    std::shared_ptr<GlbFile::tokent> antePrevToken;
    std::shared_ptr<GlbFile::tokent> origNextToken;

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
        std::shared_ptr<GlbFile::tokent> marker = createToken(GlbFile::ValueType::MARKER, tkn);
        marker->next = jsonRootToken;
        jsonRootToken = marker;
        st.push(marker);

      }

      if (origNextToken == nullptr) break;
      token = origNextToken;
    }
  }

  std::vector<std::shared_ptr<GlbFile::tokent>> GlbFile::getTokens(uint32_t index) {
    std::vector<std::shared_ptr<GlbFile::tokent>> ret;
    std::shared_ptr<GlbFile::tokent> currentToken = token_queues[index];
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
    std::ifstream fileOnDisk;
    fileOnDisk.open(filename, std::ios::binary);
    if (!fileOnDisk.is_open()) {
      throw std::runtime_error("Could not open GLB file" + filename);
    }

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

    fileOnDisk.close();

    parseJson(jsonRootToken);

  }

  void GlbFile::printToken(std::shared_ptr<GlbFile::tokent> token) {
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
      std::shared_ptr<GlbFile::tokent> currenttoken = tokenQueue;
      do {
        printToken(currenttoken);
        printf(" ");
        currenttoken = currenttoken->next;
      } while (currenttoken != nullptr);
      printf("\n\r");
    }
  }

  std::shared_ptr<GlbFile::tokent> GlbFile::getToken(const std::string& name) {
    return getToken(name, token_queues.size() - 1);
  }

  std::vector<std::shared_ptr<GlbFile::tokent>> GlbFile::getChildTokens(std::shared_ptr<GlbFile::tokent> token) {
    return getTokens(std::stoi(token->value));
  }

  std::shared_ptr<GlbFile::tokent> GlbFile::getChildToken(std::shared_ptr<GlbFile::tokent> token, const std::string& name) {
    return getToken(name, std::stoi(token->value));
  }

  std::vector<char> GlbFile::getBufferByView(size_t viewIndex) {
    std::vector<std::shared_ptr<GlbFile::tokent>> bufferViews = getChildTokens(getToken("bufferViews"));

    uint32_t byteLength = std::stoi(getChildToken(bufferViews[viewIndex], "byteLength")->value);
    uint32_t byteOffset = std::stoi(getChildToken(bufferViews[viewIndex], "byteOffset")->value);

    return std::vector<char>(binBuffer.begin() + byteOffset, binBuffer.begin() + byteOffset + byteLength);

  }

  std::vector<char> GlbFile::getBufferByAccessor(size_t index) {

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
}