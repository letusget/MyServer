#include <yaml-cpp/yaml.h>

#include "config.h"

myserver::ConfigVar<int>::ptr g_int_value_config = myserver::Config::Lookup("system.port", (int)8080, "system port");
myserver::ConfigVar<float>::ptr g_float_value_config =
    myserver::Config::Lookup("system.value", (float)6.16f, "system value");

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

    YAML::Node root = YAML::LoadFile("../bin/conf/log.yml");
    myserver::Config::LoadFromYaml(root);

    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    MYSERVER_LOG_INFO(MYSERVER_LOG_ROOT()) << "after: " << g_float_value_config->getValue();
}
int main(int argc, char** argv) {
    // test_yaml();
    test_config();
    return 0;
}