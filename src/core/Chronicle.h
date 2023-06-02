// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

namespace chronicle
{

namespace platform
{
struct WindowCloseEvent;
}

class Chronicle
{
  public:
    static void init();
    static void deinit();
    static void run();

  private:
    static inline bool _isRunning{true};

    static void onWindowClose(const platform::WindowCloseEvent &evn);
};

} // namespace chronicle