#ifndef BLOCKY3D_CHUNKS_HPP
#define BLOCKY3D_CHUNKS_HPP
#include<vector>
#include<array>
#include<optional>
#include"blocks.hpp"
namespace blocky{
    class Collision{
        glm::ivec3 blockpos;
        glm::vec3 cpos;
        public:
            Collision(glm::ivec3 bp,glm::vec3 cp) : blockpos(bp), cpos(cp){}
            glm::ivec3 getTouchingBlock() const{
                return blockpos;
            }
            glm::vec3 getTouchingPos() const{
                return cpos;
            }
    };
    glm::vec3 inline fix_z(glm::vec3 in){
        glm::vec3 fxd = in;
        fxd.z = -fxd.z;
        return fxd;
    }
    int BUILD_LIMIT = 512;
    const constexpr int CHUNK_SIZE = 16;
    class Chunk{
        std::array<std::array<std::vector<pBlock>,CHUNK_SIZE>,CHUNK_SIZE> blocks;
        static bool inbounds(int x,int y,int z){
            if((x<0)||(y<0)||(z<0))return false;
            if((x>=CHUNK_SIZE)||(y>=BUILD_LIMIT)||(z>=CHUNK_SIZE))return false;
            return true;
        }
        public:
            const std::optional<pBlock> maybeAt(int x,int y,int z) const{
                if(inbounds(x,y,z))
                    return blocks.at(x).at(z).at(y);
                return std::optional<pBlock>();
            }
            const std::optional<pBlock> maybeAt(glm::ivec3 pos) const{
                return maybeAt(pos.x,pos.y,pos.z);
            }
            const pBlock at(int x,int y,int z) const{
                auto pb = maybeAt(x,y,z);
                if(!pb.has_value()){
                    throw std::out_of_range("Invalid block "+std::to_string(x)
                    +"@"+std::to_string(y)+"@"+std::to_string(z));
                }
                return pb.value();
            }
            Chunk(pBlock airBlock){
                for(auto& column : blocks){
                    for(std::vector<pBlock>& column2 : column){
                        column2.reserve(BUILD_LIMIT);
                        for(int i=0;i<BUILD_LIMIT;i++){
                            column2.push_back(copyBlock(airBlock));
                        }
                    }
                }
            }
            Chunk(pBlockType airType) : Chunk(std::make_shared<Block>(airType)){}
            void set(int x,int y,int z,pBlock tgrt){
                at(x,y,z);//check in-range
                blocks[x][z][y] = tgrt;
            }
            void display(glm::vec3 pos) const{
                for(int x=0;x<CHUNK_SIZE;x++){
                    for(int z=0;z<CHUNK_SIZE;z++){
                        for(int y=0;y<BUILD_LIMIT;y++){
                            at(x,y,z)->display(pos+glm::vec3(x,y,-z-1.0)*BLOCK_SIZE_GL,
                            maybeAt(x,y,z-1).value_or(nullptr),maybeAt(x,y,z+1).value_or(nullptr),
                            maybeAt(x-1,y,z).value_or(nullptr),maybeAt(x+1,y,z).value_or(nullptr),
                            maybeAt(x,y+1,z).value_or(nullptr),maybeAt(x,y-1,z).value_or(nullptr));
                        }
                    }
                }
            }
            void displayNoCull(glm::vec3 pos) const{
                for(int x=0;x<CHUNK_SIZE;x++){
                    for(int z=0;z<CHUNK_SIZE;z++){
                        for(int y=0;y<BUILD_LIMIT;y++){
                            at(x,y,z)->display_singleblock(pos+glm::vec3(x,y,-z-1.0)*BLOCK_SIZE_GL);
                        }
                    }
                }
            }
        private:
            static glm::vec3 normalize(glm::vec3 param){
                return glm::normalize(param);
            }
            static float normalize(float param){
                return ((param<0.0)?-1.0:1.0);
            }
        public:
            std::optional<Collision> rayCast(glm::vec3 src,glm::vec3 rdirvec,glm::vec3 loc_of_this,float reach){
                std::optional<pBlock> mbe;
                src -= loc_of_this;
                glm::vec3 dst = src+rdirvec*reach;
                const float dx = abs(src.x-dst.x);
                const float dy = abs(src.y-dst.y);
                const float dz = abs(src.z-dst.z);
                glm::vec3 direction = normalize(dst-src);
                #define square(expr) ((expr)*(expr))
                float incx = sqrt(square(dy/dx)+square(dz/dx)+1);
                float incy = sqrt(square(dx/dy)+square(dz/dy)+1);
                float incz = sqrt(square(dx/dz)+square(dy/dz)+1);
                #undef square
                int ixpp,iypp,izpp;
                float lx,ly,lz;
                glm::ivec3 investigating = glm::floor(src);
                mbe = maybeAt(investigating.x,investigating.y,investigating.z);
                if(direction.x<0){
                    ixpp = -1;
                    lx = (src.x-static_cast<float>(investigating.x))*incx;
                }else{
                    ixpp = 1;
                    lx = (static_cast<float>(investigating.x+1)-src.x)*incx;
                }
                if(direction.y<0){
                    iypp = -1;
                    ly = (src.y-static_cast<float>(investigating.y))*incy;
                }else{
                    iypp = 1;
                    ly = (static_cast<float>(investigating.y+1)-src.y)*incy;
                }
                if(direction.z<0){
                    izpp = -1;
                    lz = (src.z-static_cast<float>(investigating.z))*incz;
                }else{
                    izpp = 1;
                    lz = (static_cast<float>(investigating.z+1)-src.z)*incz;
                }
                //Is there a collision?
                bool kaboom = false;
                //How far have we got?
                float dist = 0.0;
                //Total distance
                float tdist = glm::length(dst-src);
                float minthat;
                while(!kaboom && dist<tdist){
                    minthat = glm::min(lx,glm::min(ly,lz));
                    if(lx==minthat){
                        investigating.x += ixpp;
                        dist = lx;
                        lx += incx;
                    }else if(ly==minthat){
                        investigating.y += iypp;
                        dist = ly;
                        ly += incy;
                    }else{
                        investigating.z += izpp;
                        dist = lz;
                        lz += incz;
                    }
                    mbe = maybeAt(investigating.x,investigating.y,investigating.z);
                    if(mbe.has_value()){
                        if(mbe.value()->getType()->getModel()->contains({0.5,0.5,0.5})){
                            return Collision(investigating,src+direction*dist);
                        }
                    }
                }
                return std::optional<Collision>();
            }
            std::optional<Collision> rayCast4096vox(glm::vec3 src,glm::vec3 rdirvec,glm::vec3 loc_of_this,float reach){
                std::optional<pBlock> mbe;
                src -= loc_of_this;
                glm::vec3 dst = src+rdirvec*reach;
                const float dx = abs(src.x-dst.x);
                const float dy = abs(src.y-dst.y);
                const float dz = abs(src.z-dst.z);
                glm::vec3 direction = normalize(dst-src);
                #define square(expr) ((expr)*(expr))
                float incx = sqrt(square(dy/dx)+square(dz/dx)+1);
                float incy = sqrt(square(dx/dy)+square(dz/dy)+1);
                float incz = sqrt(square(dx/dz)+square(dy/dz)+1);
                #undef square
                int ixpp,iypp,izpp;
                float lx,ly,lz;
                glm::ivec3 investigating = glm::floor(src*16.0f);
                glm::ivec3 investigating_fullblock = glm::floor(src);
                mbe = maybeAt(investigating_fullblock);
                if(direction.x<0){
                    ixpp = -1;
                    lx = (src.x-static_cast<float>(investigating.x))*incx;
                }else{
                    ixpp = 1;
                    lx = (static_cast<float>(investigating.x+1)-src.x)*incx;
                }
                if(direction.y<0){
                    iypp = -1;
                    ly = (src.y-static_cast<float>(investigating.y))*incy;
                }else{
                    iypp = 1;
                    ly = (static_cast<float>(investigating.y+1)-src.y)*incy;
                }
                if(direction.z<0){
                    izpp = -1;
                    lz = (src.z-static_cast<float>(investigating.z))*incz;
                }else{
                    izpp = 1;
                    lz = (static_cast<float>(investigating.z+1)-src.z)*incz;
                }
                //Is there a collision?
                bool kaboom = false;
                //How far have we got?
                float dist = 0.0;
                //Total distance
                float tdist = glm::length(dst-src)*16.0f;
                float minthat;
                glm::ivec3 subvox;
                while(!kaboom && dist<tdist){
                    minthat = glm::min(lx,glm::min(ly,lz));
                    if(lx==minthat){
                        investigating.x += ixpp;
                        dist = lx;
                        lx += incx;
                    }else if(ly==minthat){
                        investigating.y += iypp;
                        dist = ly;
                        ly += incy;
                    }else{
                        investigating.z += izpp;
                        dist = lz;
                        lz += incz;
                    }
                    investigating_fullblock = glm::floor(static_cast<glm::vec3>(investigating)/16.0f);
                    mbe = maybeAt(investigating_fullblock);
                    subvox = glm::floor(glm::fract(static_cast<glm::vec3>(investigating)/16.0f)*16.0f);
                    if(mbe.has_value()){
                        if(mbe.value()->getType()->getModel()->phys_at(subvox)){
                            return Collision(static_cast<glm::vec3>(investigating)/16.0f,src+direction*dist);
                        }
                    }
                }
                return std::optional<Collision>();
            }
            std::optional<glm::ivec3> get_blockpos(glm::vec3 pos){
                if((pos.x<0)||(pos.y<0)||(pos.z<0))return std::optional<glm::ivec3>();//nothing
                if((pos.x>=CHUNK_SIZE)||(pos.y>=BUILD_LIMIT)||(pos.z>=CHUNK_SIZE))return std::optional<glm::ivec3>();//nothing
                glm::ivec3 ssubpos = glm::floor(pos);
                return ssubpos;
            }
    };
}
#endif//BLOCKY3D_CHUNKS_HPP
