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

#define WORD_SIZE 2

#if defined(__ANDROID__)
#define SAMPLE_DATATYPE uint8_t
#else
#define SAMPLE_DATATYPE short
#endif

#if defined(__ANDROID__)
#include <oboe/Oboe.h>
#define SAMPLES_PER_FRAME 1
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
   * @brief Class that loads and plays a sound from an ogg file,
   *        or a native sound file. In the latter case, the file
   *        loaded has to have been produced by converting an .ogg
   *        file to a small3d native binary file using the
   *        format converter program, s3dfc, produced by this project's
   *        build scripts, for example by running
   *        s3dfc engine.ogg engine.bin
   *        (this would convert an engine sound .ogg file to a native
   *        binary sound file)
   *
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

      template <class Archive>
      void serialize(Archive& archive) {
        archive(channels,
        rate,
        samples,
        size,
        duration,
        startTime,
        repeat,
        currentFrame,
        data,
        playingRepeat
        );
      }
    };

    SoundData soundData;
    
#if !defined(__ANDROID__) && !defined(SMALL3D_IOS)
    PaStream *stream;
#elif defined(__ANDROID__)
    oboe::AudioStreamBuilder *streamBuilder;
    oboe::AudioStream *stream;
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

    class AudioCallbackClass : public oboe::AudioStreamDataCallback {
    private:
      SoundData *soundData;
    public:
      AudioCallbackClass(SoundData *soundData) {
        this->soundData = soundData;
      }
      oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData,
                                            int32_t numFrames) {

        auto *out = static_cast<SAMPLE_DATATYPE*>(audioData);

        if (soundData->currentFrame * SAMPLES_PER_FRAME >= soundData->samples) {
          if (soundData->repeat) {
            soundData->currentFrame = 0;
          }
          else {
            // Always write something to the stream
            memset(out, 0, WORD_SIZE * numFrames * SAMPLES_PER_FRAME * soundData->channels);
            return oboe::DataCallbackResult::Stop;
          }
        }

        memcpy(out, &soundData->data.data()[WORD_SIZE * soundData->currentFrame *
                                            SAMPLES_PER_FRAME * soundData->channels],
               WORD_SIZE * numFrames * SAMPLES_PER_FRAME * soundData->channels);

        soundData->currentFrame += numFrames;

        return oboe::DataCallbackResult::Continue;
      }
    };

    AudioCallbackClass audioCallbackObject;

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
    explicit Sound(const std::string soundFilePath);

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
     * @brief Divide the volume (in order to lower it).
     * @param divisor Number to divide the volume by.
     */

    void divideVolume(uint32_t divisor);

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

    /**
     * @brief Save sound data in binary format
     * @param binaryFilePath Path of file to save binary data to.
     */
    void saveBinary(const std::string binaryFilePath);
    
  };

}
