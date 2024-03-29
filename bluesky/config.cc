#include "bluesky/config.h"
#include <list>

namespace bluesky
{
    //Config::ConfigVarMap Config::configs_;

    //把所有map的形式转换成pair[key: node]
    static void list_all_member(const std::string &prefix,
                                const YAML::Node &node,
                                std::list<std::pair<std::string, const YAML::Node>>& nodes)
    {
        if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
        {
            BLUESKY_LOG_ERROR(BLUESKY_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
            return;
        }
        nodes.push_back(std::make_pair(prefix, node));
        if (node.IsMap())
        {
            for (auto iter = node.begin();
                 iter != node.end();
                 ++iter)
            {
                list_all_member(prefix.empty() ? iter->first.Scalar() : prefix + "." + iter->first.Scalar(),
                                iter->second, nodes);
            }
        }
    }

    //找出所有键值对
    void Config::load_from_yaml(const YAML::Node &root)
    {
        std::list<std::pair<std::string, const YAML::Node>> allNodes;
        list_all_member("", root, allNodes);

        for (auto &node : allNodes)
        {
            std::string key = node.first;
            if (key.empty())
                continue;

            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            std::shared_ptr<ConfigVarBase> var = bluesky::Config::lookup_base(key);

            if (var)
            {
                if (node.second.IsScalar())
                {
                    var->fromString(node.second.Scalar());
                }
                else
                {
                    std::stringstream ss;
                    ss << node.second;
                    var->fromString(ss.str());
                }
            }
        }
    }
    std::shared_ptr<ConfigVarBase> Config::lookup_base(const std::string &name)
    {
        MutexType::Lock lock(get_mutex());
        auto iter = get_configs().find(name);
        if (iter == get_configs().end())
        {
            return nullptr;
        }
        return iter->second;
    }

    void Config::visit_configs(std::function<void(ConfigVarBase::Ptr)>& callback)
    {
        MutexType::Lock lock(get_mutex());
        ConfigVarMap& configs=get_configs();
        for(auto iter=configs.begin(); iter!=configs.end();iter++){
            callback(iter->second);
        }

    }

} //end of namespace
