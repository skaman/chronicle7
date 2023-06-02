// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <memory>

#include "Common/Common.h"

namespace chronicle::platform
{

struct WindowCreateInfo
{
    uint32_t width{1366};
    uint32_t height{768};
};

struct MouseEvent
{
};

struct WindowCloseEvent
{
};

template <class T> class IWindow
{
  public:
    template <typename Type> [[nodiscard]] entt::sigh<void(Type &)>::sink_type sink()
    {
        return CRTP_THIS->template sink<Type>();
    }

    static std::shared_ptr<IWindow> create(const WindowCreateInfo &createInfo)
    {
        return T::create(createInfo);
    }

  private:
    IWindow() = default;
    friend T;
};

} // namespace chronicle::platform