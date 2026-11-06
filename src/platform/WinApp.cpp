#include "horror/Canvas.hpp"
#include "horror/HudRenderer.hpp"
#include "horror/IAudio.hpp"
#include "horror/Input.hpp"
#include "horror/PixelViewport.hpp"
#include "horror/SceneRenderer.hpp"
#include "horror/Session.hpp"
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

namespace horror {
namespace {
class WinApp final {
    HWND window_{};
    Input input_{};
    GameSession session_{};
    Canvas canvas_{};
    SceneRenderer scene_{};
    HudRenderer hud_{};
    std::unique_ptr<IAudio> audio_{makeNativeAudio()};
    std::vector<Color> output_{};
    RECT windowedRect_{};
    bool fullscreen_{false};
    bool focused_{true};
    bool mouseCaptured_{false};

    static LRESULT CALLBACK windowProcedure(HWND handle,UINT message,
                                             WPARAM wParam,LPARAM lParam) {
        if(message==WM_NCCREATE) {
            auto* creation=reinterpret_cast<CREATESTRUCTW*>(lParam);
            auto* app=static_cast<WinApp*>(creation->lpCreateParams);
            SetWindowLongPtrW(handle,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(app));
        }
        auto* app=reinterpret_cast<WinApp*>(GetWindowLongPtrW(handle,GWLP_USERDATA));
        return app?app->onMessage(handle,message,wParam,lParam):
                   DefWindowProcW(handle,message,wParam,lParam);
    }
    LRESULT onMessage(HWND handle,UINT message,WPARAM wParam,LPARAM lParam) {
        switch(message) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            if(wParam==VK_F4&&(GetKeyState(VK_MENU)&0x8000))
                break;
            // F11 or Alt+Enter: switch between borderless fullscreen
            // and borderless windowed mode without recreating the game.
            const bool firstPress=(lParam&(1LL<<30))==0;
            const bool altEnter=(wParam==VK_RETURN&&
                (GetKeyState(VK_MENU)&0x8000)!=0);
            if(wParam==VK_F11||altEnter) {
                if(firstPress) setFullscreen(!fullscreen_);
                return 0;
            }
            input_.set(static_cast<int>(wParam),true);
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if(wParam==VK_F11||
               (wParam==VK_RETURN&&(lParam&(1LL<<29))!=0))
                return 0;
            input_.set(static_cast<int>(wParam),false);
            return 0;
        case WM_NCHITTEST:
            if(!fullscreen_&&!mouseCaptured_) {
                POINT p{static_cast<short>(LOWORD(lParam)),
                        static_cast<short>(HIWORD(lParam))};
                ScreenToClient(handle,&p);
                RECT area{};
                GetClientRect(handle,&area);
                const int height=calculatePixelViewport(area.right,area.bottom).scale*34;
                if(p.y>=0&&p.y<height)return HTCAPTION;
            }
            break;
        case WM_DPICHANGED:
            // Coordinates are physical pixels (per-monitor DPI aware).
            // The windowed placement supplied by Windows follows the new monitor.
            if(!fullscreen_) {
                const auto* suggested=reinterpret_cast<RECT*>(lParam);
                SetWindowPos(handle,nullptr,suggested->left,suggested->top,
                             suggested->right-suggested->left,
                             suggested->bottom-suggested->top,
                             SWP_NOACTIVATE|SWP_NOZORDER);
            }
            return 0;
        case WM_ACTIVATEAPP:
            focused_=wParam!=0;
            if(!focused_) {
                input_.releaseAll();
                session_.pauseOnFocusLoss();
                releaseMouse();
            }
            return 0;
        case WM_ERASEBKGND:return 1;
        case WM_CLOSE:DestroyWindow(handle);return 0;
        case WM_DESTROY:PostQuitMessage(0);return 0;
        default:break;
        }
        return DefWindowProcW(handle,message,wParam,lParam);
    }
    void setFullscreen(bool enabled) {
        if(!window_||fullscreen_==enabled)return;
        releaseMouse();
        if(enabled)GetWindowRect(window_,&windowedRect_);
        RECT target=windowedRect_;
        if(enabled) {
            HMONITOR monitor=MonitorFromWindow(window_,MONITOR_DEFAULTTONEAREST);
            MONITORINFO info{sizeof(info)};
            if(GetMonitorInfoW(monitor,&info))target=info.rcMonitor;
        }
        // WS_POPUP is retained in both modes: no Windows frame/title bar.
        SetWindowPos(window_,nullptr,target.left,target.top,
                     target.right-target.left,target.bottom-target.top,
                     SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        fullscreen_=enabled;
        InvalidateRect(window_,nullptr,FALSE);
    }
    void releaseMouse() {
        if(mouseCaptured_) {
            ShowCursor(TRUE);
            mouseCaptured_=false;
        }
    }
    void sampleMouse() {
        if(!focused_||session_.phase()!=Phase::Playing) {
            releaseMouse();
            return;
        }
        RECT bounds{};
        GetClientRect(window_,&bounds);
        POINT center{(bounds.right-bounds.left)/2,(bounds.bottom-bounds.top)/2};
        ClientToScreen(window_,&center);
        if(!mouseCaptured_) {
            ShowCursor(FALSE);
            mouseCaptured_=true;
            SetCursorPos(center.x,center.y);
            return;
        }
        POINT cursor{};
        if(!GetCursorPos(&cursor))return;
        const int dx=cursor.x-center.x;
        if(std::abs(dx)<200)input_.mouseDeltaX=double(dx);
        if(dx||cursor.y!=center.y)SetCursorPos(center.x,center.y);
    }
    void present() {
        RECT area{};
        GetClientRect(window_,&area);
        const int width=area.right-area.left,height=area.bottom-area.top;
        if(width<=0||height<=0)return;

        HDC dc=GetDC(window_);
        if(!dc)return;
        BITMAPINFO bitmap{};
        bitmap.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        bitmap.bmiHeader.biPlanes=1;
        bitmap.bmiHeader.biBitCount=32;
        bitmap.bmiHeader.biCompression=BI_RGB;

        const PixelViewport viewport=calculatePixelViewport(width,height);
        if(viewport.width<=width&&viewport.height<=height) {
            // CPU nearest-neighbor upscale to *physical* pixels.
            // GDI only copies the resulting DIB at 1:1: no interpolation,
            // fractional scaling or DPI virtualization can blur it.
            output_.resize(static_cast<std::size_t>(width)*height);
            std::fill(output_.begin(),output_.end(),Color{0});
            const Color* const src=canvas_.data();
            for(int sy=0;sy<ScreenH;++sy) {
                const int dy=viewport.y+sy*viewport.scale;
                Color* row=output_.data()+static_cast<std::size_t>(dy)*width+viewport.x;
                for(int sx=0;sx<ScreenW;++sx)
                    std::fill_n(row+sx*viewport.scale,viewport.scale,
                                src[sy*ScreenW+sx]);
                for(int repeat=1;repeat<viewport.scale;++repeat)
                    std::memcpy(row+static_cast<std::size_t>(repeat)*width,row,
                                static_cast<std::size_t>(viewport.width)*sizeof(Color));
            }
            bitmap.bmiHeader.biWidth=width;
            bitmap.bmiHeader.biHeight=-height;
            SetDIBitsToDevice(dc,0,0,width,height,0,0,0,height,
                              output_.data(),&bitmap,DIB_RGB_COLORS);
        } else {
            // Very small displays cannot show a full 384x216 frame at
            // integer scale; clip at native pixel resolution instead.
            bitmap.bmiHeader.biWidth=ScreenW;
            bitmap.bmiHeader.biHeight=-ScreenH;
            const int x=(width-ScreenW)/2,y=(height-ScreenH)/2;
            SetDIBitsToDevice(dc,x,y,ScreenW,ScreenH,0,0,0,ScreenH,
                              canvas_.data(),&bitmap,DIB_RGB_COLORS);
        }
        ReleaseDC(window_,dc);
    }
    void frame(double delta) {
        sampleMouse();
        session_.update(delta,input_);
        if(input_.pressed('M'))audio_->toggleMute();
        for(SoundEvent event:session_.drainSounds())audio_->play(event);
        audio_->setTension(static_cast<float>(session_.tension()));
        if(session_.phase()==Phase::Playing||session_.phase()==Phase::Paused)
            scene_.draw(canvas_,session_);
        hud_.draw(canvas_,session_,input_);
        present();
        input_.endFrame();
        if(session_.phase()!=Phase::Playing)releaseMouse();
    }
public:
    ~WinApp() {
        releaseMouse();
        if(audio_)audio_->stop();
    }
    int run(HINSTANCE instance,int showCommand) {
        // Set DPI awareness BEFORE creating a HWND. Otherwise Windows may
        // scale the framebuffer a second time and blur pixel art.
        if(!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
            SetProcessDPIAware();
        WNDCLASSEXW windowClass{};
        windowClass.cbSize=sizeof(windowClass);
        windowClass.hInstance=instance;
        windowClass.lpfnWndProc=&WinApp::windowProcedure;
        windowClass.lpszClassName=L"HorrorMeNativeWindow";
        windowClass.hCursor=LoadCursorW(nullptr,IDC_ARROW);
        windowClass.hIcon=LoadIconW(nullptr,IDI_APPLICATION);
        windowClass.style=CS_OWNDC;
        if(!RegisterClassExW(&windowClass))return 1;

        POINT mouse{};
        GetCursorPos(&mouse);
        HMONITOR monitor=MonitorFromPoint(mouse,MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO info{sizeof(info)};
        if(!GetMonitorInfoW(monitor,&info))return 2;
        const RECT work=info.rcWork;
        const int w=std::min(1152,work.right-work.left);
        const int h=std::min(648,work.bottom-work.top);
        const int x=work.left+(work.right-work.left-w)/2;
        const int y=work.top+(work.bottom-work.top-h)/2;
        // Borderless even in windowed mode; F11 toggles fullscreen.
        window_=CreateWindowExW(WS_EX_APPWINDOW,windowClass.lpszClassName,
                               L"PROTOCOLO: NO CONFORME",
                               WS_POPUP,x,y,w,h,
                               nullptr,nullptr,instance,this);
        if(!window_)return 3;
        GetWindowRect(window_,&windowedRect_);
        setFullscreen(true);
        ShowWindow(window_,showCommand);
        UpdateWindow(window_);
        audio_->start();

        LARGE_INTEGER frequency{},previous{},now{};
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&previous);
        MSG message{};
        bool done=false;
        while(!done) {
            while(PeekMessageW(&message,nullptr,0,0,PM_REMOVE)) {
                if(message.message==WM_QUIT){done=true;break;}
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            if(done)break;
            QueryPerformanceCounter(&now);
            const double dt=double(now.QuadPart-previous.QuadPart)/
                            double(frequency.QuadPart);
            if(dt<1.0/61.0){Sleep(1);continue;}
            previous=now;
            frame(std::min(dt,.05));
        }
        return static_cast<int>(message.wParam);
    }
};
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,LPWSTR,int showCommand) {
    horror::WinApp app;
    return app.run(instance,showCommand);
}
