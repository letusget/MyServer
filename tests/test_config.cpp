#include <yaml-cpp/yaml.h>

#include "config.h"

myserver::ConfigVar<int>::ptr g_int_value_config = myserver::Config::Lookup("system.port", (int)8080, "system port");
// 同样数据的不同类型会报错
myserver::ConfigVar<float>::ptr g_float_valuex_config =
    myserver::Config::Lookup("system.port", (float)8080, "system port");

myserver::ConfigVar<float>::ptr g_float_value_config =
    myserver::Config::Lookup("system.value", (float)6.16f, "system value");
myserver::ConfigVar<std::vector<int>>::ptr g_int_vector_value_config =
    myserver::Config::Lookup("system.int_vector", std::vector<int>{1, 2}, "system int vec");
myserver::ConfigVar<std::list<int>>::ptr g_int_list_value_config =
    myserver::Config::Lookup("system.int_list", std::list<int>{1, 2}, "system int list");
myserver::ConfigVar<std::set<int>>::ptr g_int_set_value_config =
    myserver::Config::Lookup("system.int_set", std::set<int>{1, 2}, "system int set");
myserver::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_value_config =
    myserver::Config::Lookup("system.int_uset", std::unordered_set<int>{1, 2}, "system int unordered_set");
myserver::ConfigVar<std::map<std::string, int>>::ptr g_str_int_map_value_config = myserver::Config::Lookup(
    "system.str_int_map", std::map<std::string, int>{{"k1", 1}, {"k2", 2}}, "system str int map");
myserver::ConfigVar<std::unordered_map<std::string, int>>::ptr g_str_int_umap_value_config = myserver::Config::Lookup(
    "system.str_int_umap", std::unordered_map<std::string, int>{{"k1", 1}, {"k2", 2}}, "system str int unordered_map");

void print_yaml(const YAML::Node& node, int level) {
    if (node.IsScalar()) {
        MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT())
            << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if (node.IsNull()) {
        MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT())
            << std::string(level * 4, ' ') << "NULL - " << node.Type() << " - " << level;
    } else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT())
                << std::string(level * 4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i) {
            MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT())
                << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    // 对于系统自启程序，如果程序CWD(当前工作目录)目录不是期望的目录，可能会存在相对路径错误的情况
    YAML::Node root = YAML::LoadFile("../bin/conf/log.yml");
    // MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "\n" << root;

    print_yaml(root, 0);
}

void test_config() {
    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "before: " << g_float_value_config->getValue();

#define XX(g_var, name, prefix)                                                                     \
    {                                                                                               \
        auto v = g_var->getValue();                                                                 \
        for (auto& i : v) {                                                                         \
            MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << #prefix " " #name ": " << i;                  \
        }                                                                                           \
        MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix)                                                                                      \
    {                                                                                                                  \
        auto v = g_var->getValue();                                                                                    \
        for (auto& i : v) {                                                                                            \
            MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << #prefix " " #name ": {" << i.first << " - " << i.second << "} "; \
        }                                                                                                              \
        MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString();                    \
    }

    XX(g_int_vector_value_config, int_vector, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_umap_value_config, str_int_umap, before);

    YAML::Node root = YAML::LoadFile("../bin/conf/log.yml");
    myserver::Config::LoadFromYaml(root);

    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "after: " << g_float_value_config->getValue();

    XX(g_int_vector_value_config, int_vector, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

class Person {
   public:
    Person(){};
    std::string getName() const { return m_name; }
    int getAge() const { return m_age; }
    bool getSex() const { return m_sex; }
    void setName(const std::string& name) { m_name = name; }
    void setAge(int age) { m_age = age; }
    void setSex(bool sex) { m_sex = sex; }

    std::string toString() const {
        std::stringstream ss;
        ss << "[ Person name = " << m_name << ", age = " << m_age << ", sex = " << m_sex << " ]";
        return ss.str();
    }

   private:
    std::string m_name = "";
    int m_age          = 0;
    bool m_sex         = false;
};

namespace myserver {
// 模板偏特化
// class 反序列化
template <>
class LexicalCast<std::string, Person> {
   public:
    Person operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.setName(node["name"].as<std::string>());
        p.setAge(node["age"].as<int>());
        p.setSex(node["sex"].as<bool>());
        return p;
    }
};
// class 序列化
template <>
class LexicalCast<Person, std::string> {
   public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.getName();
        node["age"]  = p.getAge();
        node["sex"]  = p.getSex();
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
}  // namespace myserver
myserver::ConfigVar<Person>::ptr g_person = myserver::Config::Lookup("class.person", Person(), "system person");

// map
myserver::ConfigVar<std::map<std::string, Person>>::ptr g_person_map =
    myserver::Config::Lookup("class.map", std::map<std::string, Person>(), "system person");

// vector-map
myserver::ConfigVar<std::map<std::string, std::vector<Person>>>::ptr g_person_vec_map =
    myserver::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person>>(), "system person");

void test_class() {
    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT())
        << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
#define XX_PM(g_var, prefix)                                                                                     \
    {                                                                                                            \
        auto m = g_var->getValue();                                                                              \
        for (auto& i : m) {                                                                                      \
            MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString(); \
        }                                                                                                        \
        MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << prefix << ": size=" << m.size();                               \
    }
    XX_PM(g_person_map, "class.map before");
    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "before: " << g_person_vec_map->toString();

    YAML::Node root = YAML::LoadFile("../bin/conf/log.yml");
    myserver::Config::LoadFromYaml(root);

    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT())
        << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map after");

    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "after: " << g_person_vec_map->toString();

}

int main(int argc, char** argv) {
    // test_yaml();
    // test_config();
    test_class();

    return 0;
}