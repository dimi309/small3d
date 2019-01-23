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

std::shared_ptr<small3d::Logger> logger;

namespace small3d {

  std::string intToStr(const int number)
  {
    char buffer[100];
    sprintf(buffer, "%d", number);
    return std::string(buffer);
  }

  Logger::Logger(std::ostream &stream) {
    logStream = &stream;
  }

  Logger::~Logger() {
    this->append(loggerinfo, "Logger getting destroyed");
    logStream = NULL;

  }

  void Logger::append(const LogLevel level, const std::string message) const {
    if (!logger) return;
    std::ostringstream dateTimeOstringstream;

    time_t now;

    time(&now);

    tm *t = localtime(&now);

    char buf[20];

    strftime(buf, 20,"%Y-%m-%d %H:%M:%S", t);

    dateTimeOstringstream << buf;

    std::string indicator;
    switch (level) {
    case loggerinfo:
      indicator = "INFO";
      break;
    case loggerdebug:
      indicator = "DEBUG";
      break;
    case loggererror:
      indicator = "ERROR";
      break;
    default:
      indicator = "";
      break;
    }

    *logStream << dateTimeOstringstream.str().c_str() << " - " << indicator
	       << ": " << message.c_str() << std::endl;
  }

  void initLogger() {
    if (!logger) logger = std::shared_ptr<Logger>(new Logger(std::cout));
  }

  void initLogger(std::ostream &stream) {
    if (!logger) logger = std::shared_ptr<Logger>(new Logger(stream));
  }

  void deleteLogger() {
    logger = NULL;
  }
}
