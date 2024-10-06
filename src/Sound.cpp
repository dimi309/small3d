/*
 *  Sound.cpp
 *
 *  Created on: 2017-07-17
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "Sound.hpp"
#include "Logger.hpp"

#include <vector>
#include <stdexcept>
#include <cstring>
#include <vorbis/vorbisfile.h>
#include <cassert>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <zlib.h>
#include <fstream>


#include "Time.hpp"
#include "BasePath.hpp"

#define PORTAUDIO_SAMPLE_FORMAT paInt16


#define SOUND_ID(name, handle) name + "/" + handle

namespace small3d {

  bool Sound::noOutputDevice;
  unsigned int Sound::numInstances = 0;


  PaDeviceIndex Sound::defaultOutput;

  int Sound::audioCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {

    int result = paContinue;
    SoundData* soundData = static_cast<SoundData*>(userData);

    if (soundData->startTime == 0) {
      soundData->startTime = getTimeInSeconds() - 0.1;
    }
    else if (getTimeInSeconds() - soundData->startTime > soundData->duration) {
      if (soundData->repeat) {
        soundData->startTime = getTimeInSeconds() - 0.1;
        soundData->currentFrame = 0;
      }
      else {
        return paAbort;
      }
    }

    SAMPLE_DATATYPE* out = static_cast<SAMPLE_DATATYPE*>(outputBuffer);
    unsigned long startPos = soundData->currentFrame *
      static_cast<unsigned long>(soundData->channels);
    unsigned long endPos = startPos + framesPerBuffer *
      static_cast<unsigned long>(soundData->channels);

    if (endPos > static_cast<unsigned long>(soundData->samples) * WORD_SIZE *
      soundData->channels) {
      endPos = static_cast<unsigned long>(soundData->samples) * WORD_SIZE *
        soundData->channels;
      result = paAbort;
    }

    for (unsigned long i = startPos; i < endPos;
      i += static_cast<unsigned long>(soundData->channels)) {
      for (int c = 0; c < soundData->channels; ++c) {
        *out++ = (reinterpret_cast<short*>(soundData->data.data()))[i + c];
        // Write two channels if there is only one in the file,
        // otherwise the sound will only play on one speaker when using jack.
        if (soundData->channels == 1) {
          *out++ = (reinterpret_cast<short*>(soundData->data.data()))[i + c];
        }
      }
    }

    soundData->currentFrame += framesPerBuffer;

    return result;
  }


  Sound::Sound() {

    this->stream = nullptr;

    if (numInstances == 0) {
      LOGDEBUG("No Sound instances exist. Initialising PortAudio");

      noOutputDevice = false;

      PaError initError = Pa_Initialize();

      if (initError != paNoError) {
        throw std::runtime_error("PortAudio failed to initialise: " +
          std::string(Pa_GetErrorText(initError)));
      }

      auto numDevices = Pa_GetDeviceCount();
      if (numDevices <= 0) {
        LOGERROR("Could not retrieve any sound devices! Pa_CountDevices returned: " +
          std::to_string(numDevices));
        noOutputDevice = true;
        LOGERROR("Sound disabled.");
        noOutputDevice = true;
      }
      else {

        defaultOutput = Pa_GetDefaultOutputDevice();

        if (defaultOutput == paNoDevice) {
          LOGERROR("No default sound output device.");
          LOGERROR("Sound disabled.");
          noOutputDevice = true;
        }
      }
    }

    ++numInstances;
  }

  Sound::Sound(const std::string& soundFilePath) : Sound() {

    this->load(getBasePath() + soundFilePath);

  }

  Sound::~Sound() {


    if (stream != nullptr) {

      stop();


      Pa_CloseStream(stream);

    }
    --numInstances;
    if (numInstances == 0) {


      //At times, this has caused crashes on MacOS
      Pa_Terminate();

    }
  }

  void Sound::load(const std::string& soundFilePath) {

    if (!noOutputDevice) {

      try {

        OggVorbis_File vorbisFile;


        FILE* fp = fopen((soundFilePath).c_str(), "rb");
        if (!fp) {
          throw std::runtime_error("Could not open file " + soundFilePath);
        }

        if (ov_open_callbacks(reinterpret_cast<void*>(fp), &vorbisFile, NULL, 0,
          OV_CALLBACKS_NOCLOSE) < 0) {
          fclose(fp);
          throw std::runtime_error("Could not read file " +
            soundFilePath + " as ogg.");
        }
        else {
          LOGDEBUG("Opened OV callbacks for " + soundFilePath + ".");
        }


        vorbis_info* vi = ov_info(&vorbisFile, -1);

        this->soundData.channels = vi->channels;
        this->soundData.rate = (int)vi->rate;
        this->soundData.samples =
          static_cast<long>(ov_pcm_total(&vorbisFile, -1));
        this->soundData.size = soundData.channels * soundData.samples * WORD_SIZE;
        this->soundData.duration = static_cast<double>(soundData.samples) /
          static_cast<double>(soundData.rate);

        char pcmout[4096];
        int current_section;
        long ret = 0;
        long pos = 0;

        do {
          ret = ov_read(&vorbisFile, pcmout, sizeof(pcmout), 0, WORD_SIZE, 1,
            &current_section);
          if (ret < 0) {

            fclose(fp);

            throw std::runtime_error("Error in sound stream.");

          }
          else if (ret > 0) {

            this->soundData.data.insert(soundData.data.end(), &pcmout[0],
              &pcmout[ret]);

            pos += ret;

          }
        } while (ret != 0);

        ov_clear(&vorbisFile);


        fclose(fp);

      }
      catch (std::exception& ex) {
        LOGDEBUG(ex.what());
        LOGDEBUG("Opening as binary...");


        std::ifstream is;
        is.open(soundFilePath, std::ios::in | std::ios::binary);
        if (!is.is_open()) {
          throw std::runtime_error("Could not open file " + soundFilePath);
        }


        std::string readData = "";
        const uint32_t CHUNK = 16384;

        char rd[CHUNK];
        while (!is.eof()) {
          memset(rd, 0, CHUNK);
          is.read(rd, CHUNK);
          for (uint32_t idx = 0; idx < is.gcount(); ++idx) {
            readData += rd[idx];
          }
        }

        is.close();
        unsigned char out[CHUNK];
        z_stream strm;

        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;

        if (inflateInit(&strm) != Z_OK) {
          throw std::runtime_error("Failed to initialise inflate stream.");
        }

        strm.avail_in = readData.length();
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(readData.c_str()));
        std::string uncompressedData = "";

        do {
          strm.avail_out = CHUNK;
          strm.next_out = out;
          if (inflate(&strm, Z_NO_FLUSH) == Z_STREAM_ERROR) {
            LOGERROR("Stream error");
          }
          uint32_t have = CHUNK - strm.avail_out;
          for (uint32_t idx = 0; idx < have; ++idx) {
            uncompressedData += out[idx];
          }
        } while (strm.avail_out == 0);
        deflateEnd(&strm);

        std::istringstream iss(uncompressedData, std::ios::binary | std::ios::in);

        cereal::BinaryInputArchive iarchive(iss);
        iarchive(this->soundData);
        iss.clear();
        uncompressedData.clear();


        is.close();

        LOGDEBUG("Loaded sound from binary file " + soundFilePath);

      }

      LOGDEBUG("Loaded sound - channels " + std::to_string(this->soundData.channels) +
        " - rate " + std::to_string(this->soundData.rate) + " - samples " +
        std::to_string(this->soundData.samples) + " - size in bytes " +
        std::to_string(this->soundData.size) + " - duration " +
        std::to_string(this->soundData.duration) +
        +" - start time " + std::to_string(this->soundData.startTime) +
        +" - repeat? " + std::to_string(this->soundData.repeat) +
        +" - current frame " + std::to_string(this->soundData.currentFrame) +
        +" - data vector size " + std::to_string(this->soundData.data.size()) +
        +" - playing repeat? " + std::to_string(this->soundData.playingRepeat)
      );

    }

    this->openStream();

  }

  void Sound::openStream() {
    if (noOutputDevice) return;


    PaStreamParameters outputParams = {};

    outputParams.device = defaultOutput;
    // Declare two channels if there is only one. This is
    // so that we can send the sound to both speakers when
    // using jack and not alsa on Linux. For the rest of the
    // cases, it will still work.
    outputParams.channelCount = this->soundData.channels == 1 ? 2 : this->soundData.channels;
    outputParams.hostApiSpecificStreamInfo = NULL;

    outputParams.sampleFormat = PORTAUDIO_SAMPLE_FORMAT;

    this->soundData.currentFrame = 0;
    this->soundData.startTime = 0;

    PaError error;

    error = Pa_OpenStream(&stream, NULL, &outputParams, this->soundData.rate,
      1024, paNoFlag,
      Sound::audioCallback, &this->soundData);
    if (error != paNoError) {
      throw std::runtime_error("Failed to open PortAudio stream: " +
        std::string(Pa_GetErrorText(error)));
    }

  }

  void Sound::divideVolume(uint32_t divisor) {

    assert(WORD_SIZE == 2);

    for (auto dp = soundData.data.begin(); dp != soundData.data.end(); dp += 2) {
      int16_t n;
      memcpy(&n, &(*dp), 2);
      n /= divisor;
      memcpy(&(*dp), &n, 2);
    }
  }

  void Sound::play(const bool repeat) {
    if (repeat) {
      if (soundData.playingRepeat) return;
      soundData.playingRepeat = true;
    }

    if (!noOutputDevice && this->soundData.size > 0) {


      if (Pa_IsStreamActive(stream)) return;

      PaError error;

      if (Pa_IsStreamStopped(stream) == 0) {
        error = Pa_AbortStream(stream);
        if (error != paNoError) {
          throw std::runtime_error("Failed to abort stream on play: " +
            std::string(Pa_GetErrorText(error)));
        }
      }

      this->soundData.currentFrame = 0;
      this->soundData.startTime = 0;
      this->soundData.repeat = repeat;


      error = Pa_StartStream(stream);
      if (error != paNoError) {
        throw std::runtime_error("Failed to start stream: " +
          std::string(Pa_GetErrorText(error)));
      }

    }
  }

  void Sound::stop() {
    if (soundData.repeat) {
      if (!soundData.playingRepeat) return;
      soundData.playingRepeat = false;
    }

    if (Pa_IsStreamStopped(stream)) return;



    if (this->stream != nullptr) {


      Pa_AbortStream(stream);

      this->soundData.currentFrame = 0;
      this->soundData.startTime = 0;
    }
  }

  Sound::Sound(const Sound& other) : Sound() {
    this->soundData = other.soundData;


    this->stream = nullptr;

    this->openStream();
    ++numInstances;
  }

  Sound::Sound(const Sound&& other) : Sound() {
    this->soundData = other.soundData;

    this->stream = nullptr;

    this->openStream();
    ++numInstances;
  }

  Sound& Sound::operator=(const Sound& other) {

    if (this->stream != nullptr) {

      Pa_AbortStream(this->stream);
      Pa_CloseStream(this->stream);

    }


    this->soundData = other.soundData;


    this->stream = nullptr;

    this->openStream();
    return *this;
  }

  Sound& Sound::operator=(const Sound&& other) {

    if (this->stream != nullptr) {


      Pa_AbortStream(this->stream);
      Pa_CloseStream(this->stream);

    }
    this->soundData = other.soundData;

    this->stream = nullptr;

    this->openStream();
    return *this;
  }

  void Sound::saveBinary(const std::string & binaryFilePath) {

    const uint32_t CHUNK = 16384;

    std::stringstream ss(std::ios::out | std::ios::binary | std::ios::trunc);
    cereal::BinaryOutputArchive oarchive(ss);
    oarchive(soundData);

    unsigned char out[CHUNK];

    z_stream strm;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    if (deflateInit(&strm, Z_DEFAULT_COMPRESSION) != Z_OK) {
      throw std::runtime_error("Failed to initialise deflate stream.");
    }

    auto strBuffer = ss.str();

    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(strBuffer.c_str()));
    strm.avail_in = strBuffer.length();
    std::string compressedData = "";
    int defRet = 0;
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;

      defRet = deflate(&strm, strm.avail_in > 0 ? Z_NO_FLUSH : Z_FINISH);
      if (defRet == Z_STREAM_ERROR) {
        LOGERROR("Stream error");
      }
      uint32_t have = CHUNK - strm.avail_out;
      for (uint32_t idx = 0; idx < have; ++idx) {
        compressedData += out[idx];
      }
    } while (defRet != Z_STREAM_END);
    deflateEnd(&strm);

    std::ofstream ofstr(binaryFilePath, std::ios::out | std::ios::binary);
    ofstr.write(compressedData.c_str(), compressedData.length());
    ofstr.close();

  }
}
