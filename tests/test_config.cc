#include "bluesky/log.h"
#include "bluesky/config.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
std::shared_ptr<bluesky::ConfigVar<int>> g_int_value_config =
    bluesky::Config::lookup("system.port", (int)8080, "system port");
std::shared_ptr<bluesky::ConfigVar<float>> g_float_value_config =
    bluesky::Config::lookup("system.value", (float)10.2f, "system value");
std::shared_ptr<bluesky::ConfigVar<std::string>> g_string_value_config =
    bluesky::Config::lookup("logs.name", (std::string)"super", "logs.name");

void print_yaml(const YAML::Node& node, int level)
{
    if(node.IsScalar()){
        BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << std::string(level * 4, ' ')
                                             << node.Scalar() << " - " << node.Type() << " - " << level;
    }else if(node.IsNull()){
        BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << std::string(level * 4, ' ')
                                             << "NULL - " << node.Type() << " - " << level;
    }else if(node.IsMap()){
        for (auto it = node.begin(); it != node.end();it++){
            BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << std::string(level * 4, ' ')
                                                 << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }else if(node.IsSequence()){
        for (size_t i = 0; i < node.size();i++){
            BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << std::string(level * 4, ' ')
                                                 << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml()
{
    YAML::Node node = YAML::LoadFile("../bin/config/log.yaml");
    print_yaml(node, 0);

    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << node.Scalar();
}

void test_config()
{
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "before: " << g_int_value_config->get_value();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "before: " << g_float_value_config->get_value();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "before: " << g_string_value_config->get_value();
    YAML::Node node = YAML::LoadFile("../bin/config/log.yaml");
    bluesky::Config::load_from_yaml(node);

    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "after: " << g_int_value_config->get_value();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "after: " << g_float_value_config->toString();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "after: " << g_string_value_config->get_value();


}
int main()
{

    //test_yaml();
    test_config();
    return 0;
}