#include "config.h"

namespace myserver {
Config::ConfigVarMap Config::s_datas;

// 虚函数必须要提供定义
myserver::ConfigVarBase::~ConfigVarBase() {}

}  // namespace myserver
