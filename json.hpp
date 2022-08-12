#ifndef BLOCKY3D_JSON_HPP
#define BLOCKY3D_JSON_HPP
#include<unordered_map>
#include<vector>
#include<memory>
#include<string>
#include"exc.hpp"
namespace blocky{
    #define inherits : public
    enum class JsonKind{
        integer,boolean,list,hashtable,nothing
    };
    struct Index{
        int numind;//unions need weird destructors
        std::string strind;
        bool is_numerical=false;
        Index(int x){
            numind = x;
            is_numerical = true;
        }
        Index(std::string x){
            strind = x;
            is_numerical = false;
        }
    };
    struct Json{
        public:
            JsonKind kind;
            virtual std::string dump() const = 0;
            virtual std::shared_ptr<Json> operator[](Index ind) const{
                throw json_error("Indexing on json kind "+std::to_string((int)kind));
            }
            virtual std::shared_ptr<Json>& operator[](Index ind){
                throw json_error("Indexing on json kind "+std::to_string((int)kind));
            }
    };
    struct IntegerJson inherits Json{
        IntegerJson(){
            kind=JsonKind::integer;
        }
        int data;
        std::string dump() const override{
            return std::to_string(data);
        }
    };
    struct BooleanJson inherits Json{
        BooleanJson(){
            kind=JsonKind::boolean;
        }
        bool data;
        std::string dump() const override{
            return (data?"true":"false");
        }
    };
    struct ListJson inherits Json{
        ListJson(){
            kind=JsonKind::list;
        }
        std::vector<std::shared_ptr<Json>> data;
        
        std::string dump() const override{
            if(data.empty())return "[]";
            std::string str = "[";
            for(size_t i=0;i<data.size();i++){
                str += data[i]->dump();
                if((i+1)<data.size())str += ",";
            }
            return str+"]";
        }
        std::shared_ptr<Json>& operator[](Index ind) override{
            if(!ind.is_numerical)
                throw json_error("Looking up key "+ind.strind+" on list json");
            return data.at(ind.numind);
        }
        std::shared_ptr<Json> operator[](Index ind) const override{
            if(!ind.is_numerical)
                throw json_error("Looking up key "+ind.strind+" on list json");
            return data.at(ind.numind);
        }
    };
    struct DictJson inherits Json{
        DictJson(){
            kind = JsonKind::hashtable;
        }
        std::unordered_map<std::string,std::shared_ptr<Json>> data;
        std::string dump() const override{
            if(data.empty())return "{}";
            std::string str = "{";
            size_t i=0;
            for(auto& [k,v] : data){
                str += k+":"+v->dump();
                i++;
                if(i<data.size()){
                    str += ",";
                }
            }
            return str+"}";
        }
        std::shared_ptr<Json>& operator[](Index ind) override{
            if(ind.is_numerical)
                throw json_error("Indexing #"+std::to_string(ind.numind)+" on dict json");
            return data.at(ind.strind);
        }
        std::shared_ptr<Json> operator[](Index ind) const override{
            if(ind.is_numerical)
                throw json_error("Indexing #"+std::to_string(ind.numind)+" on dict json");
            return data.at(ind.strind);
        }
    };
    #undef inherits
}
#endif