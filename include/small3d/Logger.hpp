/**
 * @file Logger.hpp
 * @brief Header of the Logger class
 *
 * Created on: 2014/10/18
 *     Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

/**
 * Logging is accessed through macros so that it can be completely
 * omitted if deactivated.
 */

#define LOGERROR(MESSAGE) logger->append(loggererror, MESSAGE)

#define LOGINFO(MESSAGE) logger->append(loggerinfo, MESSAGE)

#if defined(DEBUG) || defined(_DEBUG) || !defined (NDEBUG)
#define LOGDEBUG(MESSAGE) logger->append(loggerdebug, MESSAGE)
#else
#define LOGDEBUG(MESSAGE)
#endif

#include <ostream>
#include <memory>

namespace small3d {

  std::string intToStr(const int number);

  /**
   * @brief Possible logging levels.
   */

  enum LogLevel {
    loggerinfo, loggerdebug, loggererror
  };

  /**
   * @class Logger
   * @brief The standard logging class for small3d.
   */

  class Logger {
  private:
    std::ostream *logStream;
  public:

    /**
     * @brief Constructor with stream for output.
     *
     * @param [in,out]	stream	The stream to which events will be logged.
     */

    Logger(std::ostream &stream);

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

  void initLogger();

  void initLogger(std::ostream &stream);

  void deleteLogger();
}

extern std::shared_ptr<small3d::Logger> logger;
