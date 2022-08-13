#ifndef BLOCKY3DBLOCKS
#define BLOCKY3DBLOCKS
#include<memory>
#include<string>
#include<vector>
#include<unordered_map>
#include<gsdl.hpp>
#include"helper.hpp"
#include"json.hpp"
#include"textures.hpp"
#include<glm/glm.hpp>
namespace blocky{
    #define inrng(x,lwr,upr) ((x<upr)&&(x>lwr))
    class Hitbox{
        glm::vec3 pos_;
        glm::vec3 dims;
        public:
            Hitbox() = default;
            Hitbox(glm::vec3 pos,glm::vec3 dims) : pos_(pos), dims(dims) {}
            glm::vec3 getpos() const{return pos_;}
            glm::vec3& pos(){return pos_;}
            glm::vec3 getdims() const{return dims;}
            bool contains(glm::vec3 point) const{
                return inrng(point.x,pos_.x,pos_.x+dims.x)
                    && inrng(point.y,pos_.y,pos_.y+dims.y)
                    && inrng(point.z,pos_.z,pos_.z+dims.z);
            }
            bool intersects(Hitbox other) const{
                return (pos_.x<(other.pos_.x+other.dims.x))&&
                       (pos_.y<(other.pos_.y+other.dims.y))&&
                       (pos_.z<(other.pos_.z+other.dims.z))&&
                       (other.pos_.x<(pos_.x+dims.x))&&
                       (other.pos_.y<(pos_.x+dims.y))&&
                       (other.pos_.z<(pos_.x+dims.z));
            }
    };
    #undef inrng
    float BLOCK_SIZE_GL = 1.0;
    enum class Rotatability{
        NONE,ALL,HORIZONTAL,VERTICAL,AXES
    };
    class Direction{
        public:
            enum _Direction{
                FRONT=0,BACK=1,LEFT=2,RIGHT=3,TOP=4,BOTTOM=5
            };
            Direction() = default;
            Direction(_Direction dir) : _direc(dir){}
            constexpr operator _Direction() const{
                return _direc;
            }
            explicit operator bool() const = delete;
            constexpr Direction opposite() const{
                switch(_direc){
                    case FRONT:return BACK;
                    case BACK:return FRONT;
                    case LEFT:return RIGHT;
                    case RIGHT:return LEFT;
                    case TOP:return BOTTOM;
                    case BOTTOM:return TOP;
                    default:throw blocky_error("Internal Error on line "+std::to_string(__LINE__));
                }
            }
        private:
            _Direction _direc;
    };
    class BlockState;
    class BlockTypeBuilder;
    class Model;
    class BlockType;
    class Block;
    class SolidModel;
    class OneFaceModel;
    class NothingModel;
    typedef std::shared_ptr<BlockState> pBlockState;
    typedef std::shared_ptr<Model> pModel;
    typedef std::shared_ptr<SolidModel> pSolidModel;
    typedef std::shared_ptr<BlockType> pBlockType;
    typedef std::shared_ptr<Block> pBlock;
    typedef std::shared_ptr<OneFaceModel> pOneFaceModel;
    typedef std::shared_ptr<NothingModel> pNothingModel;
    class BlockType{
        private:
            friend class BlockTypeBuilder;
            Rotatability rottype;
            std::shared_ptr<DictJson> CustomProperties;
            pModel model;
            pBlockState defaultState;
            std::vector<pAnimatedTexture> texture_storage;
            std::string id;
        public:
            static pBlockType constructWithId(std::string id){
                pBlockType bt = std::make_shared<BlockType>();
                bt->id = id;
                return bt;
            }
            BlockType(const BlockType& other){
                rottype = other.rottype;
                CustomProperties = other.CustomProperties;
                model = other.model;
                defaultState = other.defaultState;
                id = other.id;
            }
            BlockType(){
                defaultState = std::make_shared<BlockState>();
            }
            std::string getID() const{
                return id;
            }
            Rotatability getRotatability() const{
                return rottype;
            }
            const std::shared_ptr<DictJson>& getProperties() const{
                return CustomProperties;
            }
            const pBlockState& getDefaultState() const{
                return defaultState;
            }
            const pModel& getModel() const{
                return model;
            }
            const std::vector<pAnimatedTexture>& getTextures() const{
                return texture_storage;
            }
    };
    class Block{
        pBlockType type;
        pBlockState state;
        public:
            Block(pBlockType typ){
                type = typ;
                state = typ->getDefaultState();
            }
            const pBlockType& getType() const{
                return type;
            }
            const pBlockState& getState() const{
                return state;
            }
            void inline display_singleblock(glm::vec3 pos) const;
            void inline display(glm::vec3 pos,
                pBlock infront=nullptr,pBlock atback=nullptr,
                pBlock onleft=nullptr,pBlock onright=nullptr,
                pBlock ontop=nullptr,pBlock atbottom=nullptr) const;
        };
    class BlockState{
        std::shared_ptr<DictJson> state;
        public:
            std::shared_ptr<Json>& operator[](std::string key){
                return (*state)[key];
            }
            const std::shared_ptr<Json>& operator[](std::string key) const{
                return (*state)[key];
            }
    };
    class Model{
        public:
            virtual bool isFaceTransparent(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt) const = 0;
            virtual void drawFace(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt,glm::vec3 loc) const = 0;
            virtual bool shouldCullFace(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt,
            pBlock next_to) const{
//******CANDIDATE FOR CLEANUP//TRAIN WRECKS(a.getB().getC().doSomething())
                if(next_to==nullptr)return false;
                return !(
(next_to->getType()->getModel()->
isFaceTransparent(
face.opposite(),next_to->getState(),
next_to->getType()->getTextures()))
&&(
next_to->getType()->getModel()->isFaceTransparent(face,next_to->getState(),
next_to->getType()->getTextures()))
);
            }
            virtual void drawFaceIfNeccesary(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt,glm::vec3 loc,
            pBlock next_to) const{
                if(!shouldCullFace(face,bs,tt,next_to))drawFace(face,bs,tt,loc);
                else{
                    if(next_to!=nullptr)return;
                    #include<iostream>
                    std::cout << "Debug: Face Culled";
                    throw blocky_error("FACE CULLED: DEBUG EXCEPTION(trapped)");
                }
            }
            virtual void draw(pBlockState bs,
            const std::vector<pAnimatedTexture>& tt,glm::vec3 loc) const{
                drawFace(Direction::FRONT,bs,tt,loc);
                drawFace(Direction::BACK,bs,tt,loc);
                drawFace(Direction::LEFT,bs,tt,loc);
                drawFace(Direction::RIGHT,bs,tt,loc);
                drawFace(Direction::TOP,bs,tt,loc);
                drawFace(Direction::BOTTOM,bs,tt,loc);
            }
            virtual void drawIfNeccesary(pBlockState bs,
            const std::vector<pAnimatedTexture>& tt,glm::vec3 loc,
            pBlock front,pBlock back,pBlock left,pBlock right,pBlock top,pBlock bottom)
            const{
                drawFaceIfNeccesary(Direction::FRONT,bs,tt,loc,front);
                drawFaceIfNeccesary(Direction::BACK,bs,tt,loc,back);
                drawFaceIfNeccesary(Direction::LEFT,bs,tt,loc,left);
                drawFaceIfNeccesary(Direction::RIGHT,bs,tt,loc,right);
                drawFaceIfNeccesary(Direction::TOP,bs,tt,loc,top);
                drawFaceIfNeccesary(Direction::BOTTOM,bs,tt,loc,bottom);
            };
            virtual Hitbox getHitbox() const = 0;
            virtual ~Model() = default;
    };
    class SolidModel : public Model{
        public:
            virtual Hitbox getHitbox() const override{
                return Hitbox(glm::vec3(0.0),glm::vec3(1.0));
            }
            virtual const pAnimatedTexture& faceOf(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt) const{
                return tt.at(static_cast<int>(face));
            }
            virtual bool isFaceTransparent(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt) const override{
                return faceOf(face,bs,tt)->isTransparent();
            }
            virtual void drawFace(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt,glm::vec3 loc) const override{
                glm::vec3 bleft,bright,tleft,tright;
                bleft = loc;
                switch(face){
                    case Direction::FRONT:[[fallthrough]];
                    case Direction::BACK:{
                        if(face==Direction::FRONT){
                            bleft.z += BLOCK_SIZE_GL;
                        }
                        bright = bleft;
                        bright.x += BLOCK_SIZE_GL;
                        tleft = bleft;
                        tleft.y += BLOCK_SIZE_GL;
                        tright = tleft;
                        tright.x += BLOCK_SIZE_GL;
                        if(face==Direction::FRONT){
                            std::swap(bright,bleft);
                            std::swap(tright,tleft);
                        }
                        break;
                    };
                    case Direction::TOP:[[fallthrough]];
                    case Direction::BOTTOM:{
                        if(face==Direction::TOP){
                            bleft.y += BLOCK_SIZE_GL;
                        }
                        bleft.z += BLOCK_SIZE_GL;
                        bright = bleft;
                        bright.x += BLOCK_SIZE_GL;
                        tleft = bleft;

                        tleft = loc;
                        tleft.y = bleft.y;

                        tright = tleft;
                        tright.x += BLOCK_SIZE_GL;
                        if(face==Direction::TOP){
                            std::swap(bright,bleft);
                            std::swap(tright,tleft);
                        }else if(face==Direction::BOTTOM){
                            std::swap(bleft,tright);
                            std::swap(tleft,bright);
                        }
                        break;
                    };
                    case Direction::RIGHT:[[fallthrough]];
                    case Direction::LEFT:{
                        if(face==Direction::RIGHT){
                            bleft.x += BLOCK_SIZE_GL;
                        }
                        bright = bleft;
                        bright.z += BLOCK_SIZE_GL;
                        tleft = bleft;
                        tleft.y += BLOCK_SIZE_GL;
                        tright = tleft;
                        tright.z += BLOCK_SIZE_GL;
                        if(face==Direction::LEFT){
                            std::swap(bright,bleft);
                            std::swap(tright,tleft);
                        }
                        break;
                    };
                    default:{
                        throw blocky_error("Internal Error at lineno "+std::to_string(__LINE__));
                    }
                }
                auto r3d = std::make_shared<pygame::geometry::Rect3D>(
                bleft,bright,tleft,tright
                );
                pygame::draw::rect3D(r3d,faceOf(face,bs,tt)->curFrame());
            }
    };
    class OneFaceModel : public SolidModel{
virtual const pAnimatedTexture& faceOf(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt) const override{
                return tt.at(0);
            }
    };
    class NothingModel : public Model{
            virtual Hitbox getHitbox() const override{
                return Hitbox(glm::vec3(0.0),glm::vec3(0.0));
            }
            virtual bool isFaceTransparent(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt) const override{
                return true;
            }
            virtual void drawFace(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt,glm::vec3 loc) const override{
                return;
            }
    };
    pSolidModel SOLID_MODEL = std::make_shared<SolidModel>();
    pOneFaceModel ONEFACE_MODEL = std::make_shared<OneFaceModel>();
    pNothingModel NOTHING_MODEL = std::make_shared<NothingModel>();
    
    #define pat pAnimatedTexture
    std::vector<pAnimatedTexture> inline makeSolidTextures(pat front,pat back,pat left,pat right,pat top,pat bottom){
        return vectorOf<pat>(front,back,left,right,top,bottom);
    }
    #define prt pygame::prTexture
    #define mks(xxx) std::make_shared<AnimatedTexture>(xxx)
    std::vector<pAnimatedTexture> inline makeSolidTextures(prt front,prt back,prt left,prt right,prt top,prt bottom){
        return vectorOf<pat>(
            mks(front),
            mks(back),
            mks(left),
            mks(right),
            mks(top),
            mks(bottom)
        );
    }
    #undef mks
    #undef prt
    #undef pat
    namespace{
        std::unordered_map<std::string,pBlockType> block_registry;
    }
    const pBlockType DEFAULT_BLOCKTYPE = BlockType::constructWithId("blocky3d:placeholder");
    void registerBlock(pBlockType bt){
        block_registry.insert_or_assign(bt->getID(),bt);
    }
    pBlockType getBlockType(std::string id){
        if(id=="blocky:placeholder"){
            return DEFAULT_BLOCKTYPE;
        }
        return block_registry.at(id);
    }
    class BlockTypeBuilder{
        pBlockType thetype;
        public:
            BlockTypeBuilder(pBlockType blocktypebase=DEFAULT_BLOCKTYPE){
                thetype = blocktypebase;
            }
            BlockTypeBuilder(std::string id){
                thetype = std::make_shared<BlockType>(*DEFAULT_BLOCKTYPE);
                thetype->id = id;
            }
            BlockTypeBuilder& id(std::string id){
                thetype->id = id;
                return *this;
            }
            BlockTypeBuilder& rotatability(Rotatability rotat){
                thetype->rottype = rotat;
                return *this;
            }
            BlockTypeBuilder& model(pModel mt){
                thetype->model = mt;
                return *this;
            }
            BlockTypeBuilder& textures(std::vector<pAnimatedTexture> textures){
                thetype->texture_storage = textures;
                return *this;
            }
            pBlockType finalize() const{
                return thetype;
            }
    };
    void inline Block::display_singleblock(glm::vec3 pos) const{
        type->getModel()->draw(state,type->getTextures(),pos);
    }
    void inline Block::display(glm::vec3 pos,
    pBlock infront,pBlock atback,
    pBlock onleft,pBlock onright,
    pBlock ontop,pBlock atbottom) const{
        type->getModel()->drawIfNeccesary(state,type->getTextures(),
            pos,infront,atback,onleft,onright,ontop,atbottom
        );
    }
    pBlock copyBlock(pBlock src){
        return std::make_shared<Block>(*src);
    }
}
#endif// BLOCKY3DBLOCKS
