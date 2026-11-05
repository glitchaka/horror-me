#include "horror/Canvas.hpp"
#include "horror/HudRenderer.hpp"
#include "horror/IAudio.hpp"
#include "horror/Input.hpp"
#include "horror/SceneRenderer.hpp"
#include "horror/Session.hpp"
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <memory>

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
        case WM_SYSKEYDOWN:
            if(wParam==VK_F4&&(GetKeyState(VK_MENU)&0x8000))
                break;
            input_.set(static_cast<int>(wParam),true);
            return 0;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            input_.set(static_cast<int>(wParam),false);
            return 0;
        case WM_ACTIVATEAPP:
            focused_=wParam!=0;
            if(!focused_) {
                input_.releaseAll();
                session_.pauseOnFocusLoss();
                releaseMouse();
            }
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_CLOSE:
            DestroyWindow(handle);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            break;
        }
        return DefWindowProcW(handle,message,wParam,lParam);
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
        const double scale=std::min(double(width)/ScreenW,double(height)/ScreenH);
        const int drawW=std::max(1,int(ScreenW*scale));
        const int drawH=std::max(1,int(ScreenH*scale));
        const int left=(width-drawW)/2,top=(height-drawH)/2;
        HDC dc=GetDC(window_);
        if(!dc)return;
        const HBRUSH black=CreateSolidBrush(RGB(0,0,0));
        FillRect(dc,&area,black);
        DeleteObject(black);
        BITMAPINFO bitmap{};
        bitmap.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        bitmap.bmiHeader.biWidth=ScreenW;
        bitmap.bmiHeader.biHeight=-ScreenH;
        bitmap.bmiHeader.biPlanes=1;
        bitmap.bmiHeader.biBitCount=32;
        bitmap.bmiHeader.biCompression=BI_RGB;
        SetStretchBltMode(dc,COLORONCOLOR);
        StretchDIBits(dc,left,top,drawW,drawH,0,0,ScreenW,ScreenH,
                      canvas_.data(),&bitmap,DIB_RGB_COLORS,SRCCOPY);
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
        WNDCLASSEXW windowClass{};
        windowClass.cbSize=sizeof(windowClass);
        windowClass.hInstance=instance;
        windowClass.lpfnWndProc=&WinApp::windowProcedure;
        windowClass.lpszClassName=L"HorrorMeNativeWindow";
        windowClass.hCursor=LoadCursorW(nullptr,IDC_ARROW);
        windowClass.hIcon=LoadIconW(nullptr,IDI_APPLICATION);
        windowClass.style=CS_OWNDC;
        if(!RegisterClassExW(&windowClass))return 1;
        RECT bounds{0,0,1152,648};
        AdjustWindowRect(&bounds,WS_OVERLAPPEDWINDOW,FALSE);
        window_=CreateWindowExW(0,windowClass.lpszClassName,
                               L"PROTOCOLO: NO CONFORME | Horror sistemico",
                               WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,
                               bounds.right-bounds.left,bounds.bottom-bounds.top,
                               nullptr,nullptr,instance,this);
        if(!window_)return 2;
        SetProcessDPIAware();
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
                if(message.message==WM_QUIT) {
                    done=true;break;
                }
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            if(done)break;
            QueryPerformanceCounter(&now);
            const double dt=double(now.QuadPart-previous.QuadPart)/
                            double(frequency.QuadPart);
            if(dt<1.0/61.0) {
                Sleep(1);
                continue;
            }
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
