#ifndef BLOCKY3D_CHUNKS_HPP
#define BLOCKY3D_CHUNKS_HPP
#include<vector>
#include<array>
#include<optional>
#include"blocks.hpp"
namespace blocky{
    int BUILD_LIMIT = 512;
    const constexpr int CHUNK_SIZE = 16;
    class Collision;
    std::optional<glm::ivec3> inline get_blockpos(glm::vec3 pos){
        if((pos.x<0)||(pos.y<0)||(pos.z<0))return std::optional<glm::ivec3>();//nothing
        if((pos.x>=CHUNK_SIZE)||(pos.y>=BUILD_LIMIT)||(pos.z>=CHUNK_SIZE))return std::optional<glm::ivec3>();//nothing
        glm::ivec3 ssubpos = glm::floor(pos);
        return ssubpos;
    }
    class Chunk;
    typedef std::shared_ptr<Chunk> pChunk;
    class Chunk{
        std::array<std::array<std::vector<pBlock>,CHUNK_SIZE>,CHUNK_SIZE> blocks;
        public:
            static inline bool inbounds(int x,int y,int z);
            static inline bool inbounds(glm::ivec3 p);
            glm::vec3 chunkpos=glm::vec3(0.0);
            const inline pBlock at(int x,int y,int z) const;
            const inline pBlock at(glm::ivec3 pos) const;
            const inline std::optional<pBlock> maybeAt(int x,int y,int z) const;
            const inline std::optional<pBlock> maybeAt(int x,int y,int z,pChunk l,pChunk r,pChunk f,pChunk b) const;
            const inline std::optional<pBlock> maybeAt(glm::ivec3 pos) const;
            Chunk(pBlock airBlock);
            Chunk(pBlockType airType) : Chunk(std::make_shared<Block>(airType)){}
            void inline set(int x,int y,int z,pBlock tgrt);
            void inline set(glm::ivec3 pos,pBlock tgrt);
            void inline display(pChunk left,pChunk right,pChunk front,pChunk back) const;
            void inline displayNoCull(glm::vec3 pos) const;
        public:
            std::optional<Collision> rayCast(glm::vec3 src,glm::vec3 rdirvec,float reach);
    };
    inline bool Chunk::inbounds(int x,int y,int z){
        if((x<0)||(y<0)||(z<0))return false;
        if((x>=CHUNK_SIZE)||(y>=BUILD_LIMIT)||(z>=CHUNK_SIZE))return false;
        return true;
    }
    inline bool Chunk::inbounds(glm::ivec3 p){
        return inbounds(p.x,p.y,p.z);
    }
    const pBlock Chunk::at(int x,int y,int z) const{
        if(!inbounds(x,y,z))throw blocky_error("Chunk::at() out of bounds");
        return blocks.at(x).at(z).at(y);
    }
    const pBlock Chunk::at(glm::ivec3 pos) const{
        return at(pos.x,pos.y,pos.z);
    }
    Chunk::Chunk(pBlock airBlock){
        for(auto& column : blocks){
            for(std::vector<pBlock>& column2 : column){
                column2.reserve(BUILD_LIMIT);
                for(int i=0;i<BUILD_LIMIT;i++){
                    column2.push_back(copyBlock(airBlock));
                }
            }
        }
    }
    const std::optional<pBlock> Chunk::maybeAt(int x,int y,int z) const{
        if(inbounds(x,y,z))
            return at(x,y,z);
        return std::optional<pBlock>();
    }
    const std::optional<pBlock> Chunk::maybeAt(
            int x,int y,int z,pChunk l,pChunk r,pChunk f,pChunk b) const{
        if(inbounds(x,y,z))
            return at(x,y,z);
        if(!inbounds(0,y,0)/*y wrong*/){
            return std::optional<pBlock>();
        }
        if((l!=nullptr)&&inbounds(x+CHUNK_SIZE,y,z)){
            return l->at(x+CHUNK_SIZE,y,z);
        }
        if((r!=nullptr)&&inbounds(x-CHUNK_SIZE,y,z)){
            return r->at(x-CHUNK_SIZE,y,z);
        }
        if((b!=nullptr)&&inbounds(x,y,z+CHUNK_SIZE)){
            return b->at(x,y,z+CHUNK_SIZE);
        }
        if((f!=nullptr)&&inbounds(x,y,z-CHUNK_SIZE)){
            return f->at(x,y,z-CHUNK_SIZE);
        }
        return std::optional<pBlock>();
    }
    const std::optional<pBlock> Chunk::maybeAt(glm::ivec3 pos) const{
        return maybeAt(pos.x,pos.y,pos.z);
    }
    void inline Chunk::set(int x,int y,int z,pBlock tgrt){
        if(!inbounds(x,y,z))throw blocky_error("Chunk::set out of bounds");
        blocks[x][z][y] = tgrt;
    }
    void Chunk::set(glm::ivec3 pos,pBlock tgrt){
        set(pos.x,pos.y,pos.z,tgrt);
    }
    void Chunk::display(pChunk left=nullptr,pChunk right=nullptr,pChunk front=nullptr,pChunk back=nullptr) const{
        #define _getblock(x,y,z) (maybeAt(x,y,z,left,right,front,back).value_or(nullptr))
        #define getblock(x,y,z) _getblock((x),(y),(z))
        for(int x=0;x<CHUNK_SIZE;x++){
            for(int z=0;z<CHUNK_SIZE;z++){
                for(int y=0;y<BUILD_LIMIT;y++){
                    at(x,y,z)->display(chunkpos+glm::vec3(x,y,-z-1.0)*BLOCK_SIZE_GL,
                    getblock(x,y,z-1),getblock(x,y,z+1),
                    getblock(x-1,y,z),getblock(x+1,y,z),
                    getblock(x,y+1,z),getblock(x,y-1,z));
                }
            }
        }
        #undef _getblock
        #undef getblock
    }
    void Chunk::displayNoCull(glm::vec3 pos) const{
        for(int x=0;x<CHUNK_SIZE;x++){
            for(int z=0;z<CHUNK_SIZE;z++){
                for(int y=0;y<BUILD_LIMIT;y++){
                    at(x,y,z)->display_singleblock(pos+glm::vec3(x,y,-z-1.0)*BLOCK_SIZE_GL);
                }
            }
        }
    }
    class Collision{
        glm::ivec3 blockpos;
        glm::vec3 cpos;
        friend class Chunk;
        Direction face;
        public:
            Collision(glm::ivec3 bp,glm::vec3 cp,Direction face) : blockpos(bp), cpos(cp), face(face){}
            glm::ivec3 getTouchingBlock() const{
                return blockpos;
            }
            Direction getFace() const{
                return face;
            }
            glm::vec3 getTouchingPos() const{
                return cpos;
            }
    };
    std::optional<Collision> genericRayCast(std::optional<bool>(*getcb)(glm::ivec3,void*),void *usrptr,glm::vec3 src,glm::vec3 rdirvec,float reach){
        if(getcb(glm::floor(src),usrptr).value_or(false)){
            return Collision(glm::floor(src),src,Direction::NONE);
        }
        std::optional<bool> mbe;
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
        glm::ivec3 last_inves = investigating;
        mbe = getcb(investigating,usrptr);
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
            last_inves = investigating;
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
            mbe = getcb(investigating,usrptr);
            if(mbe.value_or(false))return Collision(investigating,src+direction*dist,Direction::from_ray(investigating,last_inves));
        }
        return std::optional<Collision>();
    }
    glm::vec3 inline fix_z(glm::vec3 in){
        return {in.x,in.y,-in.z};
    }
    namespace{
        std::optional<bool> inline _chnk_getter_16(glm::ivec3 pos,void *usrptr){
            auto cptr = static_cast<Chunk*>(usrptr);
            glm::vec3 fpos(pos);
            std::optional<pBlock> optblk = cptr->maybeAt(glm::floor(fpos/16.0f));
            if(!optblk.has_value()){
                return std::optional<bool>();
            }
            return optblk.value()->getModel()->phys_at(glm::floor(glm::fract(fpos/16.0f)*16.0f));
        }
    }
    std::optional<Collision> Chunk::rayCast(glm::vec3 src,glm::vec3 rdirvec,float reach){
        std::optional<Collision> c = genericRayCast(_chnk_getter_16,this,(src-chunkpos)*16.0f,rdirvec,reach*16.0f);
        if(!c.has_value())return std::optional<Collision>();
        Collision coll = c.value();
        coll.blockpos = glm::floor(glm::vec3(coll.blockpos)/16.0f);
        coll.cpos /= 16.0f;
        return coll;
    }
}
#endif//BLOCKY3D_CHUNKS_HPP
