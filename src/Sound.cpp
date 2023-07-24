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


#if defined(__ANDROID__)
#include "small3d_android.h"

struct membuf : std::streambuf
{
  membuf(char* begin, char* end) {
    this->setg(begin, begin, end);
  }
};

#endif

#include "Time.hpp"
#include "BasePath.hpp"

#define PORTAUDIO_SAMPLE_FORMAT paInt16

#ifdef __ANDROID__

#include <thread>
#include <chrono>
#endif

#define SOUND_ID(name, handle) name + "/" + handle

#ifdef __ANDROID__

typedef struct _dsvorbis {
  const SAMPLE_DATATYPE* data;
  size_t size;
  size_t pos;
} dsvorbis;

size_t s3d_read_func(void* ptr, size_t size, size_t nmemb, void* datasource) {

  dsvorbis* ds = reinterpret_cast<dsvorbis*>(datasource);

  if (ds->pos + size * nmemb < ds->size) {
    memcpy(ptr, &ds->data[ds->pos], size * nmemb);
    ds->pos += size * nmemb;
    return nmemb;
  }
  else if (ds->pos < ds->size) {
    size_t elementsRead = (ds->size - ds->pos) / size;
    memcpy(ptr, &ds->data[ds->pos], elementsRead * size);
    ds->pos += elementsRead * size;
    return elementsRead;
  }
  else {
    return 0;
  }
}

int s3d_seek_func(void* datasource, ogg_int64_t offset, int whence) {

  dsvorbis* ds = reinterpret_cast<dsvorbis*>(datasource);

  switch (whence) {
  case SEEK_SET:
    if (offset <= ds->size && offset >= 0) {
      ds->pos = offset;
    }
    else {
      return 1;
    }
    break;
  case SEEK_CUR:
    if (ds->pos + offset >= 0 && ds->pos + offset <= ds->size) {
      ds->pos += offset;
    }
    else {
      return 1;
    }
    break;
  case SEEK_END:
    if (ds->size + offset >= 0 && ds->size + offset <= ds->size) {
      ds->pos = ds->size + offset;
    }
    else {
      return 1;
    }
    break;
  default:
    return 1;
  }
  return 0;
}

long s3d_tell_func(void* datasource) {
  dsvorbis* ds = reinterpret_cast<dsvorbis*>(datasource);
  return ds->pos;
}

static ov_callbacks OV_SMALL3D_ANDROID_MEMORY_NOCLOSE = {
  (size_t(*)(void*, size_t, size_t, void*))  s3d_read_func,
  (int (*)(void*, ogg_int64_t, int))           s3d_seek_func,
  (int (*)(void*))                             NULL,
  (long (*)(void*))                            s3d_tell_func

};

#endif

namespace small3d {

  bool Sound::noOutputDevice;
  unsigned int Sound::numInstances = 0;

#ifdef SMALL3D_IOS
  ALCdevice* Sound::openalDevice;
  ALCcontext* Sound::openalContext;
#endif

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
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
      }
    }

    soundData->currentFrame += framesPerBuffer;

    return result;
  }

#elif defined(__ANDROID__)

#endif

#ifdef __ANDROID__
  Sound::Sound() : audioCallbackObject(&this->soundData) {
#else
  Sound::Sound() {
#endif

#if !defined(SMALL3D_IOS)
    this->stream = nullptr;
#endif

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
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
#elif defined(__ANDROID__)
    streamBuilder = new oboe::AudioStreamBuilder();
    streamBuilder->setDirection(oboe::Direction::Output);
    streamBuilder->setPerformanceMode(oboe::PerformanceMode::LowLatency);
    streamBuilder->setChannelCount(oboe::ChannelCount::Mono);
    streamBuilder->setDataCallback(&audioCallbackObject);

#elif defined(SMALL3D_IOS)
    if (numInstances == 0) {
      openalDevice = alcOpenDevice(nullptr);
      if (!openalDevice) {
        throw std::runtime_error("Could not open OpenAL device.");
      }
      openalContext = alcCreateContext(openalDevice, nullptr);
      alcMakeContextCurrent(openalContext);

    }
#endif
    ++numInstances;
  }

  Sound::Sound(const std::string soundFilePath) : Sound() {

    this->load(getBasePath() + soundFilePath);

  }

  Sound::~Sound() {

#if !defined(SMALL3D_IOS)
    if (stream != nullptr) {
#else
    if (true) {
#endif
      stop();

#if defined(__ANDROID__)
      stream->close();
#elif defined(SMALL3D_IOS)
      alDeleteBuffers((ALuint)1, &openalBuffer);
      alDeleteSources((ALuint)1, &openalSource);
#else
      Pa_CloseStream(stream);
#endif

    }
    --numInstances;
    if (numInstances == 0) {

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
      //At times, this has caused crashes on MacOS
      Pa_Terminate();
#endif

#ifdef SMALL3D_IOS
      alcMakeContextCurrent(nullptr);
      alcDestroyContext(openalContext);
      alcCloseDevice(openalDevice);
#endif
    }
  }

  void Sound::load(const std::string soundFilePath) {

    if (!noOutputDevice) {

      try {

        OggVorbis_File vorbisFile;

#ifndef __ANDROID__

        FILE* fp = fopen((soundFilePath).c_str(), "rb");
        if (!fp) {
          throw std::runtime_error("Could not open file " + soundFilePath);
        }

        if (ov_open_callbacks((void*)fp, &vorbisFile, NULL, 0,
          OV_CALLBACKS_NOCLOSE) < 0) {
          fclose(fp);
          throw std::runtime_error("Could not read file " +
            soundFilePath + " as ogg.");
        }
        else {
          LOGDEBUG("Opened OV callbacks for " + soundFilePath + ".");
        }

#else
        AAsset* asset = AAssetManager_open(small3d_android_app->activity->assetManager,
          soundFilePath.c_str(),
          AASSET_MODE_STREAMING);
        if (!asset) {
          throw std::runtime_error(
            "Opening asset " + soundFilePath + " has failed!");
        }
        else {
          LOGDEBUG("Asset " + soundFilePath + " opened successfully.");
        }

        dsvorbis filedata = {};
        filedata.size = AAsset_getLength(asset);
        filedata.pos = 0;
        filedata.data = (const SAMPLE_DATATYPE*)AAsset_getBuffer(asset);

        LOGINFO("Asset length " + std::to_string(filedata.size));

        if (ov_open_callbacks(&filedata, &vorbisFile, NULL, 0,
          OV_SMALL3D_ANDROID_MEMORY_NOCLOSE) < 0) {
          AAsset_close(asset);
          throw std::runtime_error("Could not load sound from file " +
            soundFilePath);
        }
        else {
          LOGDEBUG("Opened OV callbacks for " + soundFilePath + ".");
        }

#endif

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
#ifndef __ANDROID__
            fclose(fp);
#else
            AAsset_close(asset);
#endif

            throw std::runtime_error("Error in sound stream.");

          }
          else if (ret > 0) {

            this->soundData.data.insert(soundData.data.end(), &pcmout[0],
              &pcmout[ret]);

            pos += ret;

          }
        } while (ret != 0);

        ov_clear(&vorbisFile);

#ifndef __ANDROID__
        fclose(fp);
#else
        AAsset_close(asset);
#endif


      }
      catch (std::exception& ex) {
        LOGDEBUG(ex.what());
        LOGDEBUG("Opening as binary...");

#ifdef __ANDROID__
        AAsset* asset = AAssetManager_open(small3d_android_app->activity->assetManager,
          soundFilePath.c_str(),
          AASSET_MODE_STREAMING);
        if (!asset) throw std::runtime_error("Opening asset " + soundFilePath +
          " has failed!");
        off_t assetLength;
        assetLength = AAsset_getLength(asset);
        const void* buffer = AAsset_getBuffer(asset);
        membuf sbuf((char*)buffer, (char*)buffer + sizeof(char) * assetLength);
        std::istream is(&sbuf);
        if (!is) {
          throw std::runtime_error("Could not open file " + soundFilePath);
        }
#else
        std::ifstream is;
        is.open(soundFilePath, std::ios::in | std::ios::binary);
        if (!is.is_open()) {
          throw std::runtime_error("Could not open file " + soundFilePath);
        }
#endif

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
#ifdef __ANDROID__
        AAsset_close(asset);
#else
        is.close();
#endif
        unsigned char out[CHUNK];
        uint32_t have = 0;
        z_stream strm;
        int flush;
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
          have = CHUNK - strm.avail_out;
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

#ifdef __ANDROID__
        AAsset_close(asset);
#else
        is.close();
#endif
        LOGDEBUG("Loaded sound from binary file " + soundFilePath);

      }

      LOGDEBUG("Loaded sound - channels " + std::to_string(this->soundData.channels) +
        " - rate " + std::to_string(this->soundData.rate) + " - samples " +
        std::to_string(this->soundData.samples) + " - size in bytes " +
        std::to_string(this->soundData.size) + " - duration " +
        std::to_string(this->soundData.duration) +
        + " - start time " + std::to_string(this->soundData.startTime) +
        + " - repeat? " + std::to_string(this->soundData.repeat) +
        + " - current frame " + std::to_string(this->soundData.currentFrame) +
        + " - data vector size " + std::to_string(this->soundData.data.size()) +
        + " - playing repeat? " + std::to_string(this->soundData.playingRepeat)
      );

    }

    this->openStream();

  }

  void Sound::openStream() {
    if (noOutputDevice) return;
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    PaStreamParameters outputParams;

    memset(&outputParams, 0, sizeof(PaStreamParameters));
    outputParams.device = defaultOutput;
    outputParams.channelCount = this->soundData.channels;
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
#elif defined(__ANDROID__)
    streamBuilder->setSampleRate(soundData.rate / SAMPLES_PER_FRAME);
    streamBuilder->setChannelCount(soundData.channels);
    streamBuilder->setFormat(oboe::AudioFormat::I16);
    streamBuilder->setSharingMode(oboe::SharingMode::Shared);
    streamBuilder->openStream(&stream);

#elif defined(SMALL3D_IOS)
    alGenSources((ALuint)1, &openalSource);
    alGenBuffers((ALuint)1, &openalBuffer);
    alBufferData(openalBuffer, AL_FORMAT_MONO16, (const ALvoid*)soundData.data.data(),
      (ALsizei)soundData.size, (ALsizei)soundData.rate);
    alSourcei(openalSource, AL_BUFFER, openalBuffer);
#endif
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

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
      if (Pa_IsStreamActive(stream)) return;

      PaError error;

      if (Pa_IsStreamStopped(stream) == 0) {
        error = Pa_AbortStream(stream);
        if (error != paNoError) {
          throw std::runtime_error("Failed to abort stream on play: " +
            std::string(Pa_GetErrorText(error)));
        }
      }

#elif defined(__ANDROID__)

      auto st = stream->getState();
      if (st == oboe::StreamState::Started || st == oboe::StreamState::Starting) {
        return;
      }

#elif defined(SMALL3D_IOS)
      int state = 0;
      alGetSourcei(openalSource, AL_SOURCE_STATE, &state);
      if (state == AL_PLAYING) return;
      alSourcei(openalSource, AL_LOOPING, repeat);
      alSourcePlay(openalSource);
#endif

      this->soundData.currentFrame = 0;
      this->soundData.startTime = 0;
      this->soundData.repeat = repeat;

#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
      error = Pa_StartStream(stream);
      if (error != paNoError) {
        throw std::runtime_error("Failed to start stream: " +
          std::string(Pa_GetErrorText(error)));
      }
#elif defined(__ANDROID__)
      if (stream->requestStart() != oboe::Result::OK) {
        LOGDEBUG("Failed to request start of sound stream.");
      }
#endif
    }
  }

  void Sound::stop() {
    if (soundData.repeat) {
      if (!soundData.playingRepeat) return;
      soundData.playingRepeat = false;
    }
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    if (Pa_IsStreamStopped(stream)) return;
#endif

#if !defined(SMALL3D_IOS)
    if (this->stream != nullptr) {
#else
    if (true) {
#endif

#if defined(__ANDROID__)

      auto st = stream->getState();
      if (st == oboe::StreamState::Started ||
        st == oboe::StreamState::Starting ||
        st == oboe::StreamState::Paused ||
        st == oboe::StreamState::Pausing) {
        stream->requestStop();
      }
#elif defined(SMALL3D_IOS)
      alSourceStop(openalSource);
#else
      Pa_AbortStream(stream);
#endif
      this->soundData.currentFrame = 0;
      this->soundData.startTime = 0;
    }
  }

  Sound::Sound(const Sound & other) noexcept : Sound() {
    this->soundData = other.soundData;

#if !defined(SMALL3D_IOS)
    this->stream = nullptr;
#endif
    this->openStream();
    ++numInstances;
  }

  Sound::Sound(const Sound && other) noexcept : Sound() {
    this->soundData = other.soundData;
#if !defined(SMALL3D_IOS)
    this->stream = nullptr;
#endif
    this->openStream();
    ++numInstances;
  }

  Sound& Sound::operator=(const Sound & other) noexcept {
#if  !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    if (this->stream != nullptr) {

      Pa_AbortStream(this->stream);
      Pa_CloseStream(this->stream);

    }
#endif

    this->soundData = other.soundData;

#if !defined(SMALL3D_IOS)
    this->stream = nullptr;
#endif

    this->openStream();
    return *this;
  }

  Sound& Sound::operator=(const Sound && other) noexcept {
#if !defined(SMALL3D_IOS)
    if (this->stream != nullptr) {
#else
    if (true) {
#endif
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
      Pa_AbortStream(this->stream);
      Pa_CloseStream(this->stream);
#endif
    }
    this->soundData = other.soundData;

#if !defined(SMALL3D_IOS)
    this->stream = nullptr;
#endif
    this->openStream();
    return *this;
  }

  void Sound::saveBinary(const std::string binaryFilePath) {

    const uint32_t CHUNK = 16384;

    std::stringstream ss(std::ios::out | std::ios::binary | std::ios::trunc);
    cereal::BinaryOutputArchive oarchive(ss);
    oarchive(soundData);

    unsigned char out[CHUNK];
    uint32_t have = 0;
    z_stream strm;
    int flush;

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
      have = CHUNK - strm.avail_out;
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
