/**
 * @file Sound.hpp
 * @brief Header of the Sound class
 *
 *  Created on: 2017-07-17
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include <unordered_map>
#ifndef __ANDROID__
#include <portaudio.h>
#else
#include <aaudio/AAudio.h>
#endif
#include <vector>
#include <string>

// TODO: In addition to playing audio on PCs, this streams audio to android
//       but repeating a sound and concurrent sounds don't work. Stopping a
//       sound does not look that great either.

namespace small3d {

  /**
   * @class Sound
   *
   * @brief Class that loads and plays a sound from an ogg file.
   */
  class Sound {
  private:

    struct SoundData {
      int channels;
      int rate;
      long samples;
      long size;
      double duration;
      double startTime;
      bool repeat;
      unsigned long currentFrame;
      std::vector<char> data;
    };

    SoundData soundData;
#ifndef __ANDROID__
    PaStream *stream;
#else
    AAudioStreamBuilder *streamBuilder;
    AAudioStream *stream;
#endif

    static bool noOutputDevice;

    static unsigned int numInstances;
#ifndef __ANDROID__
    static PaDeviceIndex defaultOutput;
    static int audioCallback(const void *inputBuffer, void *outputBuffer,
			     unsigned long framesPerBuffer,
			     const PaStreamCallbackTimeInfo *timeInfo,
			     PaStreamCallbackFlags statusFlags,
			     void *userData);
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
     * @brief repeat Repeat the sound after it ends?
     */
    void play(const bool repeat=false);

    /**
     * @brief Stop playing the sound.
     */
    void stop();

    /**
     * @brief Copy constructor
     */
    Sound(const Sound& other);

    /**
     * @brief Move constructor
     */
    Sound(const Sound&& other);

    /**
     * @brief Copy assignment
     */
    Sound& operator=(const Sound& other);

    /** 
     * @brief Move assignment
     */
    Sound& operator=(const Sound&& other);
    
  };

}
