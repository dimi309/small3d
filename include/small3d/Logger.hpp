/**
 * @file Logger.hpp
 * @brief Logger used in small3d
 *
 * Created on: 2014/10/18
 *     Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

/**
 *  Log an error
 */
#define LOGERROR(MESSAGE) logger->append(LogLevel::loggererror, MESSAGE)

/**
 * Log information (shows up even when not debugging)
 */
#define LOGINFO(MESSAGE) logger->append(LogLevel::loggerinfo, MESSAGE)

/** 
 * Log debug information (only shows up when debugging)
 */
#if defined(DEBUG) || defined(_DEBUG) || !defined (NDEBUG)
#define LOGDEBUG(MESSAGE) logger->append(LogLevel::loggerdebug, MESSAGE)
#else
#define LOGDEBUG(MESSAGE)
#endif

#include <ostream>
#include <memory>

namespace small3d {

  /**
   * @brief Possible logging levels.
   */

  enum class LogLevel {
    loggerinfo, loggerdebug, loggererror
  };

  /**
   * @class Logger
   * @brief Used for logging through macros (LOGERROR, LOGDEBUG, LOGINFO)
   */

  class Logger {
  public:

    /**
     * @brief Constructor with stream for output.
     *
     */

    Logger();

    /**
     * @brief Destructor.
     */

    ~Logger();

    /**
     * @brief Appends a message to the logger.
     *
     * @param	level  	The logging level (debug, info, etc).
     * @param	message	The message.
     */

    void append(const LogLevel level, const std::string message) const;
  };

  /**
   * @brief Initialise the logger.
   */
  void initLogger();

  /**
   * @brief Destroy the logger.
   */
  void deleteLogger();
}

/**
 * @brief The logger object used by the logging macros.
 */
extern std::shared_ptr<small3d::Logger> logger;
