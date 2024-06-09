#include "util.h"

#include <dirent.h>
#include <execinfo.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>

#include "log.h"

namespace myserver {

// 本系统的日志对象为system
mylog::Logger::ptr g_logger = MYLOG_LOG_NAME("system");

pid_t GetThreadId() { return syscall(SYS_gettid); }

u_int32_t GetFiberId() { return 0; }

void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    // 参数检查, 这里对于错误参数直接抛出异常并返回
    if (size <= 0) {
        throw std::invalid_argument("Invalid size: must be greater than 0");
    }

    if (skip < 0 || skip >= size) {
        throw std::invalid_argument("Invalid skip: must be between 0 and size-1");
    }

    // 这里不使用backtrace的指针方式(不使用栈空间)，考虑到协程的栈内存有限，为节省内存，使用堆空间来存储栈信息
    void** array = (void**)malloc(sizeof(void*) * size);
    if (array == nullptr) {
        MYLOG_LOG_ERROR(g_logger) << "malloc failed in Backtrace";
        throw std::bad_alloc();
    }

    // 获取堆栈地址信息
    size_t len = ::backtrace(array, size);

    // 将堆栈帧地址转换为函数名和偏移量的字符串
    char** strings = backtrace_symbols(array, len);
    if (strings == nullptr) {
        MYLOG_LOG_ERROR(g_logger) << "backtrace_symbols failed";
        free(array);
        throw std::bad_alloc();
    }

    try {
        for (size_t i = skip; i < len; ++i) {
            bt.push_back(strings[i]);
        }
    } catch (...) {
        free(strings);
        free(array);
        throw;  // 重新抛出异常
    }

    // 释放堆栈信息
    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::ostringstream oss;
    for (auto& s : bt) {
        oss << prefix << s << "\n";
    }
    return oss.str();
}

/**
 * 列出指定目录下符合后缀的文件名
 * @param files 输出的文件名列表
 * @param path 目录路径
 * @param subfix 后缀名
 */
void FSUtil::ListAllFiles(std::vector<std::string>& files, const std::string& path, const std::string& subfix) {
    if (access(path.c_str(), F_OK) != 0) {
        return;
    }
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }
    struct dirent* dp = nullptr;
    while ((dp = readdir(dir)) != nullptr) {
        if (dp->d_type == DT_REG) {
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
                continue;
            }
            ListAllFiles(files, path + "/" + dp->d_name, subfix);
        } else if (dp->d_type == DT_REG) {
            std::string filename(dp->d_name);
            if (subfix.empty()) {
                files.push_back(path + "/" + filename);
            } else {
                if (filename.size() < subfix.size()) {
                    continue;
                }
                if (filename.substr(filename.length() - subfix.size()) == subfix) {
                    files.push_back(path + "/" + filename);
                }
            }
        }
        closedir(dir);
    }
}

/**
 * 获取文件属性
 * @param path 文件路径
 * @param st 文件属性结构体
 * @return 成功返回true，失败返回false
 */
static int __lstat(const char* path, struct stat* st = nullptr) {
    struct stat lst;
    // 这里使用lstat是因为stat不包含符号链接的属性
    int res = lstat(path, &lst);
    if (st) {
        *st = lst;
    }
    return res;
}

/**
 * 创建目录
 * @param dirname 目录路径
 * @return 成功返回0，失败返回-1
 */
static int __mkdir(const char* dirname) {
    if (access(dirname, F_OK) == 0) {
        return 0;
    }

    // 使用mkdir仅可以创建单级目录
    // 权限: 00700 | 00070 | 00004 | 00001 = 00775
    return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

/**
 * 递归创建目录
 * @param path 目录路径
 * @return 成功返回true，失败返回false
 */
bool FSUtil::Mkdir(const std::string& path) {
    if (__lstat(path.c_str(), nullptr) == 0) {
        return true;
    }

    // 多级目录
    char* filepath = strdup(path.c_str());
    char* ptr      = strchr(filepath + 1, '/');

    do {
        for (; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
            *ptr = '\0';
            if (__mkdir(filepath) != 0) {
                break;
            }
        }
        if (ptr != nullptr) {
            break;
        } else if (__mkdir(filepath) != 0) {
            break;
        }
        free(filepath);
        return true;
    } while (0);

    free(filepath);
    return false;
}

/**
 * 递归删除目录及其子目录
 * @param path 目录路径
 * @return 成功返回true，失败返回false
 * @note 如果是文件，则直接删除；如果是空目录，则删除；如果是非空目录，则递归删除其子目录
 */
bool FSUtil::Remove(const std::string& path) {
    struct stat st;
    if (lstat(path.c_str(), &st)) {
        return true;
    }
    if (!(st.st_mode & S_IFDIR)) {
        return Unlink(path, true);
    }

    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return false;
    }

    bool ret          = true;
    struct dirent* dp = nullptr;
    while ((dp = readdir(dir))) {
        if (!strcmp(dp->d_name, ".")) {
            continue;
        }
        std::string dirname = path + "/" + dp->d_name;
        ret                 = Remove(dirname);
    }
    closedir(dir);
    if (::rmdir(path.c_str())) {
        ret = false;
    }
    return ret;
}

/**
 * 重命名文件或目录
 * @param oldpath 旧路径
 * @param newpath 新路径
 * @return 成功返回true，失败返回false
 * @note 如果新路径已存在，则先删除；然后将旧路径重命名为新路径
 */
bool FSUtil::Rename(const std::string& oldpath, const std::string& newpath) {
    if (!Remove(newpath)) {
        return false;
    }
    return rename(oldpath.c_str(), newpath.c_str()) == 0;
}

/**
 * 检查指定的PID文件是否存在一个正在运行的进程
 * @param pidfile PID文件路径
 * @return 存在正在运行的进程返回true，否则返回false
 */
bool FSUtil::IsRunningPidfile(const std::string& pidfile) {
    if (__lstat(pidfile.c_str(), nullptr) != 0) {
        return false;
    }

    // 读取PID文件中的PID，并检查该PID对应的进程是否存在
    std::ifstream ifs(pidfile.c_str());
    std::string line;
    if (!ifs || !std::getline(ifs, line)) {
        return false;
    }
    if (line.empty()) {
        return false;
    }
    pid_t pid = atoi(line.c_str());
    if (pid <= 1) {
        return false;
    }
    if (kill(pid, 0) != 0) {
        return false;
    }
    return true;
}

/**
 * 删除文件
 * @param filename 文件路径
 * @param exist 是否存在
 * @return 成功返回true，失败返回false
 * @note 如果exist为false且文件不存在，则不做任何操作；否则，使用unlik函数删除文件
 */
bool FSUtil::Unlink(const std::string& filename, bool exist) {
    if (!exist && __lstat(filename.c_str(), nullptr)) {
        return true;
    }
    return ::unlink(filename.c_str()) == 0;
}

/**
 * 获取文件或目录的绝对路径
 * @param path 文件路径
 * @param realpath 输出的绝对路径
 * @return 成功返回true，失败返回false
 * @note 调用realpath函数获取文件的绝对路径，realpath函数会将符号链接转换为真实路径
 */
bool FSUtil::Realpath(const std::string& path, std::string& realpath) {
    if (__lstat(path.c_str(), nullptr)) {
        return false;
    }

    char* ptr = ::realpath(path.c_str(), nullptr);
    if (nullptr == ptr) {
        return false;
    }
    std::string(ptr).swap(realpath);
    free(ptr);
    return true;
}

/**
 * 创建符号链接
 * @param from 符号链接指向的文件
 * @param to 符号链接路径
 * @return 成功返回true，失败返回false
 * @note 如果to已存在，则先删除；然后创建符号链接
 */
bool FSUtil::Syslink(const std::string& from, const std::string& to) {
    if (!Remove(to)) {
        return false;
    }
    return ::symlink(from.c_str(), to.c_str()) == 0;
}

/**
 * 获取路径的目录部分
 * @param path 路径
 * @return 目录部分
 * @note 路径最后一个'/'之前的部分
 */
std::string FSUtil::Dirname(const std::string& path) {
    if (path.empty()) {
        return ".";
    }
    auto pos = path.rfind('/');
    if (pos == 0) {
        return "/";
    } else if (pos == std::string::npos) {
        return ".";
    } else {
        return path.substr(0, pos);
    }
}

/**
 * 获取路径的文件名部分
 * @param path 路径
 * @return 文件名部分
 * @note 路径最后一个'/'之后的部分
 */
std::string FSUtil::Basename(const std::string& path) {
    if (path.empty()) {
        return path;
    }
    auto pos = path.rfind('/');
    if (pos == std::string::npos) {
        return path;
    } else {
        return path.substr(pos + 1);
    }
}

/**
 * 打开文件进行读取
 * @param ifs 文件流
 * @param filename 文件路径
 * @param mode 打开模式
 * @return 成功返回true，失败返回false
 */
bool FSUtil::OpenForRead(std::ifstream& ifs, const std::string& filename, std::ios_base::openmode mode) {
    ifs.open(filename.c_str(), mode);
    return ifs.is_open();
}

/**
 * 打开文件进行写入
 * @param ofs 文件流
 * @param filename 文件路径
 * @param mode 打开模式
 * @return 成功返回true，失败返回false
 */
bool FSUtil::OpenForWrite(std::ofstream& ofs, const std::string& filename, std::ios_base::openmode mode) {
    ofs.open(filename.c_str(), mode);
    if (!ofs.is_open()) {
        std::string dir = Dirname(filename);
        Mkdir(dir);
        ofs.open(filename.c_str(), mode);
    }
    return ofs.is_open();
}

}  // namespace myserver
