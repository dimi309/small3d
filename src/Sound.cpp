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

#if defined(__ANDROID__) || defined(SMALL3D_IOS)
#include "vkzos.h"
#include <sys/time.h>
#else
#include <GLFW/glfw3.h>
#endif

#ifdef SMALL3D_IOS
#include "interop.h"
#endif

#define WORD_SIZE 2
#define PORTAUDIO_SAMPLE_FORMAT paInt16

// todo: check if this is correct for iOS
#ifndef __ANDROID__
#define SAMPLE_DATATYPE short
#else
#define SAMPLE_DATATYPE uint8_t
#define SAMPLES_PER_FRAME 1
#endif

#define SOUND_ID(name, handle) name + "/" + handle

#ifdef __ANDROID__

typedef struct _dsvorbis {
  const SAMPLE_DATATYPE *data;
  size_t size;
  size_t pos;
} dsvorbis;

size_t s3d_read_func(void *ptr, size_t size, size_t nmemb, void *datasource) {

  dsvorbis *ds = reinterpret_cast<dsvorbis*>(datasource);

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

int s3d_seek_func(void *datasource, ogg_int64_t offset, int whence) {

  dsvorbis *ds = reinterpret_cast<dsvorbis*>(datasource);

  switch(whence) {
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

long s3d_tell_func(void *datasource) {
  dsvorbis *ds = reinterpret_cast<dsvorbis*>(datasource);
  return ds->pos;
}

static ov_callbacks OV_SMALL3D_ANDROID_MEMORY_NOCLOSE = {
  (size_t (*)(void *, size_t, size_t, void *))  s3d_read_func,
  (int (*)(void *, ogg_int64_t, int))           s3d_seek_func,
  (int (*)(void *))                             NULL,
  (long (*)(void *))                            s3d_tell_func

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

  int Sound::audioCallback(const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo *timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData) {
    
    int result = paContinue;
    SoundData *soundData = static_cast<SoundData *>(userData);

    if (soundData->startTime == 0) {
      soundData->startTime = glfwGetTime() - 0.1;
    } else if (glfwGetTime() - soundData->startTime > soundData->duration) {
      if (soundData->repeat) {
        soundData->startTime = glfwGetTime() - 0.1;
        soundData->currentFrame = 0;
      }
      else {
        return paAbort;
      }
    }
    
    SAMPLE_DATATYPE *out = static_cast<SAMPLE_DATATYPE *>(outputBuffer);
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
        *out++ = (reinterpret_cast<short *>(soundData->data.data()))[i + c];
      }
    }
    
    soundData->currentFrame += framesPerBuffer;
    
    return result;
  }

#elif defined(__ANDROID__)
   aaudio_data_callback_result_t Sound::audioCallback (
    AAudioStream *stream,
    void *userData,
    void *audioData,
    int32_t numFrames) {

     SoundData *soundData = static_cast<SoundData *>(userData);

     SAMPLE_DATATYPE *out = static_cast<SAMPLE_DATATYPE *>(audioData);

     if (soundData->currentFrame * SAMPLES_PER_FRAME >= soundData->samples) {
       if (soundData->repeat) {
         soundData->currentFrame = 0;
       } else {
         return AAUDIO_CALLBACK_RESULT_STOP;
       }
     }

     memcpy(out, &soundData->data.data()[WORD_SIZE * soundData->currentFrame *
                                         SAMPLES_PER_FRAME * soundData->channels],
            WORD_SIZE * numFrames * SAMPLES_PER_FRAME * soundData->channels);

     soundData->currentFrame += numFrames;

     return AAUDIO_CALLBACK_RESULT_CONTINUE;
  }
#endif


  Sound::Sound() {

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
    
      defaultOutput = Pa_GetDefaultOutputDevice();
    
      if (defaultOutput == paNoDevice) {
        LOGERROR("No default sound output device.");
        noOutputDevice = true;
      }
    }
#elif defined(__ANDROID__)
    if (AAudio_createStreamBuilder(&streamBuilder) != AAUDIO_OK) {
      LOGERROR("Failed to create stream builder.");
      noOutputDevice = true;
    } else {
      AAudioStreamBuilder_setDeviceId(streamBuilder, AAUDIO_UNSPECIFIED);
    }
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
#ifdef SMALL3D_IOS
    std::string basePath = get_base_path();
    basePath += "/";
    this->load(basePath + soundFilePath);
#else
    this->load(soundFilePath);
#endif
  }

  Sound::~Sound() {

#if !defined(SMALL3D_IOS)
    if (stream != nullptr) {
#else
    if (true) {
#endif
      stop();
      
#if defined(__ANDROID__)
      AAudioStream_close(stream);
      AAudioStreamBuilder_delete(streamBuilder);
#elif defined(SMALL3D_IOS)
      alDeleteBuffers((ALuint) 1, &openalBuffer);
      alDeleteSources((ALuint) 1, &openalSource);
#else
      Pa_CloseStream(stream);
#endif

    }
    --numInstances;
    if(numInstances == 0) {
      // This was causing a segmentation fault on MacOS when a Sound object was
      // declared as a global.
      //LOGDEBUG("Last Sound instance destroyed. Terminating PortAudio.");
      //Pa_Terminate();
#ifdef SMALL3D_IOS
      alcMakeContextCurrent(nullptr);
      alcDestroyContext(openalContext);
      alcCloseDevice(openalDevice);
#endif
    }
  }

  void Sound::load(const std::string soundFilePath) {
    
    if (!noOutputDevice) {
      
      OggVorbis_File vorbisFile;

#ifndef __ANDROID__

      FILE *fp = fopen((soundFilePath).c_str(), "rb");
      if (!fp) {
        throw std::runtime_error("Could not open file " + soundFilePath);
      }

      if (ov_open_callbacks((void *)fp, &vorbisFile, NULL, 0,
        OV_CALLBACKS_NOCLOSE) < 0) {
        throw std::runtime_error("Could not load sound from file " +
        soundFilePath);
      }
      else {
        LOGDEBUG("Opened OV callbacks for " + soundFilePath + ".");
      }

#else
      AAsset *asset = AAssetManager_open(vkz_android_app->activity->assetManager,
                                         soundFilePath.c_str(),
                                         AASSET_MODE_STREAMING);
      if(!asset) {
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

      LOGINFO("Asset length " + intToStr(filedata.size));

      if (ov_open_callbacks(&filedata, &vorbisFile, NULL, 0,
                            OV_SMALL3D_ANDROID_MEMORY_NOCLOSE) < 0) {
        throw std::runtime_error("Could not load sound from file " +
                                 soundFilePath);
      }
      else {
        LOGDEBUG("Opened OV callbacks for " + soundFilePath + ".");
      }

#endif

      vorbis_info *vi = ov_info(&vorbisFile, -1);
      
      this->soundData.channels = vi->channels;
      this->soundData.rate = (int) vi->rate;
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
          
          LOGERROR("Error in sound stream.");
          
        } else if (ret > 0) {
          
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
      
      char soundInfo[100];
      sprintf(soundInfo, "Loaded sound - channels %d - rate %d - samples %ld "
	      "- size in bytes %ld", this->soundData.channels,
	      this->soundData.rate, this->soundData.samples,
	      this->soundData.size);
      LOGDEBUG(std::string(soundInfo));
    }

    this->openStream();
    
  }

  void Sound::openStream() {
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
    AAudioStreamBuilder_setSampleRate(streamBuilder, soundData.rate / SAMPLES_PER_FRAME);
    AAudioStreamBuilder_setChannelCount(streamBuilder, soundData.channels);
    AAudioStreamBuilder_setFormat(streamBuilder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setSharingMode(streamBuilder, AAUDIO_SHARING_MODE_SHARED);
    AAudioStreamBuilder_setSamplesPerFrame(streamBuilder, SAMPLES_PER_FRAME);
    // Used to call AAudioStreamBuilder_setBufferCapacityInFrames but then I read that
    // it is not guaranteed that the capacity will be the one requested, so I left it
    // unspecified.
    AAudioStreamBuilder_setDataCallback(streamBuilder, Sound::audioCallback, &soundData);

    AAudioStreamBuilder_openStream(streamBuilder, &stream);
#elif defined(SMALL3D_IOS)
    alGenSources((ALuint)1, &openalSource);
    alGenBuffers((ALuint)1, &openalBuffer);
    alBufferData(openalBuffer, AL_FORMAT_MONO16, (const ALvoid *)soundData.data.data(),
                 (ALsizei)soundData.size, (ALsizei)soundData.rate);
    alSourcei(openalSource, AL_BUFFER, openalBuffer);
#endif
  }

  void Sound::play(const bool repeat) {
    if (!noOutputDevice && this->soundData.size > 0) {
      
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)

      PaError error;

      if (Pa_IsStreamStopped(stream) == 0) {
        error = Pa_AbortStream(stream);
        if (error != paNoError) {
          throw std::runtime_error("Failed to abort stream on play: " +
				   std::string(Pa_GetErrorText(error)));
        }
      }

#elif defined(__ANDROID__)
      aaudio_stream_state_t s = AAudioStream_getState(stream);
      if (s != AAUDIO_STREAM_STATE_STOPPED && s != AAUDIO_STREAM_STATE_STOPPING) {
        AAudioStream_requestStop(stream);
      }
#elif defined(SMALL3D_IOS)
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
      AAudioStream_requestStart(stream);
#endif
  }
}

  void Sound::stop() {
    
#if !defined(SMALL3D_IOS)
    if (this->stream != nullptr) {
#else
      if(true) {
#endif

#if defined(__ANDROID__)
      if (AAudioStream_getState(stream) == AAUDIO_STREAM_STATE_STARTED ||
          AAudioStream_getState(stream) == AAUDIO_STREAM_STATE_STARTING ||
          AAudioStream_getState(stream) == AAUDIO_STREAM_STATE_PAUSED ||
          AAudioStream_getState(stream) == AAUDIO_STREAM_STATE_PAUSING) {
        AAudioStream_requestStop(stream);
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

  Sound::Sound(const Sound& other) : Sound() {
    this->soundData = other.soundData;

#if !defined(SMALL3D_IOS)
    this->stream = nullptr;
#endif
    this->openStream();
    ++numInstances;
  }

  Sound::Sound(const Sound&& other) : Sound() {
    this->soundData = other.soundData;
#if !defined(SMALL3D_IOS)
    this->stream = nullptr;
#endif
    this->openStream();
    ++numInstances;
  }

  Sound& Sound::operator=(const Sound& other) {
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

  Sound& Sound::operator=(const Sound&& other) {
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
}
