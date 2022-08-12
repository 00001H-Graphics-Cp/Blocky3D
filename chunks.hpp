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
            std::array<std::vector<pBlock>,CHUNK_SIZE>& operator[](size_t ind){
                return blocks.at(ind);
            }
            const std::array<std::vector<pBlock>,CHUNK_SIZE>& operator[](size_t ind) const{
                return blocks.at(ind);
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
            void display(glm::vec3 pos) const{
                for(int x=0;x<CHUNK_SIZE;x++){
                    for(int z=0;z<CHUNK_SIZE;z++){
                        for(int y=0;y<BUILD_LIMIT;y++){
                            blocks[x][z][y]->display(pos+glm::vec3(x,y,z)*BLOCK_SIZE_GL);
                        }
                    }
                }
            }
    };
}
#endif//BLOCKY3D_CHUNKS_HPP
