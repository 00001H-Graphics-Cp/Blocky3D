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
    typedef std::shared_ptr<BlockState> pBlockState;
    typedef std::shared_ptr<Model> pModel;
    typedef std::shared_ptr<SolidModel> pSolidModel;
    typedef std::shared_ptr<BlockType> pBlockType;
    typedef std::shared_ptr<Block> pBlock;
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
            virtual const pAnimatedTexture& faceOf(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt) const = 0;
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
            virtual ~Model() = default;
    };
    class SolidModel : public Model{
        public:
            virtual const pAnimatedTexture& faceOf(Direction face,pBlockState bs,
            const std::vector<pAnimatedTexture>& tt) const override{
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
                        break;
                    };
                    default:{
                        throw blocky_error("Internal Error at lineno "+std::to_string(__LINE__));
                    }
                }
                auto r3d = std::make_shared<pygame::geometry::Rect3D>(
                bleft,bright,tleft,tright
                );
                pygame::draw::rect3D(r3d,tt.at(static_cast<int>(face))->curFrame());
            }
    };
    pSolidModel SOLID_MODEL = std::make_shared<SolidModel>();
    #define pat pAnimatedTexture
    std::vector<pAnimatedTexture> inline makeSolidTextures(pat front,pat back,pat left,pat right,pat top,pat bottom){
        return vectorOf<pat>(front,back,left,right,top,bottom);
    }
    #undef pat
    namespace{
        std::unordered_map<std::string,pBlockType> block_registry;
    }
    pBlockType DEFAULT_BLOCKTYPE = BlockType::constructWithId("blocky3d:placeholder");
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
                thetype = DEFAULT_BLOCKTYPE;
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
