#ifndef BLOCKY3D_EXC_HPP
#define BLOCKY3D_EXC_HPP
#include<exception>
#include<string>
namespace blocky{
    #define inherits : public
    class blocky_error inherits std::exception{
        public:
            std::string reason;
            blocky_error(std::string rson) : reason(rson){}
            const char* what() const noexcept override{
                return reason.c_str();
            }
    };
    class json_error inherits blocky_error{
        using blocky_error::blocky_error;
    };
    #undef inherits
}
#endif