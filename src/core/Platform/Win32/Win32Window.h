// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Platform/Interfaces/IWindow.h"

#include "Common/Common.h"

#include <Windows.h>
#include <unordered_map>

namespace chronicle::platform::internal::win32
{

class Win32Window : public IWindow<Win32Window>, private NonCopyable<Win32Window>
{
  protected:
    explicit Win32Window(const WindowCreateInfo &createInfo);
    ~Win32Window();

  public:
    void publishEvents()
    {
        _dispatcher.update();
    }

    template <typename Type> [[nodiscard]] entt::sigh<void(Type &)>::sink_type sink()
    {
        return _dispatcher.template sink<Type>();
    }

    void *hwnd() const
    {
        return _hwnd;
    }

    static std::shared_ptr<Win32Window> create(const WindowCreateInfo &createInfo);

  private:
    HWND _hwnd{};
    uint32_t _width{};
    uint32_t _height{};
    uint32_t _frameWidth{};
    uint32_t _frameHeight{};
    float _aspectRatio{};
    bool _frame{true};
    RECT _rect{};
    DWORD _style{};

    DispatcherReaderWriter _dispatcher{};

    static inline std::unordered_map<HWND, Win32Window *> _windowHandlers{};

    void clear();
    void adjust(uint32_t width, uint32_t height, bool windowFrame);

    static LRESULT CALLBACK wndProc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam);
};

} // namespace chronicle::platform::internal::win32