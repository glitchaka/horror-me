#include "horror/IAudio.hpp"
#include "horror/Types.hpp"
#include <windows.h>
#include <mmsystem.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>
#include <thread>

namespace horror {
namespace {
class WinAudio final : public IAudio {
    static constexpr int Rate=44100,BufferSize=2048,BufferCount=6;
    HWAVEOUT device_{};
    std::array<std::array<std::int16_t,BufferSize>,BufferCount> samples_{};
    std::array<WAVEHDR,BufferCount> headers_{};
    std::thread worker_{};
    std::atomic<bool> active_{false},muted_{false};
    std::atomic<int> requested_{0};
    std::atomic<float> tension_{0};
    std::uint64_t clock_=0;
    std::uint32_t random_=0xA5C31u;
    int currentEffect_=0,effectRemaining_=0;
    double phase_=0;

    static double wave(double cycles) { return std::sin(cycles*2*Pi); }
    void fill(int index) {
        for(int j=0;j<BufferSize;++j,++clock_) {
            const int event=requested_.exchange(0);
            if(event) {
                currentEffect_=event;
                effectRemaining_=Rate/3;
                phase_=0;
            }
            const double t=double(clock_)/Rate;
            const float tension=tension_.load();
            const int step=int(t*4.8);
            constexpr std::array<double,8> tones{
                110.0,116.54,130.81,103.83,110.0,98.00,87.31,103.83
            };
            const double base=tones[static_cast<std::size_t>((step/4)%8)];
            const double bass=.29*wave(t*base*.5);
            const double drone=.12*wave(t*base*1.006)+.065*wave(t*base*1.503);
            const double beatFrac=std::fmod(t*2.4,1.0);
            const double beat=beatFrac<.14?
                std::exp(-beatFrac*30)*wave(t*(68+55*(1-beatFrac)))*.23:0;
            const double noteFrac=std::fmod(t*4.8,1.0);
            const double note=noteFrac<.3?
                (wave(t*base*2)>0?.055:-.055)*std::exp(-noteFrac*9):0;
            random_=random_*1664525u+1013904223u;
            const double hiss=(double((random_>>16)&65535)/32768.0-1)*
                              (.016+tension*.06);
            double output=(bass+drone+beat+note+hiss)*(.22+tension*.11);
            if(effectRemaining_>0) {
                const double progress=1.0-double(effectRemaining_)/(Rate/3);
                double frequency=360;
                switch(currentEffect_) {
                case 1:frequency=620-410*progress;break;
                case 2:frequency=420+900*progress;break;
                case 3:frequency=700+190*std::sin(progress*23);break;
                case 4:frequency=180-145*progress;break;
                case 5:frequency=85+230*progress;break;
                case 6:frequency=75-50*progress;break;
                case 7:frequency=440+660*progress;break;
                default:break;
                }
                phase_+=2*Pi*frequency/Rate;
                const double env=std::pow(1.0-progress,1.5);
                const double effect=(currentEffect_==4||currentEffect_==6)?
                    std::sin(phase_):(std::sin(phase_)>0?.8:-.8);
                output+=effect*env*.34;
                --effectRemaining_;
            }
            samples_[static_cast<std::size_t>(index)][static_cast<std::size_t>(j)]=
                static_cast<std::int16_t>(std::clamp(muted_.load()?0.0:output,-.95,.95)*32700);
        }
    }
    void pump() {
        while(active_.load()) {
            for(int i=0;i<BufferCount;++i) {
                WAVEHDR& header=headers_[static_cast<std::size_t>(i)];
                if(header.dwFlags&WHDR_DONE) {
                    fill(i);
                    if(device_) waveOutWrite(device_,&header,sizeof(header));
                }
            }
            Sleep(4);
        }
    }
public:
    ~WinAudio() override { stop(); }
    void start() override {
        if(device_)return;
        WAVEFORMATEX format{};
        format.wFormatTag=WAVE_FORMAT_PCM;
        format.nChannels=1;
        format.nSamplesPerSec=Rate;
        format.wBitsPerSample=16;
        format.nBlockAlign=2;
        format.nAvgBytesPerSec=Rate*2;
        if(waveOutOpen(&device_,WAVE_MAPPER,&format,0,0,CALLBACK_NULL)!=MMSYSERR_NOERROR) {
            device_=nullptr;return;
        }
        for(int i=0;i<BufferCount;++i) {
            fill(i);
            WAVEHDR& header=headers_[static_cast<std::size_t>(i)];
            header.lpData=reinterpret_cast<LPSTR>(samples_[static_cast<std::size_t>(i)].data());
            header.dwBufferLength=DWORD(BufferSize*sizeof(std::int16_t));
            if(waveOutPrepareHeader(device_,&header,sizeof(header))==MMSYSERR_NOERROR)
                waveOutWrite(device_,&header,sizeof(header));
        }
        active_.store(true);
        worker_=std::thread([this]{pump();});
    }
    void stop() override {
        active_.store(false);
        if(worker_.joinable())worker_.join();
        if(device_) {
            waveOutReset(device_);
            for(auto& header:headers_)
                if(header.dwFlags&WHDR_PREPARED)
                    waveOutUnprepareHeader(device_,&header,sizeof(header));
            waveOutClose(device_);
            device_=nullptr;
        }
    }
    void play(SoundEvent event) override {
        requested_.store(static_cast<int>(event)+1);
    }
    void setTension(float value) override { tension_.store(std::clamp(value,0.f,1.f)); }
    void toggleMute() override { muted_.store(!muted_.load()); }
};
}
std::unique_ptr<IAudio> makeNativeAudio() {
    return std::make_unique<WinAudio>();
}
}
