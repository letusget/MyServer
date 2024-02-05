#ifndef __MYSERVER_CONFIG_H__
#define __MYSERVER_CONFIG_H__

#include <yaml-cpp/yaml.h>

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

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

   protected:
    std::string m_name;
    std::string m_description;
};

template <class T>
class ConfigVar : public ConfigVarBase {
   public:
    typedef std::shared_ptr<ConfigVar> ptr;
    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        : ConfigVarBase(name, description), m_val(default_value) {}

    std::string toString() override {
        // 类型转换可能会有异常安全问题
        try {
            boost::lexical_cast<std::string>(m_val);
        } catch (std::exception& e) {
            MYSERVER_LOG_ERROR(MYSERVER_LOG_ROOT())
                << "ConfigVar::toString exception" << e.what() << "convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        // 类型转换可能会有异常安全问题
        try {
            m_val = boost::lexical_cast<T>(val);
        } catch (std::exception& e) {
            MYSERVER_LOG_ERROR(MYSERVER_LOG_ROOT())
                << "ConfigVar::toString exception" << e.what() << "convert: string to" << typeid(m_val).name();
        }
        return false;
    }

    const T getValue() const { return m_val; }
    void setValue(const T& v) { m_val = v; }

   private:
    T m_val;
};

class Config {
   public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    template <class T>
    // 这里的typename用于向编译器强调这是类型，因为有情况下 :: 后面是变量名
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value,
                                             const std::string& description = "") {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "Lookup name = " << name << " exists";
            return tmp;
        }

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
        return std::dynamic_pointer_cast<ConfigVar<T> >(
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