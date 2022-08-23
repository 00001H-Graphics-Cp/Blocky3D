#ifndef BLOCKY3D_CHUNKS_HPP
#define BLOCKY3D_CHUNKS_HPP
#include<vector>
#include<array>
#include"blocks.hpp"
namespace blocky{
    glm::vec3 inline fix_z(glm::vec3 in){
        glm::vec3 fxd = in;
        fxd.z = -fxd.z;
        return fxd;
    }
    int BUILD_LIMIT = 512;
    const constexpr int CHUNK_SIZE = 16;
    class Chunk{
        std::array<std::array<std::vector<pBlock>,CHUNK_SIZE>,CHUNK_SIZE> blocks;
        public:
            const std::optional<pBlock> maybeAt(int x,int y,int z) const{
                if((x<0)||(y<0)||(z<0))return std::optional<pBlock>();
                if((x>=CHUNK_SIZE)||(y>=BUILD_LIMIT)||(z>=CHUNK_SIZE))return std::optional<pBlock>();
                return blocks.at(x).at(z).at(y);
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
            std::optional<glm::ivec3> rayCastBad(glm::vec3 playergamepos,glm::vec3 dirvec,glm::vec3 loc_of_this,float reach){
                glm::vec3 p = playergamepos-fix_z(loc_of_this),check;
                for(float d=0;d<reach;d+=0.05){
                    check = p+dirvec*d;
                    auto bpos = get_blockpos(check);
                    if(!bpos.has_value())continue;
                    glm::ivec3 bbpos = bpos.value();
                    if(!at(bbpos.x,bbpos.y,bbpos.z)->getType()
                        ->getModel()->getHitbox().contains(check-glm::vec3(bbpos.x,bbpos.y,bbpos.z)))continue;
                    return bbpos;
                }
                return std::optional<glm::ivec3>();//no value
            }
        private:
            static glm::vec3 normalize(glm::vec3 param){
                return glm::normalize(param);
            }
            static float normalize(float param){
                return ((param<0.0)?-1.0:1.0);
            }
        public:
            std::optional<glm::ivec3> rayCastDDA(glm::vec3 srcpos,glm::vec3 rdirvec,glm::vec3 loc_of_this,float reach){
                #define roundtowards(src,dst) (((src)<(dst))?glm::ceil(src):glm::floor(src))
                #define difroundtwds(src,dst) (glm::abs(roundtowards((src),(dst))-(src)))
                #define valid(x) (!(glm::isinf(x)||glm::isnan(x)))
                #define isgn(x) (((x)<0.0)?-1:1)
                glm::vec3 dirvec = glm::normalize(rdirvec);
                srcpos = srcpos-loc_of_this;
                glm::vec3 dstpos = srcpos+rdirvec*reach;
                #define sqr(xxx) ((xxx)*(xxx))
                float incx = glm::sqrt(sqr(dirvec.y/dirvec.x)+sqr(dirvec.z/dirvec.x)+1);
                // float incy = glm::sqrt(sqr(dirvec.z/dirvec.y)+sqr(dirvec.x/dirvec.y)+1);
                float incz = glm::sqrt(sqr(dirvec.y/dirvec.z)+sqr(dirvec.x/dirvec.z)+1);
                #undef sqr
                glm::ivec3 cur_pos = glm::floor(srcpos);
                float
                dx=difroundtwds(srcpos.x,dstpos.x)*incx,
                // dy=difroundtwds(srcpos.y,dstpos.y)*incy,
                dz=difroundtwds(srcpos.z,dstpos.z)*incz,
                minit,
                dlta;
                bool hasminit = false;
                hasminit = false;
                minit = 0;
                if(dstpos.x!=srcpos.x){
                    if((dx<minit)||(!hasminit)){
                        minit = dx;
                        hasminit = true;
                    }
                // }else if(dstpos.y!=srcpos.y){
                //     if((dy<minit)||(!hasminit)){
                //         minit = dy;
                //         hasminit = true;
                //     }
                }else if(dstpos.z!=srcpos.z){
                    if((dz<minit)||(!hasminit)){
                        minit = dz;
                        hasminit = true;
                    }
                }
                if(!hasminit){
                    throw blocky_error("ERROR: No valid direction//INTERNAL FAILURE");
                }
                if(minit==dx){
                    dlta = difroundtwds(srcpos.x,dstpos.x);
                // }else if(minit==dy){
                //     dlta = difroundtwds(srcpos.y,dstpos.y);
                }else{
                    dlta = difroundtwds(srcpos.z,dstpos.z);
                }
                while(true){
                    hasminit = false;
                    minit = 0;
                    if(dstpos.x!=srcpos.x){
                        if((dx<minit)||(!hasminit)){
                            minit = dx;
                            hasminit = true;
                        }
                    // }else if(dstpos.y!=srcpos.y){
                    //     if((dy<minit)||(!hasminit)){
                    //         minit = dy;
                    //         hasminit = true;
                    //     }
                    }else if(dstpos.z!=srcpos.z){
                        if((dz<minit)||(!hasminit)){
                            minit = dz;
                            hasminit = true;
                        }
                    }
                    if(!hasminit){
                        throw blocky_error("ERROR: No valid direction//nhminit");
                    }
                    if(minit==dx){
                        cur_pos.x += isgn(dstpos.x-srcpos.x);
                        dx += incx*dlta;
                    // }else if(minit==dy){
                    //     cur_pos.y += isgn(dstpos.y-srcpos.y);
                    //     dy += incy*dlta;
                    }else if(minit==dz){
                        cur_pos.z += isgn(dstpos.z-srcpos.z);
                        dz += incz*dlta;
                    }else{
                        throw blocky_error("ERROR#floating-p//minit:mismatch");
                    }
                    dlta = 1;
                    auto x = maybeAt(cur_pos.x,cur_pos.y,cur_pos.z);
                    
                    if(glm::distance((glm::vec3)cur_pos,dstpos)>reach*5){
                        return std::optional<glm::ivec3>();
                    }
                    if(x.has_value()){
                        if(x.value()->getType()->getModel()->getHitbox().contains({0.0,0.0,0.0}))
                            return cur_pos;
                    }
                }
                #undef roundtowards
                #undef isgn
                #undef valid
                #undef difroundtwds
            }
            std::optional<glm::ivec3> get_blockpos(glm::vec3 pos){
                // std::cout << pos.x << "@" << pos.y << "@" << pos.z << std::endl;
                if((pos.x<0)||(pos.y<0)||(pos.z<0))return std::optional<glm::ivec3>();//nothing
                if((pos.x>=CHUNK_SIZE)||(pos.y>=BUILD_LIMIT)||(pos.z>=CHUNK_SIZE))return std::optional<glm::ivec3>();//nothing
                glm::ivec3 ssubpos = glm::floor(pos);
                return ssubpos;
            }
    };
}
#endif//BLOCKY3D_CHUNKS_HPP
