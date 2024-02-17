#ifndef __MYSERVER_CONFIG_H__
#define __MYSERVER_CONFIG_H__

#include <yaml-cpp/yaml.h>

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "log.h"

namespace myserver {
// 配置基类
class ConfigVarBase {
   public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "") : m_description(description) {
        m_name.reserve(name.size());
        std::transform(name.begin(), name.end(), std::back_inserter(m_name),
                       [](unsigned char c) -> char { return std::tolower(c); });
    }
    virtual ~ConfigVarBase();

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    // 转换为明文
    virtual std::string toString() = 0;
    // 解析参数
    virtual bool fromString(const std::string& val) = 0;
    // 获取类型
    virtual std::string getTypeName() const = 0;
   protected:
    std::string m_name;
    std::string m_description;
};

// 通用的基础类型解析
template <class F, class T>  // F from_type, T to_type
class LexicalCast {
   public:
    T operator()(const F& v) { return boost::lexical_cast<T>(v); }
};

// 负责类型的解析
// vector 反序列化
template <class T>
class LexicalCast<std::string, std::vector<T>> {
   public:
    std::vector<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        // C++17支持的返回值优化，允许在调用者的空间内直接构造返回对象，不需要强制move进行移动
        // return std::move(vec);
        return vec;
    }
};
// vector 序列化
template <class T>
class LexicalCast<std::vector<T>, std::string> {
   public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// list 反序列化
template <class T>
class LexicalCast<std::string, std::list<T>> {
   public:
    std::list<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        // C++17支持的返回值优化，允许在调用者的空间内直接构造返回对象，不需要强制move进行移动
        // return std::move(vec);
        return vec;
    }
};
// list 序列化
template <class T>
class LexicalCast<std::list<T>, std::string> {
   public:
    std::string operator()(const std::list<T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// set 反序列化
template <class T>
class LexicalCast<std::string, std::set<T>> {
   public:
    std::set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        // C++17支持的返回值优化，允许在调用者的空间内直接构造返回对象，不需要强制move进行移动
        // return std::move(vec);
        return vec;
    }
};
// set 序列化
template <class T>
class LexicalCast<std::set<T>, std::string> {
   public:
    std::string operator()(const std::set<T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// unordered_set 反序列化
template <class T>
class LexicalCast<std::string, std::unordered_set<T>> {
   public:
    std::unordered_set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        // C++17支持的返回值优化，允许在调用者的空间内直接构造返回对象，不需要强制move进行移动
        // return std::move(vec);
        return vec;
    }
};
// unordered_set 序列化
template <class T>
class LexicalCast<std::unordered_set<T>, std::string> {
   public:
    std::string operator()(const std::unordered_set<T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// map 反序列化
template <class T>
class LexicalCast<std::string, std::map<std::string, T>> {
   public:
    std::map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        // C++17支持的返回值优化，允许在调用者的空间内直接构造返回对象，不需要强制move进行移动
        // return std::move(vec);
        return vec;
    }
};
// map 序列化
template <class T>
class LexicalCast<std::map<std::string, T>, std::string> {
   public:
    std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// unordered_map 反序列化
template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
   public:
    std::unordered_map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        // C++17支持的返回值优化，允许在调用者的空间内直接构造返回对象，不需要强制move进行移动
        // return std::move(vec);
        return vec;
    }
};
// unordered_map 序列化
template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
   public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 将任何str转换成对应类型以及将任意类型转换为str(序列号与反序列化)
// FromStr T operator()(const std::string&) // 反序列化
// Tostr std::string operator()(const T&)   // 序列号
template <class T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>  // 特化的类型转换
class ConfigVar : public ConfigVarBase {
   public:
    typedef std::shared_ptr<ConfigVar> ptr;
    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        : ConfigVarBase(name, description), m_val(default_value) {}

    std::string toString() override {
        // 类型转换可能会有异常安全问题
        try {
            // return boost::lexical_cast<std::string>(m_val);
            return ToStr()(m_val);
        } catch (std::exception& e) {
            MYSERVER_LOG_ERROR(MYSERVER_LOG_ROOT())
                << "ConfigVar::toString exception" << e.what() << "convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        // 类型转换可能会有异常安全问题
        try {
            // m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
        } catch (std::exception& e) {
            MYSERVER_LOG_ERROR(MYSERVER_LOG_ROOT())
                << "ConfigVar::toString exception" << e.what() << "convert: string to" << typeid(m_val).name() << " - " << val;
        }
        return false;
    }

    const T getValue() const { return m_val; }
    void setValue(const T& v) { m_val = v; }
    std::string getTypeName() const override { return typeid(T).name(); }

   private:
    T m_val;
};

class Config {
   public:
    typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    template <class T>
    // 这里的typename用于向编译器强调这是类型，因为有情况下 :: 后面是变量名
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value,
                                             const std::string& description = "") {
        auto it = s_datas.find(name);
        if (it != s_datas.end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if (tmp) {
                MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "Lookup name = " << name << " exists";
                return tmp;
            } else {
                MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT())
                    << "Lookup name = " << name << " exists but type not " << typeid(T).name()
                    << " real_type = " << it->second->getTypeName() << " " << it->second->toString();
                return nullptr;
            }
        }
        // auto tmp = Lookup<T>(name);
        // if (tmp) {
        // MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "Lookup name = " << name << " exists";
        // return tmp;
        // }

        // 检查是否有非法字符(这里只判断小写，如果有大写就强制改为小写字母)
        if (name.find_first_not_of("abcdefghiJklmnopqrstuvwxyz._0123456789") != std::string::npos) {
            MYSERVER_LOG_ERROR(MYSERVER_LOG_ROOT()) << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        s_datas[name] = v;

        return v;
    }

    // 查找对应类
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto it = s_datas.find(name);
        if (it == s_datas.end()) {
            return nullptr;
        }
        // 强制将父类转为子类(只能对于shared_ptr使用)
        return std::dynamic_pointer_cast<ConfigVar<T>>(
            it->second);  // C++11之前 >> 会被解析成右移运算符，所以在模板下需要使用空格区分
    }

    // 使用yaml中的配置覆盖原有配置
    static void LoadFromYaml(const YAML::Node& root);
    // 这里只能返回指针或引用(ConfigVarBase为抽象类)返回
    static ConfigVarBase::ptr LookupBase(const std::string& name);

   private:
    static ConfigVarMap s_datas;
};

}  // namespace myserver

#endif