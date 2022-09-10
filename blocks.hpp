#ifndef BLOCKY3DBLOCKS
#define BLOCKY3DBLOCKS
#include<memory>
#include<string>
#include<vector>
#include<unordered_map>
#include<utility>
#include<optional>
#include<pygame.hpp>
#include"json.hpp"
#include"textures.hpp"
#include<glm/glm.hpp>
namespace blocky{
    std::string wtos(std::wstring inp){
        size_t leng = std::wcstombs(nullptr,inp.data(),99999);
        std::string dest;
        dest.resize(leng);
        std::wcstombs(dest.data(),inp.data(),leng);
        return dest;
    }
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
                FRONT=0,BACK=1,LEFT=2,RIGHT=3,UP=4,DOWN=5,NONE=6
            };
            Direction() = default;
            Direction(_Direction dir) : _direc(dir){}
            static void selftest(){
                #ifndef NDEBUG
                glm::ivec3 a(0,1,2);
                glm::ivec3 b(1,1,2);
                assert(from_ray(a,b)==RIGHT);
                assert(from_ray(a,b).next_to(a) == b);
                #endif
            }
            glm::ivec3 next_to(glm::ivec3 pos) const{
                switch(_direc){
                    case UP:return pos+glm::ivec3(0,1,0);
                    case DOWN:return pos-glm::ivec3(0,1,0);
                    case LEFT:return pos-glm::ivec3(1,0,0);
                    case RIGHT:return pos+glm::ivec3(1,0,0);
                    case FRONT:return pos+glm::ivec3(0,0,1);
                    case BACK:return pos-glm::ivec3(0,0,1);
                    case NONE:return pos;
                    default:throw blocky_error("Internal Error : "+std::to_string(__LINE__)+" : "+__FILE__);
                }
            }
            std::string tostr() const{
                #define _Case(x) case x:return #x
                switch(_direc){
                    _Case(UP);
                    _Case(DOWN);
                    _Case(LEFT);
                    _Case(RIGHT);
                    _Case(FRONT);
                    _Case(BACK);
                    _Case(NONE);
                    default:throw blocky_error("Internal Error : "+std::to_string(__LINE__)+" : "+__FILE__);
                }
            }
            static Direction from_ivec3(glm::ivec3 dir){
                int count = ((dir.x!=0)+(dir.y!=0)+(dir.z!=0));
                if(count==0){
                    return NONE;
                }
                if(count>1){
                    std::cerr << "Warning::Direction::from_ivec3() : " << std::to_string(__LINE__)
                              << " : " << __FILE__ << "Invalid ivec3" << std::endl;
                    return NONE;
                }
                if(dir.x>0)return RIGHT;
                if(dir.x<0)return LEFT;
                if(dir.y>0)return UP;
                if(dir.y<0)return DOWN;
                if(dir.z>0)return FRONT;
                if(dir.z<0)return BACK;
                std::cout << "Invoking undefined behavior" << std::endl;
                std::unreachable();
            }
            static Direction from_ray(glm::ivec3 a,glm::ivec3 b){
                return from_ivec3(b-a);
            }
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
                    case UP:return DOWN;
                    case DOWN:return UP;
                    case NONE:return NONE;
                    default:throw blocky_error("Internal Error : "+std::to_string(__LINE__)+" : "+__FILE__);
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
    typedef std::unique_ptr<BlockType> upBlockType;
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
            std::vector<pGameTexture> texture_storage;
            std::wstring id;
            static pBlockType PLACEHOLDER_BLOCK;
        public:
            BlockType(std::wstring id) : id(id){
            }
            static inline pBlockType default_block();
            BlockType(const BlockType& other){
                rottype = other.rottype;
                CustomProperties = other.CustomProperties;
                model = other.model;
                defaultState = other.defaultState;
                id = other.id;
            }
            const std::wstring& getID() const{
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
            const std::vector<pGameTexture>& getTextures() const{
                return texture_storage;
            }
            bool isFaceTransparent(Direction face,std::optional<pBlockState> opt_bs=std::optional<pBlockState>()) const;
    };
    pBlockType BlockType::PLACEHOLDER_BLOCK=nullptr;
    pBlockType BlockType::default_block(){
        if(PLACEHOLDER_BLOCK==nullptr){
            PLACEHOLDER_BLOCK = std::make_shared<BlockType>(L"blocky3d:placeholder");
        }
        return PLACEHOLDER_BLOCK;
    }
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
            auto getModel() const{
                return type->getModel();
            }
            const pBlockState& getState() const{
                return state;
            }
            void inline display_singleblock(glm::vec3 pos) const;
            void inline display(glm::vec3 pos,
                pBlock infront=nullptr,pBlock atback=nullptr,
                pBlock onleft=nullptr,pBlock onright=nullptr,
                pBlock ontop=nullptr,pBlock atbottom=nullptr) const;
            bool isFaceTransparent(Direction face) const{
                return type->isFaceTransparent(face,state);
            }
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
            const std::vector<pGameTexture>& tt) const = 0;
            virtual void drawFace(Direction face,pBlockState bs,
            const std::vector<pGameTexture>& tt,glm::vec3 loc) const = 0;
            virtual bool shouldCullFace(Direction face,pBlockState bs,
            const std::vector<pGameTexture>& tt,
            pBlock next_to) const{
                if(next_to==nullptr)return false;
                return !(
(next_to->isFaceTransparent(face.opposite()))
&&(next_to->isFaceTransparent(face)));
            }
            virtual void drawFaceIfNeccesary(Direction face,pBlockState bs,
            const std::vector<pGameTexture>& tt,glm::vec3 loc,
            pBlock next_to) const final{
                if(!shouldCullFace(face,bs,tt,next_to))drawFace(face,bs,tt,loc);
            }
            virtual void draw(pBlockState bs,
            const std::vector<pGameTexture>& tt,glm::vec3 loc) const{
                drawFace(Direction::FRONT,bs,tt,loc);
                drawFace(Direction::BACK,bs,tt,loc);
                drawFace(Direction::LEFT,bs,tt,loc);
                drawFace(Direction::RIGHT,bs,tt,loc);
                drawFace(Direction::UP,bs,tt,loc);
                drawFace(Direction::DOWN,bs,tt,loc);
            }
            virtual void drawIfNeccesary(pBlockState bs,
            const std::vector<pGameTexture>& tt,glm::vec3 loc,
            pBlock front,pBlock back,pBlock left,pBlock right,pBlock top,pBlock bottom)
            const{
                drawFaceIfNeccesary(Direction::FRONT,bs,tt,loc,front);
                drawFaceIfNeccesary(Direction::BACK,bs,tt,loc,back);
                drawFaceIfNeccesary(Direction::LEFT,bs,tt,loc,left);
                drawFaceIfNeccesary(Direction::RIGHT,bs,tt,loc,right);
                drawFaceIfNeccesary(Direction::UP,bs,tt,loc,top);
                drawFaceIfNeccesary(Direction::DOWN,bs,tt,loc,bottom);
            };
            virtual bool phys_at(glm::ivec3 subvox) const = 0;
            virtual bool contains(glm::vec3 subpos) final{
                return phys_at(glm::floor(subpos*16.0f));
            }
            virtual ~Model(){};
    };
    bool inline BlockType::isFaceTransparent(Direction face,std::optional<pBlockState> opt_bs) const{
            return model->isFaceTransparent(face,opt_bs.value_or(getDefaultState()),getTextures());
        }
    class SolidModel : public Model{
        public:
            virtual bool phys_at(glm::ivec3 subvox) const override{
                return true;
            }
            virtual const pGameTexture& faceOf(Direction face,pBlockState bs,
            const std::vector<pGameTexture>& tt) const{
                return tt.at(static_cast<int>(face));
            }
            virtual bool isFaceTransparent(Direction face,pBlockState bs,
            const std::vector<pGameTexture>& tt) const override{
                return faceOf(face,bs,tt)->isTransparent();
            }
            virtual void drawFace(Direction face,pBlockState bs,
            const std::vector<pGameTexture>& tt,glm::vec3 loc) const override{
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
                    case Direction::UP:[[fallthrough]];
                    case Direction::DOWN:{
                        if(face==Direction::UP){
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
                        if(face==Direction::UP){
                            std::swap(bright,bleft);
                            std::swap(tright,tleft);
                        }else if(face==Direction::DOWN){
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
virtual const pGameTexture& faceOf(Direction face,pBlockState bs,
            const std::vector<pGameTexture>& tt) const override{
                return tt.at(0);
            }
    };
    class NothingModel : public Model{
            virtual bool phys_at(glm::ivec3 subvox) const override{
                return false;
            }
            virtual bool isFaceTransparent(Direction face,pBlockState bs,
            const std::vector<pGameTexture>& tt) const override{
                return true;
            }
            virtual void drawFace(Direction face,pBlockState bs,
            const std::vector<pGameTexture>& tt,glm::vec3 loc) const override{
                return;
            }
    };
    pSolidModel SOLID_MODEL = std::make_shared<SolidModel>();
    pOneFaceModel ONEFACE_MODEL = std::make_shared<OneFaceModel>();
    pNothingModel NOTHING_MODEL = std::make_shared<NothingModel>();
    
    #define pgt pGameTexture
    auto inline makeSolidTextures(pgt front,pgt back,pgt left,pgt right,pgt top,pgt bottom){
        auto patl = new std::vector<pGameTexture>();
        patl->push_back(front);
        patl->push_back(back);
        patl->push_back(left);
        patl->push_back(right);
        patl->push_back(top);
        patl->push_back(bottom);
        return std::unique_ptr<std::vector<pGameTexture>>(patl);
    }
    #define prt pygame::prTexture
    #define mks(xxx) std::make_shared<GameTexture>(xxx)
    auto inline makeSolidTextures(prt front,prt back,prt left,prt right,prt top,prt bottom){
        return makeSolidTextures(
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
    #undef pgt
    namespace{
        std::unordered_map<std::wstring,std::shared_ptr<BlockType>> block_registry;
    }
    pBlockType const DEFAULT_BLOCKTYPE = BlockType::default_block();
    void registerBlockType(upBlockType bt){
        std::wstring id = bt->getID();
        block_registry.insert_or_assign(id,std::shared_ptr<BlockType>(bt.release()));
    }
    pBlockType getBlock(std::wstring id){
        if(id==BlockType::default_block()->getID()){
            return DEFAULT_BLOCKTYPE;
        }
        return block_registry.at(id);
    }
    class BlockTypeBuilder{
        upBlockType thetype;
        public:
            BlockTypeBuilder(pBlockType blocktypebase=DEFAULT_BLOCKTYPE){
                thetype = std::make_unique<BlockType>(*blocktypebase);
            }
            BlockTypeBuilder(std::wstring id){
                thetype = std::unique_ptr<BlockType>(new BlockType(*DEFAULT_BLOCKTYPE));
                thetype->id = id;
            }
            BlockTypeBuilder& id(std::wstring id){
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
            BlockTypeBuilder& textures(const std::vector<pGameTexture>& textures){
                thetype->texture_storage = textures;
                return *this;
            }
            BlockTypeBuilder& textures(std::unique_ptr<std::vector<pGameTexture>> textures){
                thetype->texture_storage = *textures;
                return *this;
            }
            BlockTypeBuilder& textures(const pGameTexture& texture){
                thetype->texture_storage.clear();
                thetype->texture_storage.push_back(texture);
                return *this;
            }
            upBlockType finalize(){
                return std::move(thetype);
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
    pBlock makeBlock(std::wstring id){
        return std::make_shared<Block>(getBlock(id));
    }
}
#endif// BLOCKY3DBLOCKS
