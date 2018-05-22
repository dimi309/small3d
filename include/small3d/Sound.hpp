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
#include <portaudio.h>
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
    PaStream *stream;

    static bool noOutputDevice;
    static PaDeviceIndex defaultOutput;
    static unsigned int numInstances;

    static int audioCallback(const void *inputBuffer, void *outputBuffer,
			     unsigned long framesPerBuffer,
			     const PaStreamCallbackTimeInfo *timeInfo,
			     PaStreamCallbackFlags statusFlags,
			     void *userData);
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
