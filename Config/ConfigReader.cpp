//
// Created by Vaasu Bisht on 24/10/25.
//

#include "ConfigReader.h"

using namespace tinyxml2;

std::string ConfigReader::GetRequiredElementText(const XMLElement* parent, const char* childName)
{
    if (!parent)
    {
        throw std::runtime_error("XML parsing error: Parent element is null.");
    }

    const XMLElement* child = parent->FirstChildElement(childName);
    if (!child)
    {
        throw std::runtime_error("Configuration error: Missing required element <" + std::string(childName) + ">.");
    }

    const char* text = child->GetText();
    if (!text || std::string(text).empty())
    {
        throw std::runtime_error("Configuration error: Element <" + std::string(childName) + "> cannot be empty.");
    }

    return text;
}

size_t ConfigReader::GetRequiredElementSizeT(const XMLElement* parent, const char* childName)
{
    std::string text = GetRequiredElementText(parent, childName);
    try
    {
        // Use stoul for size_t conversion
        unsigned long val = std::stoul(text);
        return static_cast<size_t>(val);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Configuration error: Value for <" + std::string(childName) + "> is not a valid positive integer. Details: " + e.what());
    }
}

ConfigReader::Config ConfigReader::LoadConfig(const std::string& path)
{
    XMLDocument doc;

    if (XMLError result = doc.LoadFile(path.c_str()); result != XML_SUCCESS)
    {
        std::stringstream ss;
        ss << "Failed to open or parse XML configuration file '" << path << "'. TinyXML-2 Error: " << doc.ErrorIDToName(result);
        throw std::runtime_error(ss.str());
    }

    Config config;
    const XMLElement* root = doc.FirstChildElement("Configuration");
    if (!root)
    {
        throw std::runtime_error("Configuration error: Root element <Configuration> not found.");
    }

    // --- OrderBookScheduler Configuration ---
    const XMLElement* obsConfig = root->FirstChildElement("OrderBookScheduler");
    if (!obsConfig)
    {
        throw std::runtime_error("Configuration error: Missing required element <OrderBookScheduler>.");
    }

    config.obWorkerPrefix = GetRequiredElementText(obsConfig, "WorkerPrefix");
    config.obWorkerCnt = GetRequiredElementSizeT(obsConfig, "WorkerCount");


    return config;
}
