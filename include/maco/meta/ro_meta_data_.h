//
// Created by skybcyang on 2020/12/19.
//

#ifndef MACO_RO_META_DATA__H
#define MACO_RO_META_DATA__H

#include <maco/map.h>
#include <maco/aggregate.h>
#include <maco/meta/meta_data_common.h>
#include <type_traits>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////////////////////
namespace ro_meta_data_ {

    template<typename T>
    struct meta_type_trait {
        using type = T;
        using element_type = T;
        using parameter_type = typename meta_data::parameter_type_trait<T>::type;
        using return_type = parameter_type;
        using value_type = T;

        template<typename F>
        constexpr static bool is_visitable = std::is_invocable_v<F, value_type>;

        template<typename F>
        constexpr static bool is_void_visitable = std::is_invocable_r_v<void, F, value_type>;

        template<typename F>
        using invoke_result_t = std::invoke_result_t<F, value_type>;

        template<typename F>
        constexpr static bool is_modifiable = std::is_invocable_v<F, value_type&>;

        inline constexpr static auto set(value_type& self, parameter_type value) -> void {
            self = value;
        }

        inline constexpr static auto get(const value_type& self) -> return_type {
            return self;
        }

        template<typename F>
        inline constexpr static auto visit(parameter_type self, F&& f) {
            return f(self);
        }

        template<typename F>
        inline static auto modify(T& self, F&& f) -> void {
            f(self);
        }
    };

    template<typename C, size_t SIZE>
    struct meta_type_trait<C[SIZE]> {
        using type = C[SIZE];
        using element_type = C;
        using parameter_type = std::pair<C const*, size_t>;
        using return_type = parameter_type;

        using value_type = struct {
            type data_;
            size_t n_;
        };

        template<typename F>
        constexpr static bool is_visitable = std::is_invocable_v<F, const C*, size_t>;

        template<typename F>
        constexpr static bool is_void_visitable = std::is_invocable_r_v<void, F, const C*, size_t>;

        template<typename F>
        using invoke_result_t = std::invoke_result_t<F, const C*, size_t>;

        template<typename F>
        constexpr static bool is_modifiable = std::is_invocable_v<F, C*&, size_t&>;

        inline constexpr static auto set(value_type& self, parameter_type value) -> void {
            auto [p, size] = value;
            auto total = std::min(SIZE, size);
            self.n_ = total;
            for(size_t i=0; i<total; i++) {
                self.data_[i] = p[i];
            }
        }

        inline constexpr static auto get(const value_type& self) -> return_type {
            return {self.data_, self.n_};
        }

        template<typename F>
        inline constexpr static auto visit(const value_type& self, F&& f)  {
            auto [p, size] = self;
            return f(p, size);
        }

        template<typename F>
        inline static auto modify(value_type& self, F&& f) -> void {
            auto& [p, size] = self;
            f(p, size);
        }
    };

    template<typename C>
    struct meta_type_trait<C[1]> : meta_type_trait<C> {};


    template<size_t N>
    struct meta_flags {
        enum { num_of_bytes = (N + 7) / 8 };
        unsigned char flags_[num_of_bytes]{};
    };

    template<typename F_SOME, typename F_NONE>
    constexpr bool same_result =
            std::is_same_v<
                    typename meta_type_trait<F_SOME>::template invoke_result_t<F_SOME>,
                    meta_data::none_invoke_result_t<F_NONE>>;
}

///////////////////////////////////////////////////////////////////////////////////////////////
#define __RO_Meta_ns ro_meta_data_
#define __RO_MeTa(x) __RO_Meta_ns::meta_type_trait<__MACO_var_type(x)>
#define __RO_Meta_value_type(x) typename __RO_MeTa(x)::value_type
#define __RO_Meta_result(x) typename __RO_MeTa(x)::return_type
#define __RO_Meta_para(x) typename __RO_MeTa(x)::parameter_type
#define __RO_Meta_elem(x) typename __RO_MeTa(x)::element_type

///////////////////////////////////////////////////////////////////////////////////////////////
#define __CUB_no_lock_meta_field__(n, x, set_visibility)                                \
private:                                                                                \
struct __MeTa_ro_cls(x){                                                                   \
private:                                                                                \
    __RO_Meta_value_type(x) __MeTa_var(x);                                              \
    bool present_flag = false;                                                          \
public:                                                                                 \
    inline constexpr auto get() const noexcept -> __RO_Meta_result(x) {                 \
       return __RO_MeTa(x)::get(__MeTa_var(x));                                         \
    }                                                                                   \
    inline constexpr auto present() const noexcept -> bool {                            \
       return present_flag;                                                             \
    }                                                                                   \
set_visibility:                                                                         \
   template<size_t SIZE> inline                                                         \
   auto set(const __RO_Meta_elem(x) (&array)[SIZE]) noexcept -> void {                  \
      set({array, SIZE});                                                               \
   }                                                                                    \
   inline auto set(__RO_Meta_para(x) pair) noexcept -> void {                           \
      __RO_MeTa(x)::set(__MeTa_var(x), pair);                                           \
      present_flag = true;                                                              \
   }                                                                                    \
};                                                                                      \
struct __MeTa_rw_cls(x) : __MeTa_ro_cls(x) {                                            \
using __MeTa_ro_cls(x)::set;                                                            \
};                                                                                      \
private:                                                                                \
__MeTa_rw_cls(x) __MACO_var_name(x);                                                    \
set_visibility:                                                                         \
__MeTa_rw_cls(x)& __MeTa_rw_name(x)(){return __MACO_var_name(x);}                       \
public:                                                                                 \
const __MeTa_ro_cls(x)& __MeTa_ro_name(x)() const {return __MACO_var_name(x);}          \


///////////////////////////////////////////////////////////////////////////////////////
#define __CUB_ro_field__(n, x) __CUB_no_lock_meta_field__(n, x, protected)

///////////////////////////////////////////////////////////////////////////////////////
#define __CUB_ro_meta_data(...)                                                \
   __MACO_map_i(__CUB_ro_field__, __VA_ARGS__)                                 \

////////////////////////////////////////////////////////////////////////
#define __CUB_export_meta_w__(x)                  \
using __secrete_parent__::__MeTa_rw_name(x);      \

////////////////////////////////////////////////////////////////////////
#define __CUB_2_stage_meta_table_(rw_stage, ro_stage, ...)   \
struct ro_stage {                                            \
  __CUB_ro_meta_data(__VA_ARGS__);                           \
};                                                           \
struct rw_stage : ro_stage {                                 \
private:                                                     \
  using __secrete_parent__ = ro_stage;                       \
public:                                                      \
  __MACO_map(__CUB_export_meta_w__, __VA_ARGS__);            \
}

#endif //MACO_RO_META_DATA__H
