// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <string>

namespace chronicle::graphics
{

struct SystemInitInfo
{
    std::string applicationName{"Chronicle"};
    bool enableDebug{};
};

template <class T> class ISystem
{
  public:
    static void init(const SystemInitInfo &systemInitInfo)
    {
        return T::init(systemInitInfo);
    }

    static void deinit()
    {
        return T::deinit();
    }

  private:
    ISystem() = default;
    friend T;
};

} // namespace chronicle::graphics