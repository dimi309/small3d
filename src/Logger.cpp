/*
 *  Logger.cpp
 *
 *  Created on: 2014/10/18
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#include "Logger.hpp"
#include <sstream>
#include <ctime>
#include <iostream>

#ifdef __ANDROID__
#include <android/log.h>
#endif

std::shared_ptr<small3d::Logger> logger;

namespace small3d {

  Logger::Logger() {
    this->append(LogLevel::loggerinfo, "Logger created");
  }

  Logger::~Logger() {
    this->append(LogLevel::loggerinfo, "Logger getting destroyed");
  }

  void Logger::append(const LogLevel level, const std::string message) const {
      if (!logger) return;


#ifdef __ANDROID__

      android_LogPriority lp;

      switch (level) {
        case loggerinfo:
          lp = ANDROID_LOG_INFO;
              break;
        case loggerdebug:
          lp = ANDROID_LOG_DEBUG;
              break;
        case loggererror:
          lp = ANDROID_LOG_ERROR;
              break;
        default:
          lp = ANDROID_LOG_UNKNOWN;
              break;
      }

      __android_log_write(lp, "small3d logger", message.c_str());
    
#else
      std::ostringstream dateTimeOstringstream;

      time_t now;

      time(&now);

      tm *t = localtime(&now);

      char buf[20];

      strftime(buf, 20,"%Y-%m-%d %H:%M:%S", t);

      dateTimeOstringstream << buf;

      std::string indicator;
      switch (level) {
      case LogLevel::loggerinfo:
        indicator = "INFO";
        break;
      case LogLevel::loggerdebug:
        indicator = "DEBUG";
        break;
      case LogLevel::loggererror:
        indicator = "ERROR";
        break;
      default:
        indicator = "";
        break;
      }
      std::cout << dateTimeOstringstream.str().c_str() << " - " << indicator
             << ": " << message.c_str() << std::endl;
#endif

    }

  void initLogger() {
    if (!logger) logger = std::shared_ptr<Logger>(new Logger());
  }

  void deleteLogger() {
    logger = NULL;
  }
}
