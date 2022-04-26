#include "mvViewport.h"
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT
ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK
HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

mvViewport*
initialize_viewport(int width, int height)
{
    mvViewport* viewport = new mvViewport{
        width,
        height,
        nullptr,
        false
    };

    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = HandleMsgSetup;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = "mvImporter Window";
    wc.hIconSm = nullptr;
    RegisterClassEx(&wc);

    // calculate window size based on desired client region size
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = height + wr.top;
    AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_OVERLAPPED | WS_THICKFRAME, FALSE);

    // create window & get hWnd
    viewport->hWnd = CreateWindowA(
        "mvImporter Window", 
        "mvImporter Sandbox",
        WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_OVERLAPPED | WS_THICKFRAME,
        0, 0, wr.right - wr.left, wr.bottom - wr.top,
        nullptr, 
        nullptr, 
        GetModuleHandle(nullptr), 
        viewport // app data
    );

    ShowWindow(viewport->hWnd, SW_SHOWDEFAULT);
    return viewport;
}
    
std::optional<int>
process_viewport_events()
{
    MSG msg;
        
    // while queue has messages, remove and dispatch them (but do not block on empty queue)
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        // check for quit because peekmessage does not signal this via return val
        if (msg.message == WM_QUIT)
            return (int)msg.wParam;

        // TranslateMessage will post auxilliary WM_CHAR messages from key msgs
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // return empty optional when not quitting app
    return {};
}

static LRESULT CALLBACK
HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    mvViewport* viewport;
    if (msg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        viewport = reinterpret_cast<mvViewport*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)viewport);
    }
    else
    {
        LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        viewport = reinterpret_cast<mvViewport*>(ptr);
    }

    switch (msg)
    {

    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            RECT rect;
            RECT crect;
            int awidth = 0;
            int aheight = 0;
            int cwidth = 0;
            int cheight = 0;
            if (GetWindowRect(hWnd, &rect))
            {
                awidth = rect.right - rect.left;
                aheight = rect.bottom - rect.top;
            }

            if (GetClientRect(hWnd, &crect))
            {
                cwidth = crect.right - crect.left;
                cheight = crect.bottom - crect.top;
            }

            viewport->width = cwidth;
            viewport->height = cheight;
            viewport->resized = true;

        }
        return 0;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }

    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
