#ifndef BLOCKY3D_TEXTURES_HPP
#define BLOCKY3D_TEXTURES_HPP
#include<vector>
#include<memory>
#include<gsdl.hpp>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
namespace blocky{
    class AnimatedTexture{
        std::vector<pygame::pTexture> frames;
        float fps;
        bool hasTransparentFrames=false;
        public:
            AnimatedTexture(float fps=20.0) : fps(fps){}
            AnimatedTexture& addFrame(pygame::pTexture frame,bool transparent=false){
                frames.push_back(frame);
                hasTransparentFrames |= transparent;
                return *this;
            }
            void makeOpaque(){
                hasTransparentFrames = false;
            }
            void makeTransparent(){
                hasTransparentFrames = true;
            }
            const pygame::pTexture& operator[](int frm) const{
                return frames.at(frm);
            }
            pygame::pTexture& operator[](int frm){
                return const_cast<pygame::pTexture&>(operator[](frm));
            }
            const pygame::pTexture& curFrame() const{
                return frames.at(static_cast<int>(glfwGetTime()*fps)%frames.size());
            }
            pygame::pTexture& curFrame(){
                return const_cast<pygame::pTexture&>(const_cast<const AnimatedTexture&>(*this).curFrame());
            }
            bool isTransparent() const{
                return hasTransparentFrames;
            }
            void setFrame(int frm,pygame::pTexture frame){
                frames.at(frm) = frame;
            }
    };
    typedef std::shared_ptr<AnimatedTexture> pAnimatedTexture;
}
#endif