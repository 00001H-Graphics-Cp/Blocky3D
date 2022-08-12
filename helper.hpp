#ifndef BLOCKY3D_HELPER_HPP
#define BLOCKY3D_HELPER_HPP
#include<any>
namespace blocky{
    template<typename T,typename... Args>
    std::vector<T> vectorOf(const Args&... element){
        std::vector<T> e {{element...}};
        return e;
    }
    typedef void (*callback)(std::any);
}
#endif//BLOCKY3D_HELPER_HPP
