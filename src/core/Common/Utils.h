// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

namespace chronicle
{

#define CHR_CONCRETE(x)                                                                                                \
    class Concrete##x : public x                                                                                       \
    {                                                                                                                  \
      public:                                                                                                          \
        template <typename... Args> explicit Concrete##x(Args &&...args) : x(std::forward<Args>(args)...)              \
        {                                                                                                              \
        }                                                                                                              \
    }

// CRTP helpers
#define CRTP_CONST_THIS static_cast<const T *>(this)
#define CRTP_THIS static_cast<T *>(this)

template <class T> class NonCopyable
{
  protected:
    constexpr NonCopyable() = default;
    ~NonCopyable() = default;

    // disallow copy constructor
    NonCopyable(const NonCopyable &) = delete;

    // disallow copy assignment
    NonCopyable &operator=(const NonCopyable &) = delete;
};

} // namespace chronicle