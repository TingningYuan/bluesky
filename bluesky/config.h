#ifndef __BLUESKY_CONFIG_H__
#define __BLUESKY_CONFIG_H__

#include "bluesky/log.h"
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <exception>

namespace bluesky
{
    /* 配置基类:定义配置项的基础接口
 * 基础属性：
 *      配置名称(get_configname)
 *      配置描述(get_description)
 *      序列化(to_string)
 *      反序列化(from_string)
 *      具体的参数类型名称(get_typename)
 */
    class ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVarBase> Ptr;

        ConfigVarBase(const std::string &name, const std::string &description = "")
            : name_(name), description_(description)
        {
            //全都转化为小写
            std::transform(name_.begin(), name_.end(), name_.begin(), ::tolower);
        }
        virtual ~ConfigVarBase() {}

        const std::string &get_configname() const { return name_; }
        const std::string &get_description() const { return description_; }

        virtual std::string toString() = 0;
        virtual bool from_string(const std::string &val) = 0;

    private:
        std::string name_;
        std::string description_;
    };

    template <class T>
    class ConfigVar : public ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVar> Ptr;

        ConfigVar(const std::string &name, const T& value, const std::string &description="")
            : ConfigVarBase(name, description), value_(value)
        {
        }

        std::string toString()
        {
            try
            {
                return boost::lexical_cast<std::string>(value_);
            }
            catch (std::exception &e)
            {
                BLUESKY_LOG_ERROR(BLUESKY_LOG_ROOT()) << "ConfigVar::to_string() exception"
                                                      << e.what() << " convert: " << typeid(value_).name() << " to string";
            }
            return "";
        }

        bool from_string(const std::string &val)
        {
            try
            {
                value_ = boost::lexical_cast<T>(val);
            }
            catch (std::exception &e)
            {
                BLUESKY_LOG_ERROR(BLUESKY_LOG_ROOT()) << "ConfigVar::from_string() exception "
                                                      << e.what() << " convert string to: " << typeid(value_).name();
            }
        }

        const T get_value() const { return value_; }
        void set_value(const T &value) { value_ = value; }

    private:
        T value_;
    };

    /*配置集合类。 提供所有配置项的ConfigVar的统一管理功能。
 *加载配置文件，更新配置文件，定义配置项等等
 *重要的函数：
 *  Lookup, 3个参数签名： 创建和定义配置项,同一个配置项只能创建一次否则会冲突
 *   Lookup, 1个参数签名： 获取对应名称的配置项,用来获取上面函数创建的配置项
 *   LoadFromConfDir: 加载配置文件,加载一个目录下面的所有yaml文件
 *                   支持多文件,不同的配置项可以分开文件定义
 *  Visit： 便利所有配置项的方法。可以用来可视化输出
 */

    class Config
    {
    public:
        typedef std::map<std::string, std::shared_ptr<ConfigVarBase>> ConfigVarMap;

        template <class T>
        /*将指定名称的配置项加入进来*/
        static typename std::shared_ptr<ConfigVar<T>> lookup(const std::string &name,
                                                             const T &value, 
                                                             const std::string &description = "")
        {
            auto temp = lookup<T>(name);
            if (temp)
            {
                BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "look up name: " << name << " exists";
                return temp;
            }

            if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
            {
                BLUESKY_LOG_ERROR(BLUESKY_LOG_ROOT()) << "look up name invalid: " << name;
                throw std::invalid_argument(name);
            }

            typename std::shared_ptr<ConfigVar<T>> v(new ConfigVar<T>(name, value, description));
            configs_[name] = v;
            return v;
        }
        template <class T>
        static typename std::shared_ptr<ConfigVar<T>> lookup(const std::string &name)
        {
            auto iter = configs_.find(name);
            if (iter == configs_.end())
                return nullptr;
            return std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
        }

        static void load_from_yaml(const YAML::Node &root);
        static std::shared_ptr<ConfigVarBase> lookup_base(const std::string &name);

    private:
        static ConfigVarMap configs_;
    };
    

} //end of namespace

#endif