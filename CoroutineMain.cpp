#include<coroutine>
#include<type_traits>
#include<algorithm>
#include<variant>

template <typename T>
struct Generator {
    struct promise_type {
        auto get_return_object() noexcept { return Generator{*this}; }

        std::suspend_always initial_suspend() const noexcept { return {}; }

        std::suspend_always final_suspend() const noexcept { return {}; }

        std::suspend_always yield_value(const T &value) 
        noexcept(std::is_nothrow_copy_constructible_v<T>) 
        {
            result = value;
            return {};
        }

        void return_void() const noexcept {}

        void unhandled_exception() noexcept(
            std::is_nothrow_copy_constructible_v<std::exception_ptr>) {
            result = std::current_exception();
        }
        T &getValue() {
            if (std::holds_alternative<std::exception_ptr>(result))
                std::rethrow_exception(std::get<std::exception_ptr>(result));
            return std::get<T>(result);
        }

       private:
        std::variant<std::monostate, T, std::exception_ptr> result;
    };

    Generator(Generator &&other) noexcept
        : coro{std::exchange(other.coro, nullptr)} {}
    Generator &operator=(Generator &&other) noexcept {
        if (coro) coro.destroy();
        coro = std::exchange(other.coro, nullptr);
    }
    ~Generator() {
        if (coro) coro.destroy();
    }
    auto &operator()() const {
        coro();
        return coro.promise().getValue();
    }

   private:
    explicit Generator(promise_type &promise) noexcept;
    std::coroutine_handle<promise_type> coro;
};
