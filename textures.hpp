#ifndef BLOCKY3D_TEXTURES_HPP
#define BLOCKY3D_TEXTURES_HPP
#include<vector>
#include<memory>
#include<gsdl.hpp>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
namespace blocky{
    class AnimatedTexture{
        std::vector<pygame::prTexture> frames;
        float fps;
        bool hasTransparentFrames=false;
        public:
            AnimatedTexture(float fps=20.0) : fps(fps){}
            AnimatedTexture(pygame::prTexture frm,float fps=20.0) : fps(fps){
                addFrame(frm);
            }
            AnimatedTexture& addFrame(pygame::prTexture frame){
                frames.push_back(frame);
                hasTransparentFrames |= (frame.alpha<1.0);
                return *this;
            }
            void makeOpaque(){
                hasTransparentFrames = false;
            }
            void makeTransparent(){
                hasTransparentFrames = true;
            }
            const pygame::prTexture& operator[](int frm) const{
                return frames.at(frm);
            }
            pygame::prTexture& operator[](int frm){
                return const_cast<pygame::prTexture&>(operator[](frm));
            }
            const pygame::prTexture& curFrame() const{
                return frames.at(static_cast<int>(glfwGetTime()*fps)%frames.size());
            }
            pygame::prTexture& curFrame(){
                return const_cast<pygame::prTexture&>(const_cast<const AnimatedTexture&>(*this).curFrame());
            }
            bool isTransparent() const{
                return hasTransparentFrames;
            }
            void setFrame(int frm,pygame::prTexture frame){
                frames.at(frm) = frame;
            }
    };
    typedef std::shared_ptr<AnimatedTexture> pAnimatedTexture;
}
#endif