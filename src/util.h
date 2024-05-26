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
// 获取系统中线程ID
pid_t GetThreadId();

// 获取协程ID
u_int32_t GetFiberId();

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

}  // namespace myserver

#endif