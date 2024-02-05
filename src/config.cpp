#include "config.h"

namespace myserver {
Config::ConfigVarMap Config::s_datas;

// 虚函数必须要提供定义
myserver::ConfigVarBase::~ConfigVarBase() {}

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
    auto it = s_datas.find(name);
    return it == s_datas.end() ? nullptr : it->second;
}
static void ListAllMember(const std::string& prefix, const YAML::Node& node,
                          std::list<std::pair<std::string, const YAML::Node>>& output) {
    // 字符校验
    if (prefix.find_first_not_of("abcdefghiJklmnopqrstuvwxyz._0123456789") != std::string::npos) {
        MYSERVER_LOG_ERROR(MYSERVER_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
    }

    output.push_back(std::make_pair(prefix, node));

    // 多层的情况
    if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            ListAllMember(prefix.empty() ? (it->first.Scalar()) : (prefix + "." + it->first.Scalar()), it->second,
                          output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);

    for (auto& i : all_nodes) {
        std::string key = i.first;
        if (key.empty()) {
            continue;
        }

        // 依然是全部转为小写
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        ConfigVarBase::ptr var = LookupBase(key);
        if (var) {
            if (i.second.IsScalar()) {
                var->fromString(i.second.Scalar());

            } else {
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }
}

}  // namespace myserver
