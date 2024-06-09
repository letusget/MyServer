#ifndef __MYLOG_SINGLETON_H__
#define __MYLOG_SINGLETON_H__

namespace mylog {

template <class T, class X = void, int N = 0>
class Singleton {
   public:
    static T* GetInstance() {
        static T v;
        return &v;
    }
};

template <class T, class X = void, int N = 0>
class SingletonPtr {
   public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

}  // namespace mylog

#endif  // __MYLOG_SINGLETON_H__