#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

/*
 * WAV File Player with Pitch/Speed Control
 *
 * A comprehensive WAV file player supporting:
 * - RIFF/WAVE format parsing with robust error handling
 * - Multiple sample formats (8-bit, 16-bit, 24-bit, 32-bit int, 32-bit float)
 * - Independent pitch and speed manipulation via high-quality resampling
 * - Full audio reversal capabilities
 * - Memory-efficient streaming for large files
 * - Thread-safe methods for concurrent playback
 *
 * Design principles:
 * - Real-time safe playback (no allocations in audio path after loading)
 * - Numerically stable interpolation algorithms
 * - Clean error handling with detailed error codes
 * - Cache-efficient memory layout
 *
 * Usage:
 *   ShortwavDSP::WavPlayer player;
 *   auto result = player.loadFile("/path/to/file.wav");
 *   if (result != WavPlayer::Error::None) { handle error... }
 *
 *   player.setSampleRate(48000.0f);
 *   player.setSpeed(1.0f);      // 1.0 = normal speed
 *   player.setPitch(1.0f);      // 1.0 = original pitch
 *   player.setReverse(false);
 *   player.play();
 *
 *   // In audio callback:
 *   float sample = player.processSample();
 *   // or for buffers:
 *   player.processBuffer(output, numSamples);
 *
 * Threading:
 * - loadFile() / unload() are blocking and should be called from a non-audio thread
 * - All playback control methods (play/pause/stop/seek) are thread-safe
 * - Parameter setters are thread-safe (use atomic operations)
 * - processSample/processBuffer are lock-free and real-time safe
 *
 * Algorithm References:
 * - Cubic interpolation: https://www.musicdsp.org/en/latest/Other/49-cubic-interpollation.html
 * - Time-stretching concepts from phase vocoder literature
 */

namespace ShortwavDSP
{

  //------------------------------------------------------------------------------
  // Error codes for WAV operations
  //------------------------------------------------------------------------------

  enum class WavError : int
  {
    None = 0,              ///< No error
    FileNotFound = -1,     ///< File does not exist or cannot be opened
    InvalidFormat = -2,    ///< Not a valid RIFF/WAVE file
    UnsupportedFormat = -3, ///< Unsupported audio format (e.g., compressed)
    CorruptedData = -4,    ///< Data chunk is malformed or truncated
    OutOfMemory = -5,      ///< Memory allocation failed
    ReadError = -6,        ///< I/O error during file reading
    InvalidState = -7,     ///< Operation not valid in current state
    InvalidParameter = -8  ///< Invalid parameter value
  };

  //------------------------------------------------------------------------------
  // WAV file format structures
  //------------------------------------------------------------------------------

  namespace detail
  {
    // RIFF chunk header
    struct RiffHeader
    {
      char chunkId[4];     // "RIFF"
      uint32_t chunkSize;  // File size - 8
      char format[4];      // "WAVE"
    };

    // Format chunk (fmt )
    struct FmtChunk
    {
      char chunkId[4];       // "fmt "
      uint32_t chunkSize;    // Size of this chunk (16 for PCM, 18 or 40 for others)
      uint16_t audioFormat;  // 1 = PCM, 3 = IEEE float, etc.
      uint16_t numChannels;  // 1 = mono, 2 = stereo
      uint32_t sampleRate;   // Samples per second
      uint32_t byteRate;     // Average bytes per second
      uint16_t blockAlign;   // Bytes per sample frame
      uint16_t bitsPerSample; // Bits per sample (8, 16, 24, 32)
    };

    // Data chunk header
    struct DataChunk
    {
      char chunkId[4];     // "data"
      uint32_t chunkSize;  // Size of audio data in bytes
    };

    // Audio format codes
    constexpr uint16_t kWavFormatPCM = 1;
    constexpr uint16_t kWavFormatIEEEFloat = 3;
    constexpr uint16_t kWavFormatExtensible = 0xFFFE;

    // Utility functions
    inline bool memEqual4(const char* a, const char* b) noexcept
    {
      return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
    }

    inline float wavClamp(float x, float minVal, float maxVal) noexcept
    {
      return std::max(minVal, std::min(maxVal, x));
    }

    // Sample format conversion utilities
    inline float int8ToFloat(int8_t sample) noexcept
    {
      // 8-bit WAV is unsigned (0-255), center at 128
      return (static_cast<float>(sample) - 128.0f) / 128.0f;
    }

    inline float uint8ToFloat(uint8_t sample) noexcept
    {
      return (static_cast<float>(sample) - 128.0f) / 128.0f;
    }

    inline float int16ToFloat(int16_t sample) noexcept
    {
      return static_cast<float>(sample) / 32768.0f;
    }

    inline float int24ToFloat(const uint8_t* bytes) noexcept
    {
      // 24-bit is stored as 3 bytes, little-endian, signed
      int32_t value = static_cast<int32_t>(bytes[0]) |
                      (static_cast<int32_t>(bytes[1]) << 8) |
                      (static_cast<int32_t>(bytes[2]) << 16);
      // Sign extend from 24-bit to 32-bit
      if (value & 0x800000)
      {
        value |= 0xFF000000;
      }
      return static_cast<float>(value) / 8388608.0f; // 2^23
    }

    inline float int32ToFloat(int32_t sample) noexcept
    {
      return static_cast<float>(sample) / 2147483648.0f; // 2^31
    }

    // Cubic interpolation (Hermite spline)
    // Returns interpolated value at position t (0..1) between y1 and y2
    // y0, y1, y2, y3 are 4 consecutive samples
    inline float cubicInterpolate(float y0, float y1, float y2, float y3, float t) noexcept
    {
      const float t2 = t * t;
      const float t3 = t2 * t;

      // Hermite basis functions
      const float a0 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
      const float a1 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
      const float a2 = -0.5f * y0 + 0.5f * y2;
      const float a3 = y1;

      return a0 * t3 + a1 * t2 + a2 * t + a3;
    }

    // Linear interpolation (for lower quality / better performance)
    inline float linearInterpolate(float y0, float y1, float t) noexcept
    {
      return y0 + t * (y1 - y0);
    }

  } // namespace detail

  //------------------------------------------------------------------------------
  // Interpolation quality settings
  //------------------------------------------------------------------------------

  enum class InterpolationQuality
  {
    None,    ///< Nearest-neighbor (lowest quality, fastest)
    Linear,  ///< Linear interpolation (good balance)
    Cubic    ///< Cubic/Hermite interpolation (highest quality)
  };

  //------------------------------------------------------------------------------
  // Playback state
  //------------------------------------------------------------------------------

  enum class PlaybackState
  {
    Stopped,
    Playing,
    Paused
  };

  //------------------------------------------------------------------------------
  // Loop mode settings
  //------------------------------------------------------------------------------

  enum class LoopMode
  {
    Off,        ///< Play once and stop
    Forward,    ///< Loop from start to end
    PingPong    ///< Alternate forward and backward
  };

  //------------------------------------------------------------------------------
  // WavPlayer - Main player class
  //------------------------------------------------------------------------------

  class WavPlayer
  {
  public:
    //--------------------------------------------------------------------------
    // Construction / Destruction
    //--------------------------------------------------------------------------

    WavPlayer() noexcept
        : fileSampleRate_(44100),
          numChannels_(1),
          numSamples_(0),
          bitsPerSample_(16),
          outputSampleRate_(44100.0f),
          speed_(1.0f),
          pitch_(1.0f),
          volume_(1.0f),
          playbackPosition_(0.0),
          state_(PlaybackState::Stopped),
          loopMode_(LoopMode::Off),
          reverse_(false),
          pingPongDirection_(1),
          interpolation_(InterpolationQuality::Cubic)
    {
    }

    ~WavPlayer() = default;

    // Non-copyable (owns file data)
    WavPlayer(const WavPlayer&) = delete;
    WavPlayer& operator=(const WavPlayer&) = delete;

    // Move semantics
    WavPlayer(WavPlayer&& other) noexcept
    {
      std::lock_guard<std::mutex> lock(other.mutex_);
      audioData_ = std::move(other.audioData_);
      filePath_ = std::move(other.filePath_);
      outputSampleRate_.store(other.outputSampleRate_.load());
      speed_.store(other.speed_.load());
      pitch_.store(other.pitch_.load());
      volume_.store(other.volume_.load());
      playbackPosition_.store(other.playbackPosition_.load());
      state_.store(other.state_.load());
      loopMode_.store(other.loopMode_.load());
      reverse_.store(other.reverse_.load());
      pingPongDirection_.store(other.pingPongDirection_.load());
      interpolation_.store(other.interpolation_.load());
      fileSampleRate_ = other.fileSampleRate_;
      numChannels_ = other.numChannels_;
      numSamples_ = other.numSamples_;
      bitsPerSample_ = other.bitsPerSample_;
    }

    WavPlayer& operator=(WavPlayer&& other) noexcept
    {
      if (this != &other)
      {
        std::lock_guard<std::mutex> lockThis(mutex_);
        std::lock_guard<std::mutex> lockOther(other.mutex_);
        audioData_ = std::move(other.audioData_);
        filePath_ = std::move(other.filePath_);
        outputSampleRate_.store(other.outputSampleRate_.load());
        speed_.store(other.speed_.load());
        pitch_.store(other.pitch_.load());
        volume_.store(other.volume_.load());
        playbackPosition_.store(other.playbackPosition_.load());
        state_.store(other.state_.load());
        loopMode_.store(other.loopMode_.load());
        reverse_.store(other.reverse_.load());
        pingPongDirection_.store(other.pingPongDirection_.load());
        interpolation_.store(other.interpolation_.load());
        fileSampleRate_ = other.fileSampleRate_;
        numChannels_ = other.numChannels_;
        numSamples_ = other.numSamples_;
        bitsPerSample_ = other.bitsPerSample_;
      }
      return *this;
    }

    //--------------------------------------------------------------------------
    // File I/O
    //--------------------------------------------------------------------------

    /// Load a WAV file from the filesystem.
    /// This method is thread-safe but blocking - do not call from audio thread.
    ///
    /// @param path Path to the WAV file
    /// @return WavError::None on success, error code otherwise
    WavError loadFile(const char* path)
    {
      if (path == nullptr || path[0] == '\0')
      {
        return WavError::InvalidParameter;
      }

      std::lock_guard<std::mutex> lock(mutex_);

      // Open file
      FILE* file = std::fopen(path, "rb");
      if (file == nullptr)
      {
        return WavError::FileNotFound;
      }

      // Use RAII for file cleanup
      struct FileGuard
      {
        FILE* f;
        ~FileGuard() { if (f) std::fclose(f); }
      } guard{file};

      // Read and validate RIFF header
      detail::RiffHeader riffHeader;
      if (std::fread(&riffHeader, sizeof(riffHeader), 1, file) != 1)
      {
        return WavError::InvalidFormat;
      }

      if (!detail::memEqual4(riffHeader.chunkId, "RIFF") ||
          !detail::memEqual4(riffHeader.format, "WAVE"))
      {
        return WavError::InvalidFormat;
      }

      // Parse chunks to find fmt and data
      detail::FmtChunk fmtChunk{};
      bool foundFmt = false;
      bool foundData = false;
      uint32_t dataSize = 0;
      long dataOffset = 0;

      while (!foundFmt || !foundData)
      {
        char chunkId[4];
        uint32_t chunkSize;

        if (std::fread(chunkId, 4, 1, file) != 1 ||
            std::fread(&chunkSize, 4, 1, file) != 1)
        {
          if (foundFmt && foundData)
            break;
          return WavError::CorruptedData;
        }

        if (detail::memEqual4(chunkId, "fmt "))
        {
          // Read format chunk
          if (chunkSize < 16)
          {
            return WavError::InvalidFormat;
          }

          // Read the basic format info (16 bytes)
          if (std::fread(&fmtChunk.audioFormat, 2, 1, file) != 1 ||
              std::fread(&fmtChunk.numChannels, 2, 1, file) != 1 ||
              std::fread(&fmtChunk.sampleRate, 4, 1, file) != 1 ||
              std::fread(&fmtChunk.byteRate, 4, 1, file) != 1 ||
              std::fread(&fmtChunk.blockAlign, 2, 1, file) != 1 ||
              std::fread(&fmtChunk.bitsPerSample, 2, 1, file) != 1)
          {
            return WavError::ReadError;
          }

          // Skip any extra format bytes
          if (chunkSize > 16)
          {
            std::fseek(file, static_cast<long>(chunkSize - 16), SEEK_CUR);
          }

          foundFmt = true;
        }
        else if (detail::memEqual4(chunkId, "data"))
        {
          dataSize = chunkSize;
          dataOffset = std::ftell(file);
          foundData = true;
          // Don't skip past data chunk - we'll read it below
          break;
        }
        else
        {
          // Skip unknown chunk (ensure even alignment)
          uint32_t skipSize = chunkSize + (chunkSize & 1);
          std::fseek(file, static_cast<long>(skipSize), SEEK_CUR);
        }
      }

      if (!foundFmt || !foundData)
      {
        return WavError::InvalidFormat;
      }

      // Validate format
      if (fmtChunk.audioFormat != detail::kWavFormatPCM &&
          fmtChunk.audioFormat != detail::kWavFormatIEEEFloat)
      {
        // Check for extensible format with PCM/float subformat
        if (fmtChunk.audioFormat != detail::kWavFormatExtensible)
        {
          return WavError::UnsupportedFormat;
        }
      }

      if (fmtChunk.numChannels == 0 || fmtChunk.numChannels > 2)
      {
        return WavError::UnsupportedFormat;
      }

      if (fmtChunk.bitsPerSample != 8 && fmtChunk.bitsPerSample != 16 &&
          fmtChunk.bitsPerSample != 24 && fmtChunk.bitsPerSample != 32)
      {
        return WavError::UnsupportedFormat;
      }

      // Calculate number of samples
      const uint32_t bytesPerSample = fmtChunk.bitsPerSample / 8;
      const uint32_t bytesPerFrame = bytesPerSample * fmtChunk.numChannels;
      const size_t numFrames = dataSize / bytesPerFrame;

      if (numFrames == 0)
      {
        return WavError::CorruptedData;
      }

      // Allocate audio buffer (convert to float, interleaved)
      std::vector<float> newData;
      try
      {
        newData.resize(numFrames * fmtChunk.numChannels);
      }
      catch (const std::bad_alloc&)
      {
        return WavError::OutOfMemory;
      }

      // Seek to data start
      std::fseek(file, dataOffset, SEEK_SET);

      // Read and convert samples
      WavError readResult = readAndConvertSamples(
          file, newData.data(), numFrames,
          fmtChunk.numChannels, fmtChunk.bitsPerSample,
          fmtChunk.audioFormat == detail::kWavFormatIEEEFloat);

      if (readResult != WavError::None)
      {
        return readResult;
      }

      // Store file info
      audioData_ = std::move(newData);
      filePath_ = path;
      fileSampleRate_ = fmtChunk.sampleRate;
      numChannels_ = fmtChunk.numChannels;
      numSamples_ = numFrames;
      bitsPerSample_ = fmtChunk.bitsPerSample;

      // Reset playback position
      playbackPosition_.store(0.0);
      state_.store(PlaybackState::Stopped);
      pingPongDirection_.store(1);

      return WavError::None;
    }

    /// Load WAV data from a memory buffer.
    /// @param data Pointer to WAV file data in memory
    /// @param size Size of the data buffer in bytes
    /// @return WavError::None on success, error code otherwise
    WavError loadFromMemory(const uint8_t* data, size_t size)
    {
      if (data == nullptr || size < sizeof(detail::RiffHeader) + 8)
      {
        return WavError::InvalidParameter;
      }

      std::lock_guard<std::mutex> lock(mutex_);

      size_t offset = 0;

      // Read RIFF header
      detail::RiffHeader riffHeader;
      std::memcpy(&riffHeader, data + offset, sizeof(riffHeader));
      offset += sizeof(riffHeader);

      if (!detail::memEqual4(riffHeader.chunkId, "RIFF") ||
          !detail::memEqual4(riffHeader.format, "WAVE"))
      {
        return WavError::InvalidFormat;
      }

      // Parse chunks
      detail::FmtChunk fmtChunk{};
      bool foundFmt = false;
      bool foundData = false;
      uint32_t dataSize = 0;
      size_t dataOffset = 0;

      while (offset + 8 <= size && (!foundFmt || !foundData))
      {
        char chunkId[4];
        uint32_t chunkSize;

        std::memcpy(chunkId, data + offset, 4);
        offset += 4;
        std::memcpy(&chunkSize, data + offset, 4);
        offset += 4;

        if (detail::memEqual4(chunkId, "fmt "))
        {
          if (offset + 16 > size)
          {
            return WavError::CorruptedData;
          }

          std::memcpy(&fmtChunk.audioFormat, data + offset, 2);
          std::memcpy(&fmtChunk.numChannels, data + offset + 2, 2);
          std::memcpy(&fmtChunk.sampleRate, data + offset + 4, 4);
          std::memcpy(&fmtChunk.byteRate, data + offset + 8, 4);
          std::memcpy(&fmtChunk.blockAlign, data + offset + 12, 2);
          std::memcpy(&fmtChunk.bitsPerSample, data + offset + 14, 2);

          offset += chunkSize + (chunkSize & 1);
          foundFmt = true;
        }
        else if (detail::memEqual4(chunkId, "data"))
        {
          dataSize = chunkSize;
          dataOffset = offset;
          foundData = true;
          break;
        }
        else
        {
          offset += chunkSize + (chunkSize & 1);
        }
      }

      if (!foundFmt || !foundData)
      {
        return WavError::InvalidFormat;
      }

      // Validate format
      if (fmtChunk.audioFormat != detail::kWavFormatPCM &&
          fmtChunk.audioFormat != detail::kWavFormatIEEEFloat)
      {
        return WavError::UnsupportedFormat;
      }

      if (fmtChunk.numChannels == 0 || fmtChunk.numChannels > 2)
      {
        return WavError::UnsupportedFormat;
      }

      // Calculate frames
      const uint32_t bytesPerSample = fmtChunk.bitsPerSample / 8;
      const uint32_t bytesPerFrame = bytesPerSample * fmtChunk.numChannels;
      const size_t numFrames = dataSize / bytesPerFrame;

      if (numFrames == 0 || dataOffset + dataSize > size)
      {
        return WavError::CorruptedData;
      }

      // Allocate and convert
      std::vector<float> newData;
      try
      {
        newData.resize(numFrames * fmtChunk.numChannels);
      }
      catch (const std::bad_alloc&)
      {
        return WavError::OutOfMemory;
      }

      // Convert samples from memory
      WavError convertResult = convertSamplesFromMemory(
          data + dataOffset, newData.data(), numFrames,
          fmtChunk.numChannels, fmtChunk.bitsPerSample,
          fmtChunk.audioFormat == detail::kWavFormatIEEEFloat);

      if (convertResult != WavError::None)
      {
        return convertResult;
      }

      // Store file info
      audioData_ = std::move(newData);
      filePath_.clear();
      fileSampleRate_ = fmtChunk.sampleRate;
      numChannels_ = fmtChunk.numChannels;
      numSamples_ = numFrames;
      bitsPerSample_ = fmtChunk.bitsPerSample;

      playbackPosition_.store(0.0);
      state_.store(PlaybackState::Stopped);
      pingPongDirection_.store(1);

      return WavError::None;
    }

    /// Unload the current file and free memory.
    void unload()
    {
      std::lock_guard<std::mutex> lock(mutex_);
      audioData_.clear();
      audioData_.shrink_to_fit();
      filePath_.clear();
      numSamples_ = 0;
      numChannels_ = 1;
      playbackPosition_.store(0.0);
      state_.store(PlaybackState::Stopped);
    }

    /// Check if a file is currently loaded.
    bool isLoaded() const noexcept
    {
      return numSamples_ > 0 && !audioData_.empty();
    }

    //--------------------------------------------------------------------------
    // Playback Control (Thread-safe)
    //--------------------------------------------------------------------------

    /// Start or resume playback.
    void play() noexcept
    {
      if (isLoaded())
      {
        state_.store(PlaybackState::Playing);
      }
    }

    /// Pause playback (maintains position).
    void pause() noexcept
    {
      state_.store(PlaybackState::Paused);
    }

    /// Stop playback and reset position to start.
    void stop() noexcept
    {
      state_.store(PlaybackState::Stopped);
      playbackPosition_.store(reverse_.load() ? static_cast<double>(numSamples_ - 1) : 0.0);
      pingPongDirection_.store(reverse_.load() ? -1 : 1);
    }

    /// Seek to a specific position (0.0 = start, 1.0 = end).
    void seek(float normalizedPosition) noexcept
    {
      if (numSamples_ > 0)
      {
        const float clamped = detail::wavClamp(normalizedPosition, 0.0f, 1.0f);
        playbackPosition_.store(clamped * static_cast<double>(numSamples_ - 1));
      }
    }

    /// Seek to a specific sample position.
    void seekToSample(size_t sampleIndex) noexcept
    {
      if (numSamples_ > 0)
      {
        playbackPosition_.store(static_cast<double>(std::min(sampleIndex, numSamples_ - 1)));
      }
    }

    /// Get current playback state.
    PlaybackState getState() const noexcept
    {
      return state_.load();
    }

    /// Check if currently playing.
    bool isPlaying() const noexcept
    {
      return state_.load() == PlaybackState::Playing;
    }

    /// Get current playback position (0.0 to 1.0).
    float getPlaybackPosition() const noexcept
    {
      if (numSamples_ == 0)
        return 0.0f;
      return static_cast<float>(playbackPosition_.load() / static_cast<double>(numSamples_ - 1));
    }

    /// Get current playback position in samples.
    double getPlaybackPositionSamples() const noexcept
    {
      return playbackPosition_.load();
    }

    //--------------------------------------------------------------------------
    // Parameter Setters (Thread-safe, atomic)
    //--------------------------------------------------------------------------

    /// Set the output sample rate (host sample rate).
    void setSampleRate(float sampleRate) noexcept
    {
      outputSampleRate_.store(std::max(1.0f, sampleRate));
    }

    /// Set playback speed (1.0 = normal, 0.5 = half speed, 2.0 = double).
    /// Speed affects both tempo and duration, not pitch when combined with pitch.
    void setSpeed(float speed) noexcept
    {
      speed_.store(detail::wavClamp(speed, 0.01f, 100.0f));
    }

    /// Set pitch ratio (1.0 = original, 0.5 = octave down, 2.0 = octave up).
    /// Pitch affects playback rate but is combined with speed for final rate.
    void setPitch(float pitch) noexcept
    {
      pitch_.store(detail::wavClamp(pitch, 0.01f, 100.0f));
    }

    /// Set playback volume (0.0 = silence, 1.0 = unity).
    void setVolume(float volume) noexcept
    {
      volume_.store(detail::wavClamp(volume, 0.0f, 10.0f));
    }

    /// Set reverse playback mode.
    void setReverse(bool reverse) noexcept
    {
      const bool wasReverse = reverse_.load();
      reverse_.store(reverse);

      // If changing direction while stopped, reset position
      if (wasReverse != reverse && state_.load() == PlaybackState::Stopped)
      {
        playbackPosition_.store(reverse ? static_cast<double>(numSamples_ - 1) : 0.0);
        pingPongDirection_.store(reverse ? -1 : 1);
      }
    }

    /// Set loop mode.
    void setLoopMode(LoopMode mode) noexcept
    {
      loopMode_.store(mode);
    }

    /// Set interpolation quality.
    void setInterpolationQuality(InterpolationQuality quality) noexcept
    {
      interpolation_.store(quality);
    }

    //--------------------------------------------------------------------------
    // Parameter Getters
    //--------------------------------------------------------------------------

    float getSampleRate() const noexcept { return outputSampleRate_.load(); }
    float getSpeed() const noexcept { return speed_.load(); }
    float getPitch() const noexcept { return pitch_.load(); }
    float getVolume() const noexcept { return volume_.load(); }
    bool getReverse() const noexcept { return reverse_.load(); }
    LoopMode getLoopMode() const noexcept { return loopMode_.load(); }
    InterpolationQuality getInterpolationQuality() const noexcept { return interpolation_.load(); }

    //--------------------------------------------------------------------------
    // File Information Getters
    //--------------------------------------------------------------------------

    uint32_t getFileSampleRate() const noexcept { return fileSampleRate_; }
    uint16_t getNumChannels() const noexcept { return numChannels_; }
    size_t getNumSamples() const noexcept { return numSamples_; }
    uint16_t getBitsPerSample() const noexcept { return bitsPerSample_; }

    /// Get total duration in seconds.
    float getDurationSeconds() const noexcept
    {
      if (fileSampleRate_ == 0 || numSamples_ == 0)
        return 0.0f;
      return static_cast<float>(numSamples_) / static_cast<float>(fileSampleRate_);
    }

    /// Get the file path (empty if loaded from memory).
    const std::string& getFilePath() const noexcept { return filePath_; }

    //--------------------------------------------------------------------------
    // Audio Processing (Real-time safe, lock-free)
    //--------------------------------------------------------------------------

    /// Process and return a single mono sample.
    /// For stereo files, channels are mixed to mono.
    /// This is real-time safe and lock-free.
    float processSample() noexcept
    {
      if (!isLoaded() || state_.load() != PlaybackState::Playing)
      {
        return 0.0f;
      }

      const float sample = readInterpolatedSample();

      // Advance playback position
      advancePosition();

      return sample * volume_.load();
    }

    /// Process and return a stereo sample pair.
    /// For mono files, the same sample is returned for both channels.
    void processSampleStereo(float& left, float& right) noexcept
    {
      if (!isLoaded() || state_.load() != PlaybackState::Playing)
      {
        left = right = 0.0f;
        return;
      }

      readInterpolatedSampleStereo(left, right);

      const float vol = volume_.load();
      left *= vol;
      right *= vol;

      advancePosition();
    }

    /// Process a buffer of mono samples.
    void processBuffer(float* output, size_t numSamples) noexcept
    {
      if (output == nullptr)
        return;

      for (size_t i = 0; i < numSamples; ++i)
      {
        output[i] = processSample();
      }
    }

    /// Process a buffer of stereo samples (interleaved L/R).
    void processBufferStereo(float* output, size_t numFrames) noexcept
    {
      if (output == nullptr)
        return;

      for (size_t i = 0; i < numFrames; ++i)
      {
        processSampleStereo(output[i * 2], output[i * 2 + 1]);
      }
    }

    /// Process stereo buffers (separate L/R).
    void processBufferStereoSplit(float* left, float* right, size_t numFrames) noexcept
    {
      if (left == nullptr || right == nullptr)
        return;

      for (size_t i = 0; i < numFrames; ++i)
      {
        processSampleStereo(left[i], right[i]);
      }
    }

    //--------------------------------------------------------------------------
    // Direct sample access (for advanced use cases)
    //--------------------------------------------------------------------------

    /// Get a raw sample from the loaded data.
    /// @param frameIndex Sample frame index (0 to numSamples-1)
    /// @param channel Channel index (0 for mono/left, 1 for right in stereo)
    /// @return Sample value, or 0.0f if out of bounds
    float getRawSample(size_t frameIndex, size_t channel = 0) const noexcept
    {
      if (frameIndex >= numSamples_ || channel >= numChannels_)
      {
        return 0.0f;
      }
      return audioData_[frameIndex * numChannels_ + channel];
    }

    /// Get read-only access to the audio data buffer.
    const float* getAudioData() const noexcept
    {
      return audioData_.data();
    }

    /// Get the size of the audio data buffer in floats.
    size_t getAudioDataSize() const noexcept
    {
      return audioData_.size();
    }

  private:
    //--------------------------------------------------------------------------
    // Internal helper methods
    //--------------------------------------------------------------------------

    /// Read and convert samples from file to float buffer.
    WavError readAndConvertSamples(FILE* file, float* output, size_t numFrames,
                                    uint16_t channels, uint16_t bits, bool isFloat)
    {
      const size_t bytesPerSample = bits / 8;
      const size_t bytesPerFrame = bytesPerSample * channels;

      // Use a reasonably sized temporary buffer for reading
      constexpr size_t kReadBufferFrames = 4096;
      std::vector<uint8_t> readBuffer;
      try
      {
        readBuffer.resize(kReadBufferFrames * bytesPerFrame);
      }
      catch (const std::bad_alloc&)
      {
        return WavError::OutOfMemory;
      }

      size_t framesRead = 0;
      while (framesRead < numFrames)
      {
        const size_t framesToRead = std::min(kReadBufferFrames, numFrames - framesRead);
        const size_t bytesToRead = framesToRead * bytesPerFrame;

        if (std::fread(readBuffer.data(), 1, bytesToRead, file) != bytesToRead)
        {
          return WavError::ReadError;
        }

        // Convert samples
        for (size_t f = 0; f < framesToRead; ++f)
        {
          for (uint16_t c = 0; c < channels; ++c)
          {
            const size_t byteOffset = (f * channels + c) * bytesPerSample;
            const size_t outIndex = (framesRead + f) * channels + c;

            if (isFloat && bits == 32)
            {
              float value;
              std::memcpy(&value, readBuffer.data() + byteOffset, 4);
              output[outIndex] = value;
            }
            else
            {
              output[outIndex] = convertSample(readBuffer.data() + byteOffset, bits);
            }
          }
        }

        framesRead += framesToRead;
      }

      return WavError::None;
    }

    /// Convert samples from memory buffer.
    WavError convertSamplesFromMemory(const uint8_t* data, float* output, size_t numFrames,
                                       uint16_t channels, uint16_t bits, bool isFloat)
    {
      const size_t bytesPerSample = bits / 8;

      for (size_t f = 0; f < numFrames; ++f)
      {
        for (uint16_t c = 0; c < channels; ++c)
        {
          const size_t byteOffset = (f * channels + c) * bytesPerSample;
          const size_t outIndex = f * channels + c;

          if (isFloat && bits == 32)
          {
            float value;
            std::memcpy(&value, data + byteOffset, 4);
            output[outIndex] = value;
          }
          else
          {
            output[outIndex] = convertSample(data + byteOffset, bits);
          }
        }
      }

      return WavError::None;
    }

    /// Convert a single sample from raw bytes to float.
    static float convertSample(const uint8_t* bytes, uint16_t bits) noexcept
    {
      switch (bits)
      {
      case 8:
        return detail::uint8ToFloat(bytes[0]);
      case 16:
      {
        int16_t value;
        std::memcpy(&value, bytes, 2);
        return detail::int16ToFloat(value);
      }
      case 24:
        return detail::int24ToFloat(bytes);
      case 32:
      {
        int32_t value;
        std::memcpy(&value, bytes, 4);
        return detail::int32ToFloat(value);
      }
      default:
        return 0.0f;
      }
    }

    /// Calculate the effective playback rate considering pitch, speed, and sample rate conversion.
    float getEffectivePlaybackRate() const noexcept
    {
      const float outRate = outputSampleRate_.load();
      const float fileRate = static_cast<float>(fileSampleRate_);
      const float rateRatio = (outRate > 0.0f) ? (fileRate / outRate) : 1.0f;
      return rateRatio * speed_.load() * pitch_.load();
    }

    /// Read an interpolated mono sample at the current position.
    float readInterpolatedSample() noexcept
    {
      const double pos = playbackPosition_.load();
      const size_t idx = static_cast<size_t>(pos);
      const float frac = static_cast<float>(pos - static_cast<double>(idx));

      if (numChannels_ == 1)
      {
        return interpolateMono(idx, frac);
      }
      else
      {
        // Mix stereo to mono
        float left, right;
        interpolateStereo(idx, frac, left, right);
        return (left + right) * 0.5f;
      }
    }

    /// Read interpolated stereo samples at the current position.
    void readInterpolatedSampleStereo(float& left, float& right) noexcept
    {
      const double pos = playbackPosition_.load();
      const size_t idx = static_cast<size_t>(pos);
      const float frac = static_cast<float>(pos - static_cast<double>(idx));

      if (numChannels_ == 1)
      {
        // Duplicate mono to stereo
        left = right = interpolateMono(idx, frac);
      }
      else
      {
        interpolateStereo(idx, frac, left, right);
      }
    }

    /// Interpolate a mono sample.
    float interpolateMono(size_t idx, float frac) const noexcept
    {
      const InterpolationQuality quality = interpolation_.load();

      if (quality == InterpolationQuality::None)
      {
        return getSampleSafe(idx, 0);
      }
      else if (quality == InterpolationQuality::Linear)
      {
        const float y0 = getSampleSafe(idx, 0);
        const float y1 = getSampleSafe(idx + 1, 0);
        return detail::linearInterpolate(y0, y1, frac);
      }
      else // Cubic
      {
        const float y0 = getSampleSafe(idx > 0 ? idx - 1 : 0, 0);
        const float y1 = getSampleSafe(idx, 0);
        const float y2 = getSampleSafe(idx + 1, 0);
        const float y3 = getSampleSafe(idx + 2, 0);
        return detail::cubicInterpolate(y0, y1, y2, y3, frac);
      }
    }

    /// Interpolate stereo samples.
    void interpolateStereo(size_t idx, float frac, float& left, float& right) const noexcept
    {
      const InterpolationQuality quality = interpolation_.load();

      if (quality == InterpolationQuality::None)
      {
        left = getSampleSafe(idx, 0);
        right = getSampleSafe(idx, 1);
      }
      else if (quality == InterpolationQuality::Linear)
      {
        left = detail::linearInterpolate(getSampleSafe(idx, 0), getSampleSafe(idx + 1, 0), frac);
        right = detail::linearInterpolate(getSampleSafe(idx, 1), getSampleSafe(idx + 1, 1), frac);
      }
      else // Cubic
      {
        const size_t i0 = idx > 0 ? idx - 1 : 0;
        left = detail::cubicInterpolate(
            getSampleSafe(i0, 0), getSampleSafe(idx, 0),
            getSampleSafe(idx + 1, 0), getSampleSafe(idx + 2, 0), frac);
        right = detail::cubicInterpolate(
            getSampleSafe(i0, 1), getSampleSafe(idx, 1),
            getSampleSafe(idx + 1, 1), getSampleSafe(idx + 2, 1), frac);
      }
    }

    /// Get a sample with bounds checking.
    float getSampleSafe(size_t frameIdx, size_t channel) const noexcept
    {
      if (frameIdx >= numSamples_)
      {
        frameIdx = numSamples_ - 1;
      }
      return audioData_[frameIdx * numChannels_ + channel];
    }

    /// Advance playback position based on speed/pitch/direction.
    void advancePosition() noexcept
    {
      const float rate = getEffectivePlaybackRate();
      const LoopMode loop = loopMode_.load();
      const bool rev = reverse_.load();
      int direction = pingPongDirection_.load();

      // Apply direction
      double delta = static_cast<double>(rate);
      if (rev)
      {
        delta = -delta;
      }
      if (loop == LoopMode::PingPong)
      {
        delta *= direction;
      }

      double pos = playbackPosition_.load() + delta;

      // Handle boundaries
      if (loop == LoopMode::Off)
      {
        if (pos < 0.0 || pos >= static_cast<double>(numSamples_))
        {
          state_.store(PlaybackState::Stopped);
          pos = detail::wavClamp(static_cast<float>(pos), 0.0f, static_cast<float>(numSamples_ - 1));
        }
      }
      else if (loop == LoopMode::Forward)
      {
        const double len = static_cast<double>(numSamples_);
        if (rev)
        {
          while (pos < 0.0)
            pos += len;
        }
        else
        {
          while (pos >= len)
            pos -= len;
        }
      }
      else if (loop == LoopMode::PingPong)
      {
        const double maxPos = static_cast<double>(numSamples_ - 1);
        if (pos < 0.0)
        {
          pos = -pos;
          pingPongDirection_.store(-direction);
        }
        else if (pos > maxPos)
        {
          pos = maxPos - (pos - maxPos);
          pingPongDirection_.store(-direction);
        }
      }

      playbackPosition_.store(pos);
    }

    //--------------------------------------------------------------------------
    // Member variables
    //--------------------------------------------------------------------------

    // Audio data (float, interleaved channels)
    std::vector<float> audioData_;

    // File information
    std::string filePath_;
    uint32_t fileSampleRate_;
    uint16_t numChannels_;
    size_t numSamples_;
    uint16_t bitsPerSample_;

    // Playback parameters (atomic for thread-safe access)
    std::atomic<float> outputSampleRate_;
    std::atomic<float> speed_;
    std::atomic<float> pitch_;
    std::atomic<float> volume_;
    std::atomic<double> playbackPosition_;
    std::atomic<PlaybackState> state_;
    std::atomic<LoopMode> loopMode_;
    std::atomic<bool> reverse_;
    std::atomic<int> pingPongDirection_;
    std::atomic<InterpolationQuality> interpolation_;

    // Mutex for file operations (not used in audio path)
    mutable std::mutex mutex_;
  };

  //------------------------------------------------------------------------------
  // Utility: Error message conversion
  //------------------------------------------------------------------------------

  inline const char* wavErrorToString(WavError error) noexcept
  {
    switch (error)
    {
    case WavError::None:
      return "No error";
    case WavError::FileNotFound:
      return "File not found or cannot be opened";
    case WavError::InvalidFormat:
      return "Invalid RIFF/WAVE format";
    case WavError::UnsupportedFormat:
      return "Unsupported audio format";
    case WavError::CorruptedData:
      return "Corrupted or truncated data";
    case WavError::OutOfMemory:
      return "Memory allocation failed";
    case WavError::ReadError:
      return "I/O error during file reading";
    case WavError::InvalidState:
      return "Invalid operation for current state";
    case WavError::InvalidParameter:
      return "Invalid parameter value";
    default:
      return "Unknown error";
    }
  }

} // namespace ShortwavDSP
