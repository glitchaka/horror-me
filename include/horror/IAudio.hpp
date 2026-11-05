#pragma once
#include "horror/Session.hpp"
#include <memory>

namespace horror {
class IAudio {
public:
    virtual ~IAudio()=default;
    virtual void start()=0;
    virtual void stop()=0;
    virtual void play(SoundEvent event)=0;
    virtual void setTension(float value)=0;
    virtual void toggleMute()=0;
};
std::unique_ptr<IAudio> makeNativeAudio();
}
