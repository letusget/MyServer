/**
 * @file util.h
 * @author william
 * @date 2024-01-28
 * @version 0.1
 * @brief 日志工具类
 */
#ifndef __MYLOG_UTIL_H__
#define __MYLOG_UTIL_H__

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <vector>

namespace myserver {

/**
 * @brief 获取系统中线程ID
 * @return pid_t
 * @note 线程ID是每个线程的唯一标识，每个线程在创建时都会分配一个唯一的ID，该ID在系统内唯一
 * @note
 * 这里使用syscall(SYS_gettid)获取线程ID，它是Linux系统调用，它返回的是当前线程的唯一标识符，不同进程间的线程ID可能相同，但它保证在同一个进程中是唯一的。
 */
pid_t GetThreadId();

/**
 * @brief 获取协程ID
 * @return u_int32_t
 * @note 协程ID是每个线程的唯一标识，每个协程在创建时都会分配一个唯一的ID，该ID在线程内唯一，不同线程的协程ID可能相同
 */
u_int32_t GetFiberId();

// 文件系统工具类
class FSUtil {
   public:
    static void ListAllFiles(std::vector<std::string>& files, const std::string& path, const std::string& subfix);
    static bool Mkdir(const std::string& path);
    static bool Remove(const std::string& path);
    static bool Rename(const std::string& oldpath, const std::string& newpath);
    static bool IsRunningPidfile(const std::string& pidfile);
    static bool Realpath(const std::string& path, std::string& realpath);
    static bool Syslink(const std::string& from, const std::string& to);
    static bool Unlink(const std::string& filename, bool exist = false);
    static std::string Dirname(const std::string& path);
    static std::string Basename(const std::string& path);
    static bool OpenForRead(std::ifstream& ifs, const std::string& filename, std::ios_base::openmode mode);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& filename, std::ios_base::openmode mode);
};

/**
 * @brief 获取程序异常信息
 * @param bt 异常信息
 * @param size 最大栈帧数
 * @param skip 跳过的栈帧数，这里默认跳过1，因为栈帧0是当前函数，1是调用者函数
 * @return void
 */
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);

/**
 * @brief 直接获取程序异常信息字符串
 * @param size 最大栈帧数
 * @param skip 跳过的栈帧数，这里默认为2，因为栈帧0是当前函数，1是调用者函数
 * @param prefix 可选的前缀
 * @return std::string
 */
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

}  // namespace myserver

#endif  // __MYLOG_UTIL_H__