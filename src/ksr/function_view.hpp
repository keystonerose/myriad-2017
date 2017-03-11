#ifndef KSR_FUNCTION_VIEW_HPP
#define KSR_FUNCTION_VIEW_HPP

#include <memory>
#include <type_traits>
#include <utility>

namespace ksr {

    ///
    /// A lightweight, type-erased reference to a callable object. This is intended to be used in
    /// passing function-like objects as parameters to other functions: the traditional approach of
    /// templating the receiving function can become cumbersome or simply not possible in certain
    /// circumstances (e.g. for virtual functions). \ref function_view is cheap to construct, copy
    /// and move, and can therefore by passed by value. Note that unlike \c std::function,
    /// \ref function_view takes no ownership of the function object passed to its constructor; it
    /// is the responsibility of calling code to ensure that (like \c std::string_view and any other
    /// reference type) the \ref function_view does not outlive the callable object it points to.
    ///

    template <typename>
    class function_view;

    template <typename Ret, typename... Args>
    class function_view<Ret(Args...)> {
    public:

        // TODO Action the [C++17] items here

        // [C++17] It would be useful to constrain this constructor further with the
        // std::is_callable() type trait (though not sufficiently so that it's worth writing such a
        // trait by hand).

        template <
            typename T,
            typename = std::enable_if_t<!std::is_base_of<function_view, std::decay_t<T>>::value>>
        function_view(T&& function) noexcept
          : m_invoke{&function_view::invoke<T>}, m_ptr{std::addressof(function)} {}

        // [C++17] It may be possible to get the noexcept specification of operator() correct once
        // this is part of the type system (though I'm not sure how practical it'll be to do).

        Ret operator()(Args&&... args) const {
            return (this->*m_invoke)(std::forward<Args>(args)...);
        }

    private:

        template <typename T>
        Ret invoke(Args&&... args) const {
            const auto function = reinterpret_cast<std::add_pointer_t<T>>(m_ptr);
            return (*function)(std::forward<Args>(args)...);
        }

        using invoke_inst = Ret (function_view::*)(Args&&...) const;
        invoke_inst m_invoke;
        void* m_ptr;
    };
}

#endif
