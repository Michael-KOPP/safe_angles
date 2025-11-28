#pragma once
#include <numbers>
#include <concepts>
#include <cmath>

namespace mkp::trigo {

    template<typename T>
    consteval T degrees_to_radians_factor() {
        return std::numbers::pi_v<T> / T(180);
    }

    template<typename T>
    consteval T radians_to_degrees_factor() {
        return T(180.) / std::numbers::pi_v<T>;
    }

    template<typename T, typename Tag>
        requires(std::floating_point<T>)
    class angle_unit {
    public:
        using underlying_type = T;
        using value_type = angle_unit<T, Tag>;
        using reference = value_type&;

        explicit constexpr angle_unit(T val = {}) noexcept : val_(val) {}

        [[nodiscard]] constexpr T value() noexcept { return val_; }

        [[nodiscard]] friend constexpr auto operator<=>(value_type a, value_type b) noexcept {
            return a.val_ <=> b.val_;
        }

        [[nodiscard]] constexpr value_type operator-() const noexcept {
            return value_type(-val_);
        }

        [[nodiscard]] constexpr value_type operator+() const noexcept {
            return *this;
        }

        constexpr reference operator+=(value_type a) noexcept {
            val_ += a.val_;
            return *this;
        }

        constexpr reference operator-=(value_type a) noexcept {
            val_ -= a.val_;
            return *this;
        }

        constexpr reference operator*=(T t) noexcept {
            val_ *= t;
            return *this;
        }

        constexpr reference operator/=(T t) noexcept {
            val_ /= t;
            return *this;
        }

        [[nodiscard]] friend constexpr value_type operator+(value_type a, value_type b) noexcept {
            return value_type(a.val_ + b.val_);
        }

        [[nodiscard]] friend constexpr value_type operator-(value_type a, value_type b) noexcept {
            return value_type(a.val_ - b.val_);
        }

        [[nodiscard]] friend constexpr value_type operator*(value_type a, T t) noexcept {
            return value_type(a.val_ * t);
        }

        [[nodiscard]] friend constexpr value_type operator*(T t, value_type a) noexcept {
            return value_type(t * a.val_);
        }

        [[nodiscard]] friend constexpr value_type operator/(value_type a, T t) noexcept {
            return value_type(a.val_ / t);
        }
    private:
        T val_;
    };

    template<typename T>
    using degrees = angle_unit<T, struct degrees_tag>;
    using degrees_f = degrees<float>;
    using degrees_d = degrees<double>;

    template<typename T>
    using radians = angle_unit<T, struct radians_tag>;
    using radians_f = radians<float>;
    using radians_d = radians<double>;

    template<typename T>
    struct is_angle_unit : std::false_type {};

    template<typename T, typename Tag>
    struct is_angle_unit<angle_unit<T, Tag>> : std::true_type {};

    template<typename T>
    static constexpr bool is_angle_unit_v = is_angle_unit<T>::value;

    template<typename T>
    concept AngleUnit = is_angle_unit_v<T>;

    template<AngleUnit T>
    using angle_underlying_t = T::underlying_type;

    template<typename>
    inline constexpr bool always_false_v = false;

    //Define conversion cast
    //Choice here is to define peer to peer conversions instead of doing it via a reference unit (rad for example)
    //It allows optimizations
    template<AngleUnit T, AngleUnit U>
    requires(!std::is_same_v<T, U>
            && std::is_same_v<angle_underlying_t<T>, angle_underlying_t<U>>)
    [[nodiscard]] constexpr T angle_cast(U u) noexcept {
        //If this static assert compiles it means you are using a conversion that is not implemented, see below
        static_assert(always_false_v<T>, "implement conversion from U to T");
    }

    //Identity cast

    template<AngleUnit T>
    [[nodiscard]] constexpr T angle_cast(T t) noexcept {
        return t;
    }

    //Supported conversions

    template<typename T>
    [[nodiscard]] constexpr radians<T> angle_cast(degrees<T> t) noexcept {
        return radians<T>(t.value() * degrees_to_radians_factor<T>());
    }

    template<typename T>
    [[nodiscard]] constexpr degrees<T> angle_cast(radians<T> t) noexcept {
        return degrees<T>(t.value() * radians_to_degrees_factor<T>());
    }

    //Functions

    template<typename T, typename Tag>
    [[nodiscard]] constexpr T sin(angle_unit<T, Tag> angle) noexcept {
        return std::sin(angle_cast<radians<T>>(angle).value());
    }

    template<typename T, typename Tag>
    [[nodiscard]] constexpr T cos(angle_unit<T, Tag> angle) noexcept {
        return std::cos(angle_cast<radians<T>>(angle).value());
    }

    template<typename T, typename Tag>
    [[nodiscard]] constexpr T tan(angle_unit<T, Tag> angle) noexcept {
        return std::tan(angle_cast<radians<T>>(angle).value());
    }

    template<typename T>
    [[nodiscard]] constexpr radians<T> asin(T x) noexcept {
        return radians<T>(std::asin(x));
    }

    template<typename T>
    [[nodiscard]] constexpr radians<T> acos(T x) noexcept {
        return radians<T>(std::acos(x));
    }

    template<typename T>
    [[nodiscard]] constexpr radians<T> atan(T x) noexcept {
        return radians<T>(std::atan(x));
    }

    template<typename T>
    [[nodiscard]] constexpr radians<T> atan2(T y, T x) noexcept {
        return radians<T>(std::atan2(y, x));
    }
}

//Std format support

#include <format>

template<typename T>
struct std::formatter<::mkp::trigo::radians<T>> : std::formatter<T>
{
    template <class FormatContext>
    auto format(::mkp::trigo::radians<T> angle, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}rd", angle.value());
    }
};

template<typename T>
struct std::formatter<::mkp::trigo::degrees<T>> : std::formatter<T>
{
    template <class FormatContext>
    auto format(::mkp::trigo::degrees<T> angle, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}°", angle.value());
    }
};

//Iostream support

#include <iostream>

namespace mkp::trigo {
    template<typename T, typename Tag>
    std::ostream& operator<<(std::ostream& os, angle_unit<T, Tag> angle) {
        os << std::format("{}", angle);
        return os;
    }
}

//Custom litterals
//Consider using namespace mkp::trigo_literals to use them

namespace mkp::trigo_literals {
    constexpr ::mkp::trigo::degrees_f operator"" _degf(long double v) {
        return ::mkp::trigo::degrees_f(static_cast<float>(v));
    }

    constexpr ::mkp::trigo::degrees_d operator"" _degd(long double v) {
        return ::mkp::trigo::degrees_d(v);
    }

    constexpr ::mkp::trigo::radians_f operator"" _radf(long double v) {
        return ::mkp::trigo::radians_f(static_cast<float>(v));
    }

    constexpr ::mkp::trigo::radians_d operator"" _radd(long double v) {
        return ::mkp::trigo::radians_d(v);
    }
}
