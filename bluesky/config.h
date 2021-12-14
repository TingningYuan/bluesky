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
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

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
        virtual bool fromString(const std::string &val) = 0;
        virtual std::string get_typename() const = 0;

    private:
        std::string name_;
        std::string description_;
    };

    //F:源类型 T:目标类型
    template <class F, class T>
    class LexicalCast
    {
    public:
        T operator()(const F &v)
        {
            return boost::lexical_cast<T>(v);
        }
    };

    template <class T>
    class LexicalCast<std::string, std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::vector<T> vec;
            std::stringstream ss;
            for (auto i = 0; i < node.size(); i++)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
        }
    };

    template <class T>
    class LexicalCast<std::vector<T>, std::string>
    {
    public:
        std::string operator()(const std::vector<T> &v)
        {
            YAML::Node node;
            for (auto &vv : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(vv)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    template <class T>
    class LexicalCast<std::string, std::list<T>>
    {
    public:
        std::list<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::list<T> lst;
            std::stringstream ss;
            for (auto i = 0; i < node.size(); i++)
            {
                ss.str("");
                ss << node[i];
                lst.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return lst;
        }
    };

    template<class T>
    class LexicalCast<std::list<T>, std::string>
    {
        public:
            std::string operator()(const std::list<T> &v){
                YAML::Node node;
                for(auto& i:v){
                    node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
                }
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };

    template<class T>
    class LexicalCast<std::string, std::set<T>>
    {
        public:
            std::set<T> operator()(const std::string& v){
                YAML::Node node = YAML::Load(v);
                typename std::set<T> st;
                std::stringstream ss;
                for (auto i = 0; i < node.size();i++){
                    ss.str("");
                    ss << node[i];
                    st.insert(LexicalCast<std::string, T>()(ss.str()));
                }
                return st;
            }
    };

    template<class T>
    class LexicalCast<std::set<T>, std::string>
    {
        public:
            std::string operator()(const std::set<T>& v){
                YAML::Node node;
                for (auto& i:v){
                    node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
                }
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };

    template<class T>
    class LexicalCast<std::string, std::unordered_set<T>>
    {
        public:
        std::unordered_set<T> operator()(const std::string& v){
            YAML::Node node = YAML::Load(v);
            std::unordered_set<T> unst;
            std::stringstream ss;
            for (auto i = 0; i < node.size();i++){
                ss.str("");
                ss << node[i];
                unst.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return unst;
        }
    };

    template<class T>
    class LexicalCast<std::unordered_set<T>, std::string>
    {
        public:
        std::string operator()(const std::unordered_set<T>& v){
            YAML::Node node;
            for(auto& i:v){
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    template<class T>
    class LexicalCast<std::string, std::map<std::string,T>>
    {
        public:
        std::map<std::string,T> operator()(const std::string& v){
            YAML::Node node = YAML::Load(v);
            std::map<std::string, T> mp;
            std::stringstream ss;
            for (auto iter = node.begin(); iter != node.end();iter++)
            {
                ss.str("");
                ss << iter->second;
                mp.insert(std::make_pair(iter->first.Scalar(),
                                         LexicalCast<std::string, T>()(ss.str())));
            }
            return mp;
        }
    };

    template<class T>
    class LexicalCast<std::map<std::string,T>, std::string>
    {
        public:
        std::string operator()(const std::map<std::string,T>& v){
            YAML::Node node;
            for(auto& [key,value]:v){
                node[key] = YAML::Load(LexicalCast<T, std::string>()(value));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

 template<class T>
    class LexicalCast<std::string, std::unordered_map<std::string,T> >
    {
        public:
        std::unordered_map<std::string,T> operator()(const std::string& v){
            YAML::Node node = YAML::Load(v);
            std::unordered_map<std::string, T> mp;
            std::stringstream ss;
            for (auto iter = node.begin(); iter != node.end();iter++)
            {
                ss.str("");
                ss << iter->second;
                mp.insert(std::make_pair(iter->first.Scalar,
                                         LexicalCast<std::string, T>()(ss.str())));
            }
            return mp;
        }
    };

    template<class T>
    class LexicalCast<std::unordered_map<std::string,T>, std::string>
    {
        public:
        std::string operator()(const std::unordered_map<std::string,T>& v){
            YAML::Node node;
            for(auto& [key,value]:v){
                node[key] = YAML::Load(LexicalCast<T, std::string>()(value));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
    /* T: 参数类型
     * FromStr: 将string转换为T类型
     * ToStr: 将T类型转换为string
     */
    template <class T,
              class FromStr = LexicalCast<std::string, T>,
              class ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVar> Ptr;
        typedef std::function<void(const T &old_value, const T &new_value)> on_change_cb;
        ConfigVar(const std::string &name, const T &value, const std::string &description = "")
            : ConfigVarBase(name, description), value_(value)
        {
        }

        std::string toString() override
        {
            try
            {
                return ToStr()(value_);
            }
            catch (std::exception &e)
            {
                BLUESKY_LOG_ERROR(BLUESKY_LOG_ROOT()) << "ConfigVar::to_string() exception"
                                                      << e.what() << " convert: " << typeid(value_).name() << " to string";
            }
            return "";
        }

        bool fromString(const std::string &val) override
        {
            try
            {
                set_value(FromStr()(val));
            }
            catch (std::exception &e)
            {
                BLUESKY_LOG_ERROR(BLUESKY_LOG_ROOT()) << "ConfigVar::from_string() exception "
                                                      << e.what() << " convert string to: " << typeid(value_).name();
            }
            return true;
        }

        const T get_value() const { return value_; }
        void set_value(const T &value) { 
            if(value==value_){
                return;
            }
            for(auto& call:callbacks_){
                call.second(value_, value);
            }
            value_ = value;
        }
        std::string get_typename() const override { return typeid(T).name(); }
        void add_listener(const uint64_t key, const on_change_cb& callback){
            callbacks_[key] = callback;
        }
        void delete_listener(uint64_t key){
            callbacks_.erase(key);
        }
        on_change_cb get_listener(uint64_t key){
            auto iter = callbacks_.find(key);
            return iter == callbacks_.end() ? nullptr : iter->second;
        }

        void clearListener(){
            callbacks_.clear();
        }

    private:
        T value_;
        //变更通知回调数组
        std::map<uint64_t, on_change_cb> callbacks_;
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
            auto iter=configs_.find(name);
            if(iter!=configs_.end())
            {
            auto temp = std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
            if (temp)
            {
                BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "look up name: " << name << " exists";
                return temp;
            }
            else{
                BLUESKY_LOG_ERROR(BLUESKY_LOG_ROOT()) << "look up name=" << name << " exists but type= " << typeid(T).name()
                                                      << " not equal real type= " << iter->second->get_typename()
                                                      << " " << iter->second->toString();
            }
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