#include "bluesky/log.h"
#include "bluesky/config.h"
#include "bluesky/log_config.h"
#include <yaml-cpp/yaml.h>
#include <iostream>

#define TEST_LOG

#ifdef DEBUG
std::shared_ptr<bluesky::ConfigVar<int>> g_int_value_config =
    bluesky::Config::lookup("system.port", (int)8080, "system port");
std::shared_ptr<bluesky::ConfigVar<float>>
    g_float_value_config =
        bluesky::Config::lookup("system.value", (float)10.2f, "system value");
std::shared_ptr<bluesky::ConfigVar<std::string>> g_string_value_config =
    bluesky::Config::lookup("logs.name", (std::string) "super", "logs.name");

std::set<int> st{1, 2, 3};
std::shared_ptr<bluesky::ConfigVar<std::set<int>>> g_set_value_config =
    bluesky::Config::lookup("set", st, "test set<int>");

void print_yaml(const YAML::Node &node, int level)
{
    if (node.IsScalar())
    {
        BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << std::string(level * 4, ' ')
                                             << node.Scalar() << " - " << node.Type() << " - " << level;
    }
    else if (node.IsNull())
    {
        BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << std::string(level * 4, ' ')
                                             << "NULL - " << node.Type() << " - " << level;
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); it++)
        {
            BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << std::string(level * 4, ' ')
                                                 << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (size_t i = 0; i < node.size(); i++)
        {
            BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << std::string(level * 4, ' ')
                                                 << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml()
{
    YAML::Node node = YAML::LoadFile("./config/log.yml");
    print_yaml(node, 0);

    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << node.Scalar();
}

void test_config()
{
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "before: " << g_int_value_config->get_value();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "before: " << g_float_value_config->get_value();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "before: " << g_string_value_config->get_value();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "before: ";
    std::set<int> result = g_set_value_config->get_value();
    for (auto &i : result)
    {
        std::cout << " " << i;
    }

    std::cout << std::endl;
    YAML::Node node = YAML::LoadFile("../bin/config/log.yml");
    bluesky::Config::load_from_yaml(node);

    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "after: " << g_int_value_config->get_value();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "after: " << g_float_value_config->toString();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "after: " << g_string_value_config->get_value();
}

#endif

#ifdef TEST_LOG

class Person
{
public:
    std::string m_name;
    int m_age;
    bool m_sex;

    std::string toString() const
    {
        std::stringstream ss;
        ss << "[person name=" << m_name
           << " age=" << m_age << " sex=" << m_sex << "]";
        return ss.str();
    }
    bool operator==(const Person &p) const
    {
        return m_name == p.m_name && m_age == p.m_age && m_sex == p.m_sex;
    }
    bool operator<(const Person &p) const
    {
        return m_age < p.m_age;
    }
};

namespace bluesky
{

    template <>
    class LexicalCast<std::string, Person>
    {
    public:
        Person operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            Person p;
            p.m_name = node["name"].as<std::string>();
            p.m_age = node["age"].as<int>();
            p.m_sex = node["sex"].as<bool>();
            return p;
        }
    };

    template <>
    class LexicalCast<Person, std::string>
    {
    public:
        std::string operator()(const Person &p)
        {
            YAML::Node node;
            node["name"] = p.m_name;
            node["age"] = p.m_age;
            node["sex"] = p.m_sex;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
} //end of namespace bluesky

std::shared_ptr<bluesky::ConfigVar<Person>> g_person =
    bluesky::Config::lookup("class.person", Person(), "system person");

std::shared_ptr<bluesky::ConfigVar<std::map<std::string, Person>>> g_person_map =
    bluesky::Config::lookup("class.map", std::map<std::string, Person>(), "system person");

std::shared_ptr<bluesky::ConfigVar<std::map<std::string, std::vector<Person>>>> g_person_vec_map =
    bluesky::Config::lookup("class.vec_map", std::map<std::string, std::vector<Person>>(), "system person");

std::shared_ptr<bluesky::ConfigVar<std::set<Person>>> g_person_set =
    bluesky::Config::lookup("class.set", std::set<Person>(), "system set person");



void test_class()
{
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "before: " << g_person->get_value().toString() << " - " << g_person->toString();

#define XX_PM(g_var, prefix)                                                                                   \
    {                                                                                                          \
        auto m = g_person_map->getValue();                                                                     \
        for (auto &i : m)                                                                                      \
        {                                                                                                      \
            BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString(); \
        }                                                                                                      \
        BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << prefix << ": size=" << m.size();                               \
    }

    g_person->add_listener([](const Person &old_value, const Person &new_value)
                           { BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "old_value=" << old_value.toString()
                                                                  << " new_value=" << new_value.toString(); });

    //XX_PM(g_person_map, "class.map before");
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "before: " << g_person_vec_map->toString();

    YAML::Node root = YAML::LoadFile("/home/bluesky/workspace/bluesky/bin/conf/test.yml");
    bluesky::Config::load_from_yaml(root);

    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "after: " << g_person->get_value().toString() << " - " << g_person->toString();
    //XX_PM(g_person_map, "class.map after");
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "after: " << g_person_vec_map->toString();
}

void test_log()
{
    static std::shared_ptr<bluesky::Logger> system_log = BLUESKY_LOG_NAME("system");
    BLUESKY_LOG_INFO(system_log) << "hello system";
    std::cout << bluesky::LoggerMgr::get_instance().toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("./conf/log.yml");
    bluesky::Config::load_from_yaml(root);
    std::cout << "=============" << std::endl;
    std::cout << bluesky::LoggerMgr::get_instance().toYamlString() << std::endl;
    std::cout << "=============" << std::endl;
    std::cout << root << std::endl;
    BLUESKY_LOG_INFO(system_log) << "hello system" << std::endl;
    BLUESKY_LOG_INFO(system_log) << "难受啊";
    //BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "helloo root";

    bluesky::FileLogAppender::Ptr file_appender(new bluesky::FileLogAppender("./test_system.txt"));
    system_log->add_appender(file_appender);
    BLUESKY_LOG_INFO(system_log) << "test file appender";
}

#endif

int main()
{

    //test_yaml();
    //test_config();

    test_log();
    return 0;
}
