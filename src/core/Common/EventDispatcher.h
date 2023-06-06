// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <concurrentqueue/concurrentqueue.h>
#include <entt/entt.hpp>
#include <readerwriterqueue/readerwriterqueue.h>

using namespace entt;

namespace chronicle
{

namespace internal
{
class BasicDispatcherHandler
{
  public:
    virtual ~BasicDispatcherHandler() = default;
    virtual void publish() = 0;
};
} // namespace internal

/// @brief Dispatcher handler based on a single producer, single consumer queue.
/// @tparam TEvent Type of event.
template <typename TEvent> class DispatcherHandlerReaderWriter final : public internal::BasicDispatcherHandler
{
  public:
    /// @brief Deliver all the events.
    void publish() override
    {
        TEvent evn;
        while (_queue.try_dequeue(evn))
        {
            _signal.publish(evn);
        }
    }

    /// @brief Enqueue an event.
    /// @param evn Event to enqueue.
    void enqueue(const TEvent &evn)
    {
        _queue.emplace(evn);
    }

    /// @brief Return a sink for the event.
    /// @return A temporary sink object.
    [[nodiscard]] entt::sigh<void(TEvent &)>::sink_type sink() noexcept
    {
        return typename entt::sigh<void(TEvent &)>::sink_type{_signal};
    }

  private:
    moodycamel::ReaderWriterQueue<TEvent> _queue{}; /// Single producer, single writer queue.
    entt::sigh<void(TEvent &)> _signal{};           /// Signal handler.
};

/// @brief Proxy for allocate and handle a dispatcher for a single producer, single consumer queue.
class DispatcherHandlerProxyReaderWriter
{
  public:
    /// @brief Allocate a dispatcher handler.
    /// @tparam Type Event type.
    /// @return Dispatcher handler.
    template <typename Type> [[nodiscard]] std::shared_ptr<internal::BasicDispatcherHandler> allocate()
    {
        return std::make_shared<DispatcherHandlerReaderWriter<Type>>();
    }

    /// @brief Enqueue an event into the dispacher handler.
    /// @tparam Type Event type.
    /// @param handler Dispatcher handler.
    /// @param data Data to enqueue.
    template <typename Type> void enqueue(std::shared_ptr<internal::BasicDispatcherHandler> handler, const Type &data)
    {
        static_cast<DispatcherHandlerReaderWriter<Type> *>(handler.get())->enqueue(std::move(data));
    }

    /// @brief Return a sink for the event from the dispatcher handler.
    /// @tparam Type Event type.
    /// @param handler Dispatcher handler.
    /// @return A temporary sink object.
    template <typename Type>
    [[nodiscard]] entt::sigh<void(Type &)>::sink_type sink(std::shared_ptr<internal::BasicDispatcherHandler> handler)
    {
        return static_cast<DispatcherHandlerReaderWriter<Type> *>(handler.get())->sink();
    }
};

/// @brief Dispatcher handler based on a concurrent queue.
/// @tparam TEvent Type of event.
template <typename TEvent> class DispatcherHandlerConcurrent final : public internal::BasicDispatcherHandler
{
  public:
    /// @brief Deliver all the events.
    void publish() override
    {
        TEvent evn;
        while (_queue.try_dequeue(evn))
        {
            _signal.publish(evn);
        }
    }

    /// @brief Enqueue an event.
    /// @param evn Event to enqueue.
    void enqueue(const TEvent &evn)
    {
        _queue.enqueue(evn);
    }

    /// @brief Return a sink for the event.
    /// @return A temporary sink object.
    [[nodiscard]] entt::sigh<void(TEvent &)>::sink_type sink() noexcept
    {
        return typename entt::sigh<void(TEvent &)>::sink_type{_signal};
    }

  private:
    moodycamel::ConcurrentQueue<TEvent> _queue{}; /// Single producer, single writer queue.
    entt::sigh<void(TEvent &)> _signal{};         /// Signal handler.
};

/// @brief Proxy for allocate and handle a dispatcher for concurrent queue.
class DispatcherHandlerProxyConcurrent
{
  public:
    /// @brief Allocate a dispatcher handler.
    /// @tparam Type Event type.
    /// @return Dispatcher handler.
    template <typename Type> [[nodiscard]] std::shared_ptr<internal::BasicDispatcherHandler> allocate()
    {
        return std::make_shared<DispatcherHandlerConcurrent<Type>>();
    }

    /// @brief Enqueue an event into the dispacher handler.
    /// @tparam Type Event type.
    /// @param handler Dispatcher handler.
    /// @param data Data to enqueue.
    template <typename Type> void enqueue(std::shared_ptr<internal::BasicDispatcherHandler> handler, const Type &data)
    {
        static_cast<DispatcherHandlerConcurrent<Type> *>(handler.get())->enqueue(std::move(data));
    }

    /// @brief Return a sink for the event from the dispatcher handler.
    /// @tparam Type Event type.
    /// @param handler Dispatcher handler.
    /// @return A temporary sink object.
    template <typename Type>
    [[nodiscard]] entt::sigh<void(Type &)>::sink_type sink(std::shared_ptr<internal::BasicDispatcherHandler> handler)
    {
        return static_cast<DispatcherHandlerConcurrent<Type> *>(handler.get())->sink();
    }
};

/// @brief Dispatcher handler based on a vector.
/// @tparam TEvent Type of event.
template <typename TEvent> class DispatcherHandlerVector final : public internal::BasicDispatcherHandler
{
  public:
    /// @brief Deliver all the events.
    void publish() override
    {
        const auto length = _queue.size();

        for (std::size_t pos{}; pos < length; ++pos)
        {
            _signal.publish(_queue[pos]);
        }

        _queue.erase(_queue.cbegin(), _queue.cbegin() + length);
    }

    /// @brief Enqueue an event.
    /// @param evn Event to enqueue.
    void enqueue(const TEvent &evn)
    {
        _queue.push_back(evn);
    }

    /// @brief Return a sink for the event.
    /// @return A temporary sink object.
    [[nodiscard]] entt::sigh<void(TEvent &)>::sink_type sink() noexcept
    {
        return typename entt::sigh<void(TEvent &)>::sink_type{_signal};
    }

  private:
    std::vector<TEvent> _queue{};         /// Single producer, single writer queue.
    entt::sigh<void(TEvent &)> _signal{}; /// Signal handler.
};

/// @brief Proxy for allocate and handle a dispatcher for vector queue.
class DispatcherHandlerProxyVector
{
  public:
    /// @brief Allocate a dispatcher handler.
    /// @tparam Type Event type.
    /// @return Dispatcher handler.
    template <typename Type> [[nodiscard]] std::shared_ptr<internal::BasicDispatcherHandler> allocate()
    {
        return std::make_shared<DispatcherHandlerVector<Type>>();
    }

    /// @brief Enqueue an event into the dispacher handler.
    /// @tparam Type Event type.
    /// @param handler Dispatcher handler.
    /// @param data Data to enqueue.
    template <typename Type> void enqueue(std::shared_ptr<internal::BasicDispatcherHandler> handler, const Type &data)
    {
        static_cast<DispatcherHandlerVector<Type> *>(handler.get())->enqueue(std::move(data));
    }

    /// @brief Return a sink for the event from the dispatcher handler.
    /// @tparam Type Event type.
    /// @param handler Dispatcher handler.
    /// @return A temporary sink object.
    template <typename Type>
    [[nodiscard]] entt::sigh<void(Type &)>::sink_type sink(std::shared_ptr<internal::BasicDispatcherHandler> handler)
    {
        return static_cast<DispatcherHandlerVector<Type> *>(handler.get())->sink();
    }
};

/// @brief Basic dispatcher for collect and dispatch events.
/// @tparam THandlerProxy Proxy for handle a specialized dispatcher.
template <typename THandlerProxy> class BasicDispatcher
{
  public:
    /// @brief Enqueue an event.
    /// @tparam Type Event type.
    /// @param value Event data.
    template <typename Type> void enqueue(const Type &value)
    {
        _proxy.enqueue<Type>(handler<Type>(), std::move(value));
    }

    /// @brief Return a sink for an event type.
    /// @tparam Type Event type.
    /// @return A temporary sink object.
    template <typename Type> [[nodiscard]] entt::sigh<void(Type &)>::sink_type sink()
    {
        return _proxy.sink<Type>(handler<Type>());
    }

    /// @brief Deliver all the peding events for a specific event type.
    /// @tparam Type Event type.
    template <typename Type> void update()
    {
        constexpr auto id = entt::type_hash<Type>::value();
        handler<Type>(id)->publish();
    }

    /// @brief Deliver all the pending events.
    void update()
    {
        for (const auto &[_, handler] : _handlers)
        {
            handler->publish();
        }
    }

  private:
    THandlerProxy _proxy{}; /// Specialized dispatcher proxy.
    entt::dense_map<entt::id_type, std::shared_ptr<internal::BasicDispatcherHandler>>
        _handlers{}; /// Dispatcher handlers.

    /// @brief Return an event from a specific event type.
    /// @tparam Type Event type.
    /// @return Dispatcher handler.
    template <typename Type> std::shared_ptr<internal::BasicDispatcherHandler> handler()
    {
        constexpr auto id = entt::type_hash<Type>::value();
        auto &handler = _handlers[id];
        if (!handler)
        {
            handler = _proxy.allocate<Type>();
        }
        return handler;
    }
};

/// @brief Dispatcher for single producer, single consumer events.
using DispatcherReaderWriter = BasicDispatcher<DispatcherHandlerProxyReaderWriter>;

/// @brief Dispatcher for concurrent events.
using DispatcherConcurrent = BasicDispatcher<DispatcherHandlerProxyConcurrent>;

/// @brief Dispatcher for single thread events.
using Dispatcher = BasicDispatcher<DispatcherHandlerProxyVector>;

} // namespace chronicle