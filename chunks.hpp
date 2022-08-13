#ifndef BLOCKY3D_CHUNKS_HPP
#define BLOCKY3D_CHUNKS_HPP
#include<vector>
#include<array>
#include"blocks.hpp"
namespace blocky{
    int BUILD_LIMIT = 512;
    const constexpr int CHUNK_SIZE = 16;
    class Chunk{
        std::array<std::array<std::vector<pBlock>,CHUNK_SIZE>,CHUNK_SIZE> blocks;
        public:
            const pBlock maybeAt(int x,int y,int z) const{
                if((x<0)||(y<0)||(z<0))return nullptr;
                if((x>=CHUNK_SIZE)||(y>=BUILD_LIMIT)||(z>=CHUNK_SIZE))return nullptr;
                return blocks.at(x).at(z).at(y);
            }
            const pBlock at(int x,int y,int z) const{
                pBlock pb = maybeAt(x,y,z);
                if(pb==nullptr){
                    throw std::out_of_range("Invalid block "+std::to_string(x)
                    +"@"+std::to_string(y)+"@"+std::to_string(z));
                }
                return pb;
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
                            maybeAt(x,y,z-1),maybeAt(x,y,z+1),
                            maybeAt(x-1,y,z),maybeAt(x+1,y,z),
                            maybeAt(x,y+1,z),maybeAt(x,y-1,z));
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
            std::optional<glm::ivec3> rayCast(glm::vec3 playergamepos,glm::vec3 gamelookvec,glm::vec3 loc_of_this,float reach){
                glm::vec3 p = playergamepos-loc_of_this,check;
                for(float d=0;d<reach;d+=0.05){
                    check = p+gamelookvec*d;
                    auto bpos = get_blockpos(check);
                    if(!bpos.has_value())continue;
                    glm::ivec3 bbpos = bpos.value();
                    if(!at(bbpos.x,bbpos.y,bbpos.z)->getType()
                        ->getModel()->getHitbox().contains(check-glm::vec3(bbpos.x,bbpos.y,-bbpos.z)))continue;
                    return bbpos;
                }
                return std::optional<glm::ivec3>();//no value
            }
            std::optional<glm::ivec3> get_blockpos(glm::vec3 pos){
                // std::cout << pos.x << "@" << pos.y << "@" << pos.z << std::endl;
                if((pos.x<0)||(pos.y<0)||(pos.z<0))return std::optional<glm::ivec3>();//nothing
                if((pos.x>=CHUNK_SIZE)||(pos.y>=BUILD_LIMIT)||(pos.z>=CHUNK_SIZE))return std::optional<glm::ivec3>();//nothing
                glm::ivec3 ssubpos = glm::trunc(pos);
                return ssubpos;
            }
    };
}
#endif//BLOCKY3D_CHUNKS_HPP
