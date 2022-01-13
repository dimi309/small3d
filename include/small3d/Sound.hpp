/**
 * @file Sound.hpp
 * @brief A sound class
 *
 *  Created on: 2017-07-17
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include <unordered_map>

#if defined(__ANDROID__)
#include <aaudio/AAudio.h>
#elif defined(SMALL3D_IOS)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <portaudio.h>
#endif

#include <vector>
#include <string>

namespace small3d {

  /**
   * @class Sound
   *
   * @brief Class that loads and plays a sound from an ogg file.
   */
  class Sound {
    
  private:

    struct SoundData {
      int channels = 0;
      int rate = 0;
      long samples = 0;
      long size = 0;
      double duration = 0;
      double startTime = 0;
      bool repeat = false;
      unsigned long currentFrame = 0;
      std::vector<char> data;
      bool playingRepeat = false;
    };

    SoundData soundData;
    
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    PaStream *stream;
#elif defined(__ANDROID__)
    AAudioStreamBuilder *streamBuilder;
    AAudioStream *stream;
#elif defined(SMALL3D_IOS)
static ALCdevice *openalDevice;
static ALCcontext *openalContext;
    ALuint openalSource;
    ALuint openalBuffer;
#endif

    static bool noOutputDevice;

    static unsigned int numInstances;
    
#if !defined(SMALL3D_IOS) && !defined(__ANDROID__)
    static PaDeviceIndex defaultOutput;
    static int audioCallback(const void *inputBuffer, void *outputBuffer,
			     unsigned long framesPerBuffer,
			     const PaStreamCallbackTimeInfo *timeInfo,
			     PaStreamCallbackFlags statusFlags,
			     void *userData);
#elif defined(__ANDROID__)
    static aaudio_data_callback_result_t audioCallback (
      AAudioStream *stream,
      void *userData,
      void *audioData,
      int32_t numFrames);
    static void errorCallback(AAudioStream *stream, void *userData, aaudio_result_t error);
    static void asyncAndroidStopOnceFinished(AAudioStream *stream, long samples);
    static void asyncAndroidStopImmediately(AAudioStream *stream);

#endif

    void load(const std::string soundFilePath);
    void openStream();

  public:
    /**
     * @brief Default constructor
     */
    Sound();

    /**
     * @brief Ogg file loading constructor
     * @param soundFilePath The path to the ogg file from which to load the sound.
     */
    Sound(const std::string soundFilePath);

    /**
     * @brief Destructor
     */
    ~Sound();

    /**
     * @brief Play the sound.
     * @param repeat Repeat the sound after it ends?
     */
    void play(const bool repeat=false);

    /**
     * @brief Stop playing the sound.
     */
    void stop();

    /**
     * @brief Copy constructor
     */
    Sound(const Sound& other) noexcept;

    /**
     * @brief Move constructor
     */
    Sound(const Sound&& other) noexcept;

    /**
     * @brief Copy assignment
     */
    Sound& operator=(const Sound& other) noexcept;

    /** 
     * @brief Move assignment
     */
    Sound& operator=(const Sound&& other) noexcept;
    
  };

}
