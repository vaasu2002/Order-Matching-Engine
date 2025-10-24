//
// Created by Vaasu Bisht on 24/10/25.
//

#pragma once

#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "tinyxml2.h"
#ifndef CONFIGREADER_H
#define CONFIGREADER_H


/**
 * @class ConfigReader
 * @brief Dedicated class to read and parse application configuration from an XML file.
 */
class ConfigReader {

 /**
  * @brief Helper function to get a required element's text from a parent XML element.
  * @param parent The parent XML element.
  * @param childName The name of the required child element.
  * @return The text content of the child element.
  */
 static std::string GetRequiredElementText(const tinyxml2::XMLElement* parent, const char* childName);

 /**
  * @brief Helper function to get a required integer value from a child element.
  * @param parent The parent XML element.
  * @param childName The name of the required child element.
  * @return The integer value of the child element.
  */
 static size_t GetRequiredElementSizeT(const tinyxml2::XMLElement* parent, const char* childName);

public:
 // Configuration structure for the application
 struct Config
 {
  std::string obWorkerPrefix;
  size_t obWorkerCnt;
 };
 static Config LoadConfig(const std::string& path);
};


#endif //CONFIGREADER_H
