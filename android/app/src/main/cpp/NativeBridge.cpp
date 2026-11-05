#include "horror/Canvas.hpp"
#include "horror/HudRenderer.hpp"
#include "horror/Input.hpp"
#include "horror/SceneRenderer.hpp"
#include "horror/Session.hpp"
#include <jni.h>
#include <algorithm>
#include <cstdint>
#include <memory>

namespace {
struct NativeEngine {
    horror::GameSession game{};
    horror::Canvas buffer{};
    horror::SceneRenderer scene{};
    horror::HudRenderer hud{};
    horror::Input input{};
};

NativeEngine& engine() {
    static NativeEngine instance{};
    return instance;
}
constexpr int Forward = 1 << 0;
constexpr int Backward = 1 << 1;
constexpr int StrafeLeft = 1 << 2;
constexpr int StrafeRight = 1 << 3;
constexpr int RotateLeft = 1 << 4;
constexpr int RotateRight = 1 << 5;
constexpr int Run = 1 << 6;
constexpr int Interact = 1 << 7;
constexpr int Scan = 1 << 8;
constexpr int Anchor = 1 << 9;
constexpr int Map = 1 << 10;
constexpr int Reject = 1 << 11;
constexpr int Enter = 1 << 12;
constexpr int Pause = 1 << 13;
constexpr int Mute = 1 << 14;
void mapInput(horror::Input& input,int flags,double lookDelta) {
    input.set('W',(flags&Forward)!=0);
    input.set('S',(flags&Backward)!=0);
    input.set('A',(flags&StrafeLeft)!=0);
    input.set('D',(flags&StrafeRight)!=0);
    input.set(37,(flags&RotateLeft)!=0);
    input.set(39,(flags&RotateRight)!=0);
    input.set(16,(flags&Run)!=0);
    input.set('E',(flags&Interact)!=0);
    input.set('F',(flags&Scan)!=0);
    input.set('Q',(flags&Anchor)!=0);
    input.set(9,(flags&Map)!=0);
    input.set('R',(flags&Reject)!=0);
    input.set(13,(flags&Enter)!=0);
    input.set('P',(flags&Pause)!=0);
    input.set('M',(flags&Mute)!=0);
    input.mouseDeltaX=std::clamp(lookDelta,-100.0,100.0);
}
} // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_glitchaka_horrorme_MainActivity_nativeFrame(
    JNIEnv* env,jclass,jint keys,jfloat lookDelta,jfloat deltaSeconds,
    jintArray target) {
    if(target==nullptr||env->GetArrayLength(target)<horror::ScreenW*horror::ScreenH)
        return;
    auto& e=engine();
    mapInput(e.input,keys,lookDelta);
    e.game.update(std::clamp(double(deltaSeconds),0.0,.05),e.input);
    if(e.game.phase()==horror::Phase::Playing||
       e.game.phase()==horror::Phase::Paused)
        e.scene.draw(e.buffer,e.game);
    e.hud.draw(e.buffer,e.game,e.input);
    e.input.endFrame();

    // Android Bitmap expects ARGB_8888; the shared framebuffer uses RGB.
    auto* output=env->GetIntArrayElements(target,nullptr);
    if(output==nullptr)return;
    const horror::Color* pixels=e.buffer.data();
    for(int i=0;i<horror::ScreenW*horror::ScreenH;++i)
        output[i]=static_cast<jint>(pixels[i]|0xff000000u);
    env->ReleaseIntArrayElements(target,output,0);
}
extern "C" JNIEXPORT jint JNICALL
Java_com_glitchaka_horrorme_MainActivity_nativePhase(JNIEnv*,jclass) {
    return static_cast<jint>(engine().game.phase());
}
extern "C" JNIEXPORT jint JNICALL
Java_com_glitchaka_horrorme_MainActivity_nativeSoundEvents(JNIEnv*,jclass) {
    auto sounds=engine().game.drainSounds();
    unsigned int mask=0;
    for(auto sound:sounds)
        mask|=(1u<<static_cast<unsigned int>(sound));
    return static_cast<jint>(mask);
}
extern "C" JNIEXPORT jfloat JNICALL
Java_com_glitchaka_horrorme_MainActivity_nativeTension(JNIEnv*,jclass) {
    return static_cast<jfloat>(engine().game.tension());
}
extern "C" JNIEXPORT void JNICALL
Java_com_glitchaka_horrorme_MainActivity_nativePause(JNIEnv*,jclass) {
    engine().game.pauseOnFocusLoss();
    engine().input.releaseAll();
}
