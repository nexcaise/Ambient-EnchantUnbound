#ifndef NC_MACRO
#define NC_MACRO

#if defined(__cplusplus)
    #define NC_CXX 1
#else
    #define NC_CXX 0
#endif

#if defined(__ANDROID__)
    #define NC_PLATFORM_ANDROID 1
#else
    #define NC_PLATFORM_ANDROID 0
#endif

#if defined(__clang__)
    #define NC_COMPILER_CLANG 1
#else
    #define NC_COMPILER_CLANG 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
    #define NC_COMPILER_GCC 1
#else
    #define NC_COMPILER_GCC 0
#endif

#define NC_STRINGIFY_IMPL(x) #x
#define NC_STRINGIFY(x) NC_STRINGIFY_IMPL(x)
#define NC_CONCAT_IMPL(a, b) a##b
#define NC_CONCAT(a, b) NC_CONCAT_IMPL(a, b)

#if defined(_WIN32) || defined(_WIN64)
    #define NC_EXPORT __declspec(dllexport)
    #define NC_IMPORT __declspec(dllimport)
    #define NC_HIDDEN
    #define NC_LOCAL
#else
    #if defined(__GNUC__) || defined(__clang__)
        #define NC_EXPORT __attribute__((visibility("default")))
        #define NC_IMPORT
        #define NC_HIDDEN __attribute__((visibility("hidden")))
        #define NC_LOCAL  __attribute__((visibility("hidden")))
    #else
        #define NC_EXPORT
        #define NC_IMPORT
        #define NC_HIDDEN
        #define NC_LOCAL
    #endif
#endif

#if defined(NC_BUILD_SHARED) || defined(NC_BUILDING_SHARED) || defined(NC_BUILD_DLL) || defined(NC_BUILDING_DLL)
    #define NC_API NC_EXPORT
#elif defined(NC_USE_SHARED) || defined(NC_USING_SHARED) || defined(NC_USE_DLL) || defined(NC_USING_DLL)
    #define NC_API NC_IMPORT
#else
    #define NC_API
#endif

#define NC_PUBLIC   NC_API
#define NC_PRIVATE  NC_LOCAL
#define NC_VISIBLE  NC_EXPORT
#define NC_INVISIBLE NC_HIDDEN

#if NC_CXX
    #define NC_EXTERN_C_BEGIN extern "C" {
    #define NC_EXTERN_C_END }
#else
    #define NC_EXTERN_C_BEGIN
    #define NC_EXTERN_C_END
#endif

#if NC_CXX
    #if defined(__has_cpp_attribute)
        #if __has_cpp_attribute(nodiscard)
            #define NC_NODISCARD [[nodiscard]]
        #else
            #define NC_NODISCARD __attribute__((warn_unused_result))
        #endif

        #if __has_cpp_attribute(maybe_unused)
            #define NC_MAYBE_UNUSED [[maybe_unused]]
        #else
            #define NC_MAYBE_UNUSED __attribute__((unused))
        #endif

        #if __has_cpp_attribute(deprecated)
            #define NC_DEPRECATED(msg) [[deprecated(msg)]]
        #else
            #define NC_DEPRECATED(msg) __attribute__((deprecated(msg)))
        #endif

        #if __has_cpp_attribute(fallthrough)
            #define NC_FALLTHROUGH [[fallthrough]]
        #else
            #define NC_FALLTHROUGH __attribute__((fallthrough))
        #endif
    #else
        #define NC_NODISCARD __attribute__((warn_unused_result))
        #define NC_MAYBE_UNUSED __attribute__((unused))
        #define NC_DEPRECATED(msg) __attribute__((deprecated(msg)))
        #define NC_FALLTHROUGH __attribute__((fallthrough))
    #endif
#else
    #define NC_NODISCARD __attribute__((warn_unused_result))
    #define NC_MAYBE_UNUSED __attribute__((unused))
    #define NC_DEPRECATED(msg) __attribute__((deprecated(msg)))
    #define NC_FALLTHROUGH __attribute__((fallthrough))
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define NC_NORETURN __attribute__((noreturn))
    #define NC_ALWAYS_INLINE inline __attribute__((always_inline))
    #define NC_NEVER_INLINE __attribute__((noinline))
    #define NC_FLATTEN __attribute__((flatten))
    #define NC_HOT __attribute__((hot))
    #define NC_COLD __attribute__((cold))
    #define NC_PURE __attribute__((pure))
    #define NC_CONST __attribute__((const))
    #define NC_PACKED __attribute__((packed))
    #define NC_ALIGNED(n) __attribute__((aligned(n)))
    #define NC_WEAK __attribute__((weak))
    #define NC_ALIAS(sym) __attribute__((alias(sym)))
    #define NC_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
    #define NC_RETURNS_NONNULL __attribute__((returns_nonnull))
    #define NC_PRINTF(fmt_idx, first_arg) __attribute__((format(printf, fmt_idx, first_arg)))
    #define NC_SCANF(fmt_idx, first_arg) __attribute__((format(scanf, fmt_idx, first_arg)))
    #define NC_UNUSED __attribute__((unused))
    #define NC_USED __attribute__((used))
    #define NC_CONSTRUCTOR __attribute__((constructor))
    #define NC_DESTRUCTOR __attribute__((destructor))
#else
    #define NC_NORETURN
    #define NC_ALWAYS_INLINE inline
    #define NC_NEVER_INLINE
    #define NC_FLATTEN
    #define NC_HOT
    #define NC_COLD
    #define NC_PURE
    #define NC_CONST
    #define NC_PACKED
    #define NC_ALIGNED(n)
    #define NC_WEAK
    #define NC_ALIAS(sym)
    #define NC_NONNULL(...)
    #define NC_RETURNS_NONNULL
    #define NC_PRINTF(fmt_idx, first_arg)
    #define NC_SCANF(fmt_idx, first_arg)
    #define NC_UNUSED
    #define NC_USED
    #define NC_CONSTRUCTOR
    #define NC_DESTRUCTOR
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define NC_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define NC_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define NC_ASSUME(x) do { if (!(x)) __builtin_unreachable(); } while (0)
    #define NC_UNREACHABLE() __builtin_unreachable()
#else
    #define NC_LIKELY(x)   (x)
    #define NC_UNLIKELY(x) (x)
    #define NC_ASSUME(x) ((void)0)
    #define NC_UNREACHABLE() ((void)0)
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define NC_RESTRICT __restrict__
#else
    #define NC_RESTRICT restrict
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define NC_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
    #define NC_NO_SANITIZE_THREAD  __attribute__((no_sanitize_thread))
    #define NC_NO_SANITIZE_UNDEFINED __attribute__((no_sanitize("undefined")))
#else
    #define NC_NO_SANITIZE_ADDRESS
    #define NC_NO_SANITIZE_THREAD
    #define NC_NO_SANITIZE_UNDEFINED
#endif

#define API NC_API
#define NAPI NC_HIDDEN
#define LoadAPI NC_CONSTRUCTOR
#define UnloadAPI NC_DESTRUCTOR

#if NC_CXX
    #define CAPI extern "C" API
#else
    #define CAPI extern API
#endif

#define API_PUBLIC   NC_PUBLIC
#define API_PRIVATE  NC_PRIVATE
#define API_EXPORT   NC_EXPORT
#define API_IMPORT   NC_IMPORT
#define API_VISIBLE  NC_VISIBLE
#define API_HIDDEN   NC_INVISIBLE
#define API_USED     NC_USED
#define API_UNUSED   NC_UNUSED
#define API_MAYBE_UNUSED NC_MAYBE_UNUSED
#define API_NODISCARD NC_NODISCARD
#define API_DEPRECATED(msg) NC_DEPRECATED(msg)
#define API_NORETURN NC_NORETURN
#define API_ALWAYS_INLINE NC_ALWAYS_INLINE
#define API_NEVER_INLINE NC_NEVER_INLINE
#define API_FLATTEN NC_FLATTEN
#define API_HOT NC_HOT
#define API_COLD NC_COLD
#define API_PURE NC_PURE
#define API_CONST NC_CONST
#define API_PACKED NC_PACKED
#define API_ALIGNED(n) NC_ALIGNED(n)
#define API_WEAK NC_WEAK
#define API_ALIAS(sym) NC_ALIAS(sym)
#define API_NONNULL(...) NC_NONNULL(__VA_ARGS__)
#define API_RETURNS_NONNULL NC_RETURNS_NONNULL
#define API_PRINTF(fmt_idx, first_arg) NC_PRINTF(fmt_idx, first_arg)
#define API_SCANF(fmt_idx, first_arg) NC_SCANF(fmt_idx, first_arg)
#define API_LIKELY(x) NC_LIKELY(x)
#define API_UNLIKELY(x) NC_UNLIKELY(x)
#define API_ASSUME(x) NC_ASSUME(x)
#define API_UNREACHABLE() NC_UNREACHABLE()
#define API_RESTRICT NC_RESTRICT
#define API_FALLTHROUGH NC_FALLTHROUGH
#define API_CONSTRUCTOR NC_CONSTRUCTOR
#define API_DESTRUCTOR NC_DESTRUCTOR
#define API_NO_SANITIZE_ADDRESS NC_NO_SANITIZE_ADDRESS
#define API_NO_SANITIZE_THREAD NC_NO_SANITIZE_THREAD
#define API_NO_SANITIZE_UNDEFINED NC_NO_SANITIZE_UNDEFINED

#endif