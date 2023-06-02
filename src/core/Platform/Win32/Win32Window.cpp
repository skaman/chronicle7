// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Win32Window.h"

#include <Winuser.h>
#include <algorithm>

namespace chronicle::platform::internal::win32
{

CHR_CONCRETE(Win32Window);

Win32Window::Win32Window(const WindowCreateInfo &createInfo)
{
    SetDllDirectoryA(".");

    auto instance = GetModuleHandle(nullptr);

    WNDCLASSEXW wnd;
    std::memset(&wnd, 0, sizeof(wnd));
    wnd.cbSize = sizeof(wnd);
    wnd.style = CS_HREDRAW | CS_VREDRAW;
    wnd.lpfnWndProc = wndProc;
    wnd.hInstance = instance;
    wnd.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wnd.lpszClassName = L"Chronicle";
    wnd.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wnd);

    _hwnd = CreateWindowExA(WS_EX_ACCEPTFILES, "chronicle", "Chronicle", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0,
                            createInfo.width, createInfo.height, nullptr, nullptr, instance, nullptr);

    _windowHandlers[_hwnd] = this;

    // m_flags[0] = 0 | ENTRY_WINDOW_FLAG_ASPECT_RATIO | ENTRY_WINDOW_FLAG_FRAME;

    adjust(createInfo.width, createInfo.height, true);
    clear();
}

Win32Window::~Win32Window()
{
    _windowHandlers.erase(_hwnd);
    DestroyWindow(_hwnd);
}

std::shared_ptr<Win32Window> Win32Window::create(const WindowCreateInfo &createInfo)
{
    return std::make_shared<ConcreteWin32Window>(createInfo);
}

void Win32Window::clear()
{
    RECT rect;
    GetWindowRect(_hwnd, &rect);
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
    HDC hdc = GetDC(_hwnd);
    SelectObject(hdc, brush);
    FillRect(hdc, &rect, brush);
    ReleaseDC(_hwnd, hdc);
    DeleteObject(brush);
}

void Win32Window::adjust(uint32_t width, uint32_t height, bool windowFrame)
{
    _width = width;
    _height = height;
    _aspectRatio = float(width) / float(height);

    ShowWindow(_hwnd, SW_SHOWNORMAL);
    RECT rect;
    RECT newrect = {0, 0, (LONG)_width, (LONG)_height};
    DWORD style = WS_POPUP | WS_SYSMENU;

    if (_frame)
    {
        GetWindowRect(_hwnd, &_rect);
        _style = GetWindowLong(_hwnd, GWL_STYLE);
    }

    if (windowFrame)
    {
        rect = _rect;
        style = _style;
    }
    else
    {
        HMONITOR monitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{};
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(monitor, &mi);
        newrect = mi.rcMonitor;
        rect = mi.rcMonitor;
        _aspectRatio = float(newrect.right - newrect.left) / float(newrect.bottom - newrect.top);
    }

    SetWindowLong(_hwnd, GWL_STYLE, style);
    uint32_t prewidth = newrect.right - newrect.left;
    uint32_t preheight = newrect.bottom - newrect.top;
    AdjustWindowRect(&newrect, style, FALSE);
    _frameWidth = (newrect.right - newrect.left) - prewidth;
    _frameHeight = (newrect.bottom - newrect.top) - preheight;
    UpdateWindow(_hwnd);

    if (rect.left == -32000 || rect.top == -32000)
    {
        rect.left = 0;
        rect.top = 0;
    }

    int32_t wLeft = rect.left;
    int32_t wTop = rect.top;
    int32_t wWidth = (newrect.right - newrect.left);
    int32_t wHeight = (newrect.bottom - newrect.top);

    if (!windowFrame)
    {
        float aspectRatio = 1.0f / _aspectRatio;
        wWidth = std::max<int32_t>(width / 4, wWidth);
        wHeight = uint32_t(float(wWidth) * aspectRatio);

        wLeft = newrect.left + (newrect.right - newrect.left - width) / 2;
        wTop = newrect.top + (newrect.bottom - newrect.top - height) / 2;
    }

    SetWindowPos(_hwnd, HWND_TOP, wLeft, wTop, wWidth, wHeight, SWP_SHOWWINDOW);

    ShowWindow(_hwnd, SW_RESTORE);

    _frame = windowFrame;
}

LRESULT Win32Window::wndProc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
{
    if (!_windowHandlers.contains(hwnd))
        return DefWindowProcW(hwnd, id, wparam, lparam);

    auto *window = _windowHandlers[hwnd];

    switch (id)
    {
    case WM_DESTROY:
        break;

    case WM_QUIT:
    case WM_CLOSE:
        window->_dispatcher.enqueue(WindowCloseEvent{});
        return 0;

    case WM_SIZING:
        return 0;

    case WM_SIZE:
        break;

    case WM_SYSCOMMAND:
        break;

    case WM_MOUSEMOVE:
        window->_dispatcher.enqueue(MouseEvent{});
        break;

    case WM_MOUSEWHEEL:
        break;

    case WM_LBUTTONDOWN:
        break;

    case WM_LBUTTONUP:
        break;

    case WM_LBUTTONDBLCLK:
        break;

    case WM_MBUTTONDOWN:
        break;

    case WM_MBUTTONUP:
        break;

    case WM_MBUTTONDBLCLK:
        break;

    case WM_RBUTTONDOWN:
        break;

    case WM_RBUTTONUP:
        break;

    case WM_RBUTTONDBLCLK:
        break;

    case WM_KEYDOWN:
        break;

    case WM_SYSKEYDOWN:
        break;

    case WM_KEYUP:
        break;

    case WM_SYSKEYUP:
        break;

    case WM_CHAR:
        break;

    case WM_DROPFILES:
        break;

    default:
        break;
    }

    return DefWindowProcW(hwnd, id, wparam, lparam);
}

} // namespace chronicle::platform::internal::win32