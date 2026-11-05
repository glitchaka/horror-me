#pragma once
#include <array>

namespace horror {
struct Input {
    std::array<bool,256> keys{};
    std::array<bool,256> previous{};
    double mouseDeltaX{};
    bool held(int key) const {
        return key>=0&&key<256&&keys[static_cast<std::size_t>(key)];
    }
    bool pressed(int key) const {
        return held(key)&&!previous[static_cast<std::size_t>(key)];
    }
    void set(int key,bool down) {
        if(key>=0&&key<256) keys[static_cast<std::size_t>(key)]=down;
    }
    void endFrame() { previous=keys; mouseDeltaX=0; }
    void releaseAll() { keys.fill(false);previous.fill(false);mouseDeltaX=0; }
};
}
