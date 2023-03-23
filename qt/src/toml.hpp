//----------------------------------------------------------------------------------------------------------------------
//
// toml++ v2.1.0
// https://github.com/marzer/tomlplusplus
// SPDX-License-Identifier: MIT
//
//----------------------------------------------------------------------------------------------------------------------
//
// -         THIS FILE WAS ASSEMBLED FROM MULTIPLE HEADER FILES BY A SCRIPT - PLEASE DON'T EDIT IT DIRECTLY            -
//
// If you wish to submit a contribution to toml++, hooray and thanks! Before you crack on, please be aware that this
// file was assembled from a number of smaller files by a python script, and code contributions should not be made
// against it directly. You should instead make your changes in the relevant source file(s). The file names of the files
// that contributed to this header can be found at the beginnings and ends of the corresponding sections of this file.
//
//----------------------------------------------------------------------------------------------------------------------
//
// TOML Language Specifications:
// latest:      https://github.com/toml-lang/toml/blob/master/README.md
// v1.0.0-rc.2: https://toml.io/en/v1.0.0-rc.2
// v1.0.0-rc.1: https://toml.io/en/v1.0.0-rc.1
// v0.5.0:      https://toml.io/en/v0.5.0
// changelog:   https://github.com/toml-lang/toml/blob/master/CHANGELOG.md
//
//----------------------------------------------------------------------------------------------------------------------
//
// MIT License
//
// Copyright (c) 2019-2020 Mark Gillard <mark.gillard@outlook.com.au>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//----------------------------------------------------------------------------------------------------------------------
#ifndef INCLUDE_TOMLPLUSPLUS_H
#define INCLUDE_TOMLPLUSPLUS_H

#if 1  //------  ↓ toml_preprocessor.h  --------------------------------------------------------------------------------

#ifndef __cplusplus
	#error toml++ is a C++ library.
#endif

#ifdef __INTELLISENSE__
	#define TOML_INTELLISENSE		1
#else
	#define TOML_INTELLISENSE		0
#endif
#ifdef __clang__
	#define TOML_CLANG				__clang_major__
#else
	#define TOML_CLANG				0
#endif
#ifdef __INTEL_COMPILER
	#define TOML_ICC				__INTEL_COMPILER
	#ifdef __ICL
		#define TOML_ICC_CL			TOML_ICC
	#else
		#define TOML_ICC_CL			0
	#endif
#else
	#define TOML_ICC				0
	#define TOML_ICC_CL				0
#endif
#if defined(_MSC_VER) && !TOML_CLANG && !TOML_ICC
	#define TOML_MSVC				_MSC_VER
#else
	#define TOML_MSVC				0
#endif
#if defined(__GNUC__) && !TOML_CLANG && !TOML_ICC
	#define TOML_GCC				__GNUC__
#else
	#define TOML_GCC				0
#endif

#if TOML_CLANG

	#define TOML_PUSH_WARNINGS					_Pragma("clang diagnostic push")
	#define TOML_DISABLE_SWITCH_WARNINGS		_Pragma("clang diagnostic ignored \"-Wswitch\"")
	#define TOML_DISABLE_INIT_WARNINGS			_Pragma("clang diagnostic ignored \"-Wmissing-field-initializers\"")
	#define TOML_DISABLE_ARITHMETIC_WARNINGS	_Pragma("clang diagnostic ignored \"-Wfloat-equal\"") \
												_Pragma("clang diagnostic ignored \"-Wdouble-promotion\"") \
												_Pragma("clang diagnostic ignored \"-Wchar-subscripts\"") \
												_Pragma("clang diagnostic ignored \"-Wshift-sign-overflow\"")
	#define TOML_DISABLE_SHADOW_WARNINGS		_Pragma("clang diagnostic ignored \"-Wshadow\"")
	#define TOML_DISABLE_SPAM_WARNINGS			_Pragma("clang diagnostic ignored \"-Wweak-vtables\"")	\
												_Pragma("clang diagnostic ignored \"-Wweak-template-vtables\"") \
												_Pragma("clang diagnostic ignored \"-Wpadded\"")
	#define TOML_POP_WARNINGS					_Pragma("clang diagnostic pop")
	#define TOML_DISABLE_WARNINGS				TOML_PUSH_WARNINGS \
												_Pragma("clang diagnostic ignored \"-Weverything\"")
	#define TOML_ENABLE_WARNINGS				TOML_POP_WARNINGS
	#define TOML_ASSUME(cond)					__builtin_assume(cond)
	#define TOML_UNREACHABLE					__builtin_unreachable()
	#define TOML_ATTR(...)						__attribute__((__VA_ARGS__))
	#if defined(_MSC_VER) // msvc compat mode
		#ifdef __has_declspec_attribute
			#if __has_declspec_attribute(novtable)
				#define TOML_INTERFACE		__declspec(novtable)
			#endif
			#if __has_declspec_attribute(empty_bases)
				#define TOML_EMPTY_BASES	__declspec(empty_bases)
			#endif
			#ifndef TOML_ALWAYS_INLINE
				#define TOML_ALWAYS_INLINE	__forceinline
			#endif
			#if __has_declspec_attribute(noinline)
				#define TOML_NEVER_INLINE	__declspec(noinline)
			#endif
		#endif
	#endif
	#ifdef __has_attribute
		#if !defined(TOML_ALWAYS_INLINE) && __has_attribute(always_inline)
			#define TOML_ALWAYS_INLINE		__attribute__((__always_inline__)) inline
		#endif
		#if !defined(TOML_NEVER_INLINE) && __has_attribute(noinline)
			#define TOML_NEVER_INLINE		__attribute__((__noinline__))
		#endif
		#if !defined(TOML_TRIVIAL_ABI) && __has_attribute(trivial_abi)
			#define TOML_TRIVIAL_ABI		__attribute__((__trivial_abi__))
		#endif
	#endif
	#define TOML_LIKELY(...)				(__builtin_expect(!!(__VA_ARGS__), 1) )
	#define TOML_UNLIKELY(...)				(__builtin_expect(!!(__VA_ARGS__), 0) )

	//floating-point from_chars and to_chars are not implemented in any version of clang as of 1/1/2020
	#ifndef TOML_FLOAT_CHARCONV
		#define TOML_FLOAT_CHARCONV 0
	#endif

	#define TOML_SIMPLE_STATIC_ASSERT_MESSAGES	1

#endif // clang

#if TOML_MSVC || TOML_ICC_CL

	#define TOML_CPP_VERSION					_MSVC_LANG
	#define TOML_PUSH_WARNINGS					__pragma(warning(push))
	#if TOML_MSVC // !intel-cl
		#define TOML_PUSH_WARNINGS				__pragma(warning(push))
		#define TOML_DISABLE_SWITCH_WARNINGS	__pragma(warning(disable: 4063))
		#define TOML_POP_WARNINGS				__pragma(warning(pop))
		#define TOML_DISABLE_WARNINGS			__pragma(warning(push, 0))
		#define TOML_ENABLE_WARNINGS			TOML_POP_WARNINGS
	#endif
	#ifndef TOML_ALWAYS_INLINE
		#define TOML_ALWAYS_INLINE				__forceinline
	#endif
	#define TOML_NEVER_INLINE					__declspec(noinline)
	#define TOML_ASSUME(cond)					__assume(cond)
	#define TOML_UNREACHABLE					__assume(0)
	#define TOML_INTERFACE						__declspec(novtable)
	#define TOML_EMPTY_BASES					__declspec(empty_bases)
	#if !defined(TOML_RELOPS_REORDERING) && defined(__cpp_impl_three_way_comparison)
		#define TOML_RELOPS_REORDERING		1
	#endif

#endif // msvc

#if TOML_ICC

	#define TOML_PUSH_WARNINGS				__pragma(warning(push))
	#define TOML_DISABLE_SPAM_WARNINGS		__pragma(warning(disable: 82))	/* storage class is not first */ \
											__pragma(warning(disable: 111))	/* statement unreachable (false-positive) */ \
											__pragma(warning(disable: 1011)) /* missing return (false-positive) */ \
											__pragma(warning(disable: 2261)) /* assume expr side-effects discarded */
	#define TOML_POP_WARNINGS				__pragma(warning(pop))
	#define TOML_DISABLE_WARNINGS			__pragma(warning(push, 0))
	#define TOML_ENABLE_WARNINGS				TOML_POP_WARNINGS

#endif // icc

#if TOML_GCC

	#define TOML_PUSH_WARNINGS					_Pragma("GCC diagnostic push")
	#define TOML_DISABLE_SWITCH_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wswitch\"")						\
												_Pragma("GCC diagnostic ignored \"-Wswitch-enum\"")					\
												_Pragma("GCC diagnostic ignored \"-Wswitch-default\"")
	#define TOML_DISABLE_INIT_WARNINGS			_Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"")	\
												_Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")			\
												_Pragma("GCC diagnostic ignored \"-Wuninitialized\"")
	#define TOML_DISABLE_ARITHMETIC_WARNINGS	_Pragma("GCC diagnostic ignored \"-Wfloat-equal\"")					\
												_Pragma("GCC diagnostic ignored \"-Wsign-conversion\"")				\
												_Pragma("GCC diagnostic ignored \"-Wchar-subscripts\"")
	#define TOML_DISABLE_SHADOW_WARNINGS		_Pragma("GCC diagnostic ignored \"-Wshadow\"")
	#define TOML_DISABLE_SPAM_WARNINGS			_Pragma("GCC diagnostic ignored \"-Wpadded\"")						\
												_Pragma("GCC diagnostic ignored \"-Wcast-align\"")					\
												_Pragma("GCC diagnostic ignored \"-Wcomment\"")						\
												_Pragma("GCC diagnostic ignored \"-Wtype-limits\"")					\
												_Pragma("GCC diagnostic ignored \"-Wsuggest-attribute=const\"")		\
												_Pragma("GCC diagnostic ignored \"-Wsuggest-attribute=pure\"")
	#define TOML_POP_WARNINGS					_Pragma("GCC diagnostic pop")
	#define TOML_DISABLE_WARNINGS				TOML_PUSH_WARNINGS													\
												_Pragma("GCC diagnostic ignored \"-Wall\"")							\
												_Pragma("GCC diagnostic ignored \"-Wextra\"")						\
												_Pragma("GCC diagnostic ignored \"-Wpedantic\"")					\
												TOML_DISABLE_SWITCH_WARNINGS										\
												TOML_DISABLE_INIT_WARNINGS											\
												TOML_DISABLE_ARITHMETIC_WARNINGS									\
												TOML_DISABLE_SHADOW_WARNINGS										\
												TOML_DISABLE_SPAM_WARNINGS
	#define TOML_ENABLE_WARNINGS				TOML_POP_WARNINGS

	#define TOML_ATTR(...)						__attribute__((__VA_ARGS__))
	#ifndef TOML_ALWAYS_INLINE
		#define TOML_ALWAYS_INLINE				__attribute__((__always_inline__)) inline
	#endif
	#define TOML_NEVER_INLINE					__attribute__((__noinline__))
	#define TOML_UNREACHABLE					__builtin_unreachable()
	#if !defined(TOML_RELOPS_REORDERING) && defined(__cpp_impl_three_way_comparison)
		#define TOML_RELOPS_REORDERING 1
	#endif
	#define TOML_LIKELY(...)					(__builtin_expect(!!(__VA_ARGS__), 1) )
	#define TOML_UNLIKELY(...)					(__builtin_expect(!!(__VA_ARGS__), 0) )

	// floating-point from_chars and to_chars are not implemented in any version of gcc as of 1/1/2020
	#ifndef TOML_FLOAT_CHARCONV
		#define TOML_FLOAT_CHARCONV 0
	#endif

#endif

#ifdef TOML_CONFIG_HEADER
	#include TOML_CONFIG_HEADER
#endif

#ifdef DOXYGEN
	#define TOML_HEADER_ONLY 0
	#define TOML_WINDOWS_COMPAT 1
#endif

#if defined(TOML_ALL_INLINE) && !defined(TOML_HEADER_ONLY)
	#define TOML_HEADER_ONLY	TOML_ALL_INLINE
#endif

#if !defined(TOML_HEADER_ONLY) || (defined(TOML_HEADER_ONLY) && TOML_HEADER_ONLY) || TOML_INTELLISENSE
	#undef TOML_HEADER_ONLY
	#define TOML_HEADER_ONLY 1
#endif

#if defined(TOML_IMPLEMENTATION) || TOML_HEADER_ONLY
	#undef TOML_IMPLEMENTATION
	#define TOML_IMPLEMENTATION 1
#else
	#define TOML_IMPLEMENTATION 0
#endif

#ifndef TOML_API
	#define TOML_API
#endif

#ifndef TOML_UNRELEASED_FEATURES
	#define TOML_UNRELEASED_FEATURES 0
#endif

#ifndef TOML_LARGE_FILES
	#define TOML_LARGE_FILES 0
#endif

#ifndef TOML_UNDEF_MACROS
	#define TOML_UNDEF_MACROS 1
#endif

#ifndef TOML_PARSER
	#define TOML_PARSER 1
#endif

#ifndef DOXYGEN
	#if defined(_WIN32) && !defined(TOML_WINDOWS_COMPAT)
		#define TOML_WINDOWS_COMPAT 1
	#endif
	#if !defined(_WIN32) || !defined(TOML_WINDOWS_COMPAT)
		#undef TOML_WINDOWS_COMPAT
		#define TOML_WINDOWS_COMPAT 0
	#endif
#endif

#ifdef TOML_OPTIONAL_TYPE
	#define TOML_HAS_CUSTOM_OPTIONAL_TYPE 1
#else
	#define TOML_HAS_CUSTOM_OPTIONAL_TYPE 0
#endif

#ifdef TOML_CHAR_8_STRINGS
	#if TOML_CHAR_8_STRINGS
		#error TOML_CHAR_8_STRINGS was removed in toml++ 2.0.0; \
all value setters and getters can now work with char8_t strings implicitly so changing the underlying string type \
is no longer necessary.
	#endif
#endif

#ifndef TOML_CPP_VERSION
	#define TOML_CPP_VERSION __cplusplus
#endif
#if TOML_CPP_VERSION < 201103L
	#error toml++ requires C++17 or higher. For a TOML library supporting pre-C++11 see https://github.com/ToruNiina/Boost.toml
#elif TOML_CPP_VERSION < 201703L
	#error toml++ requires C++17 or higher. For a TOML library supporting C++11 see https://github.com/ToruNiina/toml11
#elif TOML_CPP_VERSION >= 202600L
	#define TOML_CPP 26
#elif TOML_CPP_VERSION >= 202300L
	#define TOML_CPP 23
#elif TOML_CPP_VERSION >= 202002L
	#define TOML_CPP 20
#elif TOML_CPP_VERSION >= 201703L
	#define TOML_CPP 17
#endif
#undef TOML_CPP_VERSION

#ifdef __has_include
	#define TOML_HAS_INCLUDE(header)		__has_include(header)
#else
	#define TOML_HAS_INCLUDE(header)		0
#endif

#define TOML_COMPILER_EXCEPTIONS 0
#if TOML_COMPILER_EXCEPTIONS
	#if !defined(TOML_EXCEPTIONS) || (defined(TOML_EXCEPTIONS) && TOML_EXCEPTIONS)
		#undef TOML_EXCEPTIONS
		#define TOML_EXCEPTIONS 1
	#endif
#else
	#if defined(TOML_EXCEPTIONS) && TOML_EXCEPTIONS
		#error TOML_EXCEPTIONS was explicitly enabled but exceptions are disabled/unsupported by the compiler.
	#endif
	#undef TOML_EXCEPTIONS
	#define TOML_EXCEPTIONS	0
#endif

#if TOML_EXCEPTIONS
	#define TOML_MAY_THROW
#else
	#define TOML_MAY_THROW				noexcept
#endif

#ifndef TOML_INT_CHARCONV
	#define TOML_INT_CHARCONV 1
#endif
#ifndef TOML_FLOAT_CHARCONV
	#define TOML_FLOAT_CHARCONV 1
#endif
#if (TOML_INT_CHARCONV || TOML_FLOAT_CHARCONV) && !TOML_HAS_INCLUDE(<charconv>)
	#undef TOML_INT_CHARCONV
	#undef TOML_FLOAT_CHARCONV
	#define TOML_INT_CHARCONV 0
	#define TOML_FLOAT_CHARCONV 0
#endif

#ifndef TOML_PUSH_WARNINGS
	#define TOML_PUSH_WARNINGS
#endif
#ifndef TOML_DISABLE_SWITCH_WARNINGS
	#define	TOML_DISABLE_SWITCH_WARNINGS
#endif
#ifndef TOML_DISABLE_INIT_WARNINGS
	#define	TOML_DISABLE_INIT_WARNINGS
#endif
#ifndef TOML_DISABLE_SPAM_WARNINGS
	#define TOML_DISABLE_SPAM_WARNINGS
#endif
#ifndef TOML_DISABLE_ARITHMETIC_WARNINGS
	#define TOML_DISABLE_ARITHMETIC_WARNINGS
#endif
#ifndef TOML_DISABLE_SHADOW_WARNINGS
	#define TOML_DISABLE_SHADOW_WARNINGS
#endif
#ifndef TOML_POP_WARNINGS
	#define TOML_POP_WARNINGS
#endif
#ifndef TOML_DISABLE_WARNINGS
	#define TOML_DISABLE_WARNINGS
#endif
#ifndef TOML_ENABLE_WARNINGS
	#define TOML_ENABLE_WARNINGS
#endif

#ifndef TOML_ATTR
	#define TOML_ATTR(...)
#endif

#ifndef TOML_INTERFACE
	#define TOML_INTERFACE
#endif

#ifndef TOML_EMPTY_BASES
	#define TOML_EMPTY_BASES
#endif

#ifndef TOML_NEVER_INLINE
	#define TOML_NEVER_INLINE
#endif

#ifndef TOML_ASSUME
	#define TOML_ASSUME(cond)	(void)0
#endif

#ifndef TOML_UNREACHABLE
	#define TOML_UNREACHABLE	TOML_ASSERT(false)
#endif

#define TOML_NO_DEFAULT_CASE	default: TOML_UNREACHABLE

#ifdef __cpp_consteval
	#define TOML_CONSTEVAL		consteval
#else
	#define TOML_CONSTEVAL		constexpr
#endif

#ifdef __has_cpp_attribute
	#define TOML_HAS_ATTR(...)	__has_cpp_attribute(__VA_ARGS__)
#else
	#define TOML_HAS_ATTR(...)	0
#endif

#if !defined(DOXYGEN) && !TOML_INTELLISENSE
	#if !defined(TOML_LIKELY) && TOML_HAS_ATTR(likely)
		#define TOML_LIKELY(...)	(__VA_ARGS__) [[likely]]
	#endif
	#if !defined(TOML_UNLIKELY) && TOML_HAS_ATTR(unlikely)
		#define TOML_UNLIKELY(...)	(__VA_ARGS__) [[unlikely]]
	#endif
	#if TOML_HAS_ATTR(nodiscard) >= 201907L
		#define TOML_NODISCARD_CTOR [[nodiscard]]
	#endif
#endif

#ifndef TOML_LIKELY
	#define TOML_LIKELY(...)	(__VA_ARGS__)
#endif
#ifndef TOML_UNLIKELY
	#define TOML_UNLIKELY(...)	(__VA_ARGS__)
#endif
#ifndef TOML_NODISCARD_CTOR
	#define TOML_NODISCARD_CTOR
#endif

#ifndef TOML_TRIVIAL_ABI
	#define TOML_TRIVIAL_ABI
#endif

#ifndef TOML_RELOPS_REORDERING
	#define TOML_RELOPS_REORDERING 0
#endif
#if TOML_RELOPS_REORDERING
	#define TOML_ASYMMETRICAL_EQUALITY_OPS(...)
#else
	#define TOML_ASYMMETRICAL_EQUALITY_OPS(LHS, RHS, ...)														\
		__VA_ARGS__ [[nodiscard]] friend bool operator == (RHS rhs, LHS lhs) noexcept { return lhs == rhs; }	\
		__VA_ARGS__ [[nodiscard]] friend bool operator != (LHS lhs, RHS rhs) noexcept { return !(lhs == rhs); }	\
		__VA_ARGS__ [[nodiscard]] friend bool operator != (RHS rhs, LHS lhs) noexcept { return !(lhs == rhs); }
#endif

#ifndef TOML_SIMPLE_STATIC_ASSERT_MESSAGES
	#define TOML_SIMPLE_STATIC_ASSERT_MESSAGES	0
#endif

#define TOML_CONCAT_1(x, y) x##y
#define TOML_CONCAT(x, y) TOML_CONCAT_1(x, y)

#define TOML_EVAL_BOOL_1(T, F)	T
#define TOML_EVAL_BOOL_0(T, F)	F

#if defined(__aarch64__) || defined(__ARM_ARCH_ISA_A64) || defined(_M_ARM64) || defined(__ARM_64BIT_STATE)	\
		|| defined(__arm__) || defined(_M_ARM) || defined(__ARM_32BIT_STATE)
	#define TOML_ARM 1
#else
	#define TOML_ARM 0
#endif

#define TOML_MAKE_BITOPS(type)																		\
	[[nodiscard]]																					\
	TOML_ALWAYS_INLINE																				\
	TOML_ATTR(const)																				\
	TOML_ATTR(flatten)																				\
	constexpr type operator & (type lhs, type rhs) noexcept											\
	{																								\
		return static_cast<type>(::toml::impl::unwrap_enum(lhs) & ::toml::impl::unwrap_enum(rhs));	\
	}																								\
	[[nodiscard]]																					\
	TOML_ALWAYS_INLINE																				\
	TOML_ATTR(const)																				\
	TOML_ATTR(flatten)																				\
	constexpr type operator | (type lhs, type rhs) noexcept											\
	{																								\
		return static_cast<type>(::toml::impl::unwrap_enum(lhs) | ::toml::impl::unwrap_enum(rhs));	\
	}

#ifdef __FLT16_MANT_DIG__
	#if __FLT_RADIX__ == 2					\
			&& __FLT16_MANT_DIG__ == 11		\
			&& __FLT16_DIG__ == 3			\
			&& __FLT16_MIN_EXP__ == -13		\
			&& __FLT16_MIN_10_EXP__ == -4	\
			&& __FLT16_MAX_EXP__ == 16		\
			&& __FLT16_MAX_10_EXP__ == 4
		#if (TOML_ARM && TOML_GCC) || TOML_CLANG
			#define TOML_FP16 __fp16
		#endif
		#if TOML_ARM && TOML_CLANG // not present in g++
			#define TOML_FLOAT16 _Float16
		#endif
	#endif
#endif

#if defined(__SIZEOF_FLOAT128__)		\
	&& defined(__FLT128_MANT_DIG__)		\
	&& defined(__LDBL_MANT_DIG__)		\
	&& __FLT128_MANT_DIG__ > __LDBL_MANT_DIG__
	#define TOML_FLOAT128	__float128
#endif

#ifdef __SIZEOF_INT128__
	#define TOML_INT128		__int128_t
	#define TOML_UINT128	__uint128_t
#endif

#define TOML_LIB_MAJOR		2
#define TOML_LIB_MINOR		1
#define TOML_LIB_PATCH		0

#define TOML_LANG_MAJOR		1
#define TOML_LANG_MINOR		0
#define TOML_LANG_PATCH		0

#define	TOML_LIB_SINGLE_HEADER 1

#define TOML_MAKE_VERSION(maj, min, rev)											\
		((maj) * 1000 + (min) * 25 + (rev))

#if TOML_UNRELEASED_FEATURES
	#define TOML_LANG_EFFECTIVE_VERSION												\
		TOML_MAKE_VERSION(TOML_LANG_MAJOR, TOML_LANG_MINOR, TOML_LANG_PATCH+1)
#else
	#define TOML_LANG_EFFECTIVE_VERSION												\
		TOML_MAKE_VERSION(TOML_LANG_MAJOR, TOML_LANG_MINOR, TOML_LANG_PATCH)
#endif

#define TOML_LANG_HIGHER_THAN(maj, min, rev)										\
		(TOML_LANG_EFFECTIVE_VERSION > TOML_MAKE_VERSION(maj, min, rev))

#define TOML_LANG_AT_LEAST(maj, min, rev)											\
		(TOML_LANG_EFFECTIVE_VERSION >= TOML_MAKE_VERSION(maj, min, rev))

#define TOML_LANG_UNRELEASED														\
		TOML_LANG_HIGHER_THAN(TOML_LANG_MAJOR, TOML_LANG_MINOR, TOML_LANG_PATCH)

#ifndef TOML_ABI_NAMESPACES
	#ifdef DOXYGEN
		#define TOML_ABI_NAMESPACES 0
	#else
		#define TOML_ABI_NAMESPACES 1
	#endif
#endif
#if TOML_ABI_NAMESPACES
	#define TOML_NAMESPACE_START				namespace toml { inline namespace TOML_CONCAT(v, TOML_LIB_MAJOR)
	#define TOML_NAMESPACE_END					}
	#define TOML_NAMESPACE						::toml::TOML_CONCAT(v, TOML_LIB_MAJOR)
	#define TOML_ABI_NAMESPACE_START(name)		inline namespace name {
	#define TOML_ABI_NAMESPACE_BOOL(cond, T, F)	TOML_ABI_NAMESPACE_START(TOML_CONCAT(TOML_EVAL_BOOL_, cond)(T, F))
	#define TOML_ABI_NAMESPACE_END				}
#else
	#define TOML_NAMESPACE_START				namespace toml
	#define TOML_NAMESPACE_END
	#define TOML_NAMESPACE						toml
	#define TOML_ABI_NAMESPACE_START(...)
	#define TOML_ABI_NAMESPACE_BOOL(...)
	#define TOML_ABI_NAMESPACE_END
#endif
#define TOML_IMPL_NAMESPACE_START				TOML_NAMESPACE_START { namespace impl
#define TOML_IMPL_NAMESPACE_END					} TOML_NAMESPACE_END
#if TOML_HEADER_ONLY
	#define TOML_ANON_NAMESPACE_START			TOML_IMPL_NAMESPACE_START
	#define TOML_ANON_NAMESPACE_END				TOML_IMPL_NAMESPACE_END
	#define TOML_ANON_NAMESPACE					TOML_NAMESPACE::impl
	#define TOML_USING_ANON_NAMESPACE			using namespace TOML_ANON_NAMESPACE
	#define TOML_EXTERNAL_LINKAGE				inline
	#define TOML_INTERNAL_LINKAGE				inline
#else
	#define TOML_ANON_NAMESPACE_START			namespace
	#define TOML_ANON_NAMESPACE_END
	#define TOML_ANON_NAMESPACE
	#define TOML_USING_ANON_NAMESPACE			(void)0
	#define TOML_EXTERNAL_LINKAGE
	#define TOML_INTERNAL_LINKAGE				static
#endif

TOML_DISABLE_WARNINGS
#ifndef TOML_ASSERT
	#if defined(NDEBUG) || !defined(_DEBUG)
		#define TOML_ASSERT(expr)	(void)0
	#else
		#ifndef assert
			#include <cassert>
		#endif
		#define TOML_ASSERT(expr)	assert(expr)
	#endif
#endif
TOML_ENABLE_WARNINGS

#endif //------  ↑ toml_preprocessor.h  --------------------------------------------------------------------------------

TOML_PUSH_WARNINGS
TOML_DISABLE_SPAM_WARNINGS

#if 1  //----------------------------------  ↓ toml_common.h  ----------------------------------------------------------

TOML_DISABLE_WARNINGS
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cfloat>
#include <climits>
#include <limits>
#include <memory>
#include <string_view>
#include <string>
#include <vector>
#include <map>
#include <iosfwd>
#if !TOML_HAS_CUSTOM_OPTIONAL_TYPE
	#include <optional>
#endif
#if TOML_HAS_INCLUDE(<version>)
	#include <version>
#endif
TOML_ENABLE_WARNINGS

#ifdef __cpp_lib_launder
	#define TOML_LAUNDER(x)	std::launder(x)
#else
	#define TOML_LAUNDER(x)	x
#endif

#ifndef DOXYGEN
#ifndef TOML_DISABLE_ENVIRONMENT_CHECKS
#define TOML_ENV_MESSAGE																							\
	"If you're seeing this error it's because you're building toml++ for an environment that doesn't conform to "	\
	"one of the 'ground truths' assumed by the library. Essentially this just means that I don't have the "			\
	"resources to test on more platforms, but I wish I did! You can try disabling the checks by defining "			\
	"TOML_DISABLE_ENVIRONMENT_CHECKS, but your mileage may vary. Please consider filing an issue at "				\
	"https://github.com/marzer/tomlplusplus/issues to help me improve support for your target environment. Thanks!"

static_assert(CHAR_BIT == 8, TOML_ENV_MESSAGE);
static_assert(FLT_RADIX == 2, TOML_ENV_MESSAGE);
static_assert('A' == 65, TOML_ENV_MESSAGE);
static_assert(sizeof(double) == 8, TOML_ENV_MESSAGE);
static_assert(std::numeric_limits<double>::is_iec559, TOML_ENV_MESSAGE);
static_assert(std::numeric_limits<double>::digits == 53, TOML_ENV_MESSAGE);
static_assert(std::numeric_limits<double>::digits10 == 15, TOML_ENV_MESSAGE);

#undef TOML_ENV_MESSAGE
#endif // !TOML_DISABLE_ENVIRONMENT_CHECKS
#endif // !DOXYGEN

#ifndef DOXYGEN // undocumented forward declarations are hidden from doxygen because they fuck it up =/

namespace toml // non-abi namespace; this is not an error
{
	using namespace std::string_literals;
	using namespace std::string_view_literals;
	using ::std::size_t;
	using ::std::intptr_t;
	using ::std::uintptr_t;
	using ::std::ptrdiff_t;
	using ::std::nullptr_t;
	using ::std::int8_t;
	using ::std::int16_t;
	using ::std::int32_t;
	using ::std::int64_t;
	using ::std::uint8_t;
	using ::std::uint16_t;
	using ::std::uint32_t;
	using ::std::uint64_t;
	using ::std::uint_least32_t;
	using ::std::uint_least64_t;

	// legacy typedefs
	using string_char = char;
	using string = std::string;
	using string_view = std::string_view;
}

TOML_NAMESPACE_START // abi namespace
{
	struct date;
	struct time;
	struct time_offset;

	TOML_ABI_NAMESPACE_BOOL(TOML_HAS_CUSTOM_OPTIONAL_TYPE, custopt, stdopt)
	struct date_time;
	TOML_ABI_NAMESPACE_END

	class node;
	class array;
	class table;

	template <typename> class node_view;
	template <typename> class value;
	template <typename> class default_formatter;
	template <typename> class json_formatter;

	[[nodiscard]] TOML_API bool operator == (const array& lhs, const array& rhs) noexcept;
	[[nodiscard]] TOML_API bool operator != (const array& lhs, const array& rhs) noexcept;
	[[nodiscard]] TOML_API bool operator == (const table& lhs, const table& rhs) noexcept;
	[[nodiscard]] TOML_API bool operator != (const table& lhs, const table& rhs) noexcept;

	template <typename Char>
	std::basic_ostream<Char>& operator << (std::basic_ostream<Char>&, const array&);
	template <typename Char, typename T>
	std::basic_ostream<Char>& operator << (std::basic_ostream<Char>&, const value<T>&);
	template <typename Char>
	std::basic_ostream<Char>& operator << (std::basic_ostream<Char>&, const table&);
	template <typename T, typename U>
	std::basic_ostream<T>& operator << (std::basic_ostream<T>&, default_formatter<U>&);
	template <typename T, typename U>
	std::basic_ostream<T>& operator << (std::basic_ostream<T>&, default_formatter<U>&&);
	template <typename T, typename U>
	std::basic_ostream<T>& operator << (std::basic_ostream<T>&, json_formatter<U>&);
	template <typename T, typename U>
	std::basic_ostream<T>& operator << (std::basic_ostream<T>&, json_formatter<U>&&);
	template <typename Char, typename T>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>&, const node_view<T>&);

	namespace impl
	{
		template <typename T>
		using string_map = std::map<std::string, T, std::less<>>; // heterogeneous lookup

		template <typename T>
		using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

		template <typename T, typename... U>
		inline constexpr bool is_one_of = (false || ... || std::is_same_v<T, U>);

		template <typename T>
		inline constexpr bool is_cvref = std::is_reference_v<T> || std::is_const_v<T> || std::is_volatile_v<T>;

		template <typename T>
		inline constexpr bool is_wide_string = is_one_of<
			std::decay_t<T>,
			const wchar_t*,
			wchar_t*,
			std::wstring_view,
			std::wstring
		>;

		template <typename T>
		inline constexpr bool dependent_false = false;

		#if TOML_WINDOWS_COMPAT
		[[nodiscard]] TOML_API std::string narrow(std::wstring_view) noexcept;
		[[nodiscard]] TOML_API std::wstring widen(std::string_view) noexcept;
		#ifdef __cpp_lib_char8_t
		[[nodiscard]] TOML_API std::wstring widen(std::u8string_view) noexcept;
		#endif
		#endif // TOML_WINDOWS_COMPAT

		#if TOML_ABI_NAMESPACES
			#if TOML_EXCEPTIONS
				TOML_ABI_NAMESPACE_START(ex)
				#define TOML_PARSER_TYPENAME TOML_NAMESPACE::impl::ex::parser
			#else
				TOML_ABI_NAMESPACE_START(noex)
				#define TOML_PARSER_TYPENAME TOML_NAMESPACE::impl::noex::parser
			#endif
		#else
			#define TOML_PARSER_TYPENAME TOML_NAMESPACE::impl::parser
		#endif
		class parser;
		TOML_ABI_NAMESPACE_END // TOML_EXCEPTIONS
	}
}
TOML_NAMESPACE_END

#endif // !DOXYGEN

namespace toml { }

TOML_NAMESPACE_START // abi namespace
{
	inline namespace literals {}

	#if TOML_HAS_CUSTOM_OPTIONAL_TYPE
	template <typename T>
	using optional = TOML_OPTIONAL_TYPE<T>;
	#else
	template <typename T>
	using optional = std::optional<T>;
	#endif

	enum class node_type : uint8_t
	{
		none,
		table,
		array,
		string,
		integer,
		floating_point,
		boolean,
		date,
		time,
		date_time
	};

	using source_path_ptr = std::shared_ptr<const std::string>;

	template <typename T>
	struct TOML_TRIVIAL_ABI inserter
	{
		T&& value;
	};
	template <typename T> inserter(T&&) -> inserter<T>;
}
TOML_NAMESPACE_END

TOML_IMPL_NAMESPACE_START
{
	// general value traits
	// (as they relate to their equivalent native TOML type)
	template <typename T>
	struct value_traits
	{
		using native_type = void;
		static constexpr bool is_native = false;
		static constexpr bool is_losslessly_convertible_to_native = false;
		static constexpr bool can_represent_native = false;
		static constexpr bool can_partially_represent_native = false;
		static constexpr auto type = node_type::none;
	};
	template <typename T> struct value_traits<T&> : value_traits<T> {};
	template <typename T> struct value_traits<T&&> : value_traits<T> {};
	template <typename T> struct value_traits<T* const> : value_traits<T*> {};
	template <typename T> struct value_traits<T* volatile> : value_traits<T*> {};
	template <typename T> struct value_traits<T* const volatile> : value_traits<T*> {};

	// integer value traits
	template <typename T>
	struct integer_value_limits
	{
		static constexpr auto min = (std::numeric_limits<T>::min)();
		static constexpr auto max = (std::numeric_limits<T>::max)();
	};
	template <typename T>
	struct integer_value_traits_base : integer_value_limits<T>
	{
		using native_type = int64_t;
		static constexpr bool is_native = std::is_same_v<T, native_type>;
		static constexpr bool is_signed = static_cast<T>(-1) < T{}; // for impls not specializing std::is_signed<T>
		static constexpr auto type = node_type::integer;
		static constexpr bool can_partially_represent_native = true;
	};
	template <typename T>
	struct unsigned_integer_value_traits : integer_value_traits_base<T>
	{
		static constexpr bool is_losslessly_convertible_to_native
			= integer_value_limits<T>::max <= 9223372036854775807ULL;
		static constexpr bool can_represent_native = false;

	};
	template <typename T>
	struct signed_integer_value_traits : integer_value_traits_base<T>
	{
		using native_type = int64_t;
		static constexpr bool is_losslessly_convertible_to_native
			 = integer_value_limits<T>::min >= (-9223372036854775807LL - 1LL)
			&& integer_value_limits<T>::max <= 9223372036854775807LL;
		static constexpr bool can_represent_native
			 = integer_value_limits<T>::min <= (-9223372036854775807LL - 1LL)
			&& integer_value_limits<T>::max >= 9223372036854775807LL;
	};
	template <typename T, bool S = integer_value_traits_base<T>::is_signed>
	struct integer_value_traits : signed_integer_value_traits<T> {};
	template <typename T>
	struct integer_value_traits<T, false> : unsigned_integer_value_traits<T> {};
	template <> struct value_traits<signed char>		: integer_value_traits<signed char> {};
	template <> struct value_traits<unsigned char>		: integer_value_traits<unsigned char> {};
	template <> struct value_traits<signed short>		: integer_value_traits<signed short> {};
	template <> struct value_traits<unsigned short>		: integer_value_traits<unsigned short> {};
	template <> struct value_traits<signed int>			: integer_value_traits<signed int> {};
	template <> struct value_traits<unsigned int>		: integer_value_traits<unsigned int> {};
	template <> struct value_traits<signed long>		: integer_value_traits<signed long> {};
	template <> struct value_traits<unsigned long>		: integer_value_traits<unsigned long> {};
	template <> struct value_traits<signed long long>	: integer_value_traits<signed long long> {};
	template <> struct value_traits<unsigned long long>	: integer_value_traits<unsigned long long> {};
	#ifdef TOML_INT128
	template <>
	struct integer_value_limits<TOML_INT128>
	{
		static constexpr TOML_INT128 max = static_cast<TOML_INT128>(
			(TOML_UINT128{ 1u } << ((__SIZEOF_INT128__ * CHAR_BIT) - 1)) - 1
		);
		static constexpr TOML_INT128 min = -max - TOML_INT128{ 1 };
	};
	template <>
	struct integer_value_limits<TOML_UINT128>
	{
		static constexpr TOML_UINT128 min = TOML_UINT128{};
		static constexpr TOML_UINT128 max = (2u * static_cast<TOML_UINT128>(integer_value_limits<TOML_INT128>::max)) + 1u;
	};
	template <> struct value_traits<TOML_INT128> : integer_value_traits<TOML_INT128> {};
	template <> struct value_traits<TOML_UINT128> : integer_value_traits<TOML_UINT128> {};
	#endif
	#ifdef TOML_SMALL_INT_TYPE
	template <> struct value_traits<TOML_SMALL_INT_TYPE> : signed_integer_value_traits<TOML_SMALL_INT_TYPE> {};
	#endif
	static_assert(value_traits<int64_t>::is_native);
	static_assert(value_traits<int64_t>::is_signed);
	static_assert(value_traits<int64_t>::is_losslessly_convertible_to_native);
	static_assert(value_traits<int64_t>::can_represent_native);
	static_assert(value_traits<int64_t>::can_partially_represent_native);

	// float value traits
	template <typename T>
	struct float_value_limits
	{
		static constexpr bool is_iec559 = std::numeric_limits<T>::is_iec559;
		static constexpr int digits = std::numeric_limits<T>::digits;
		static constexpr int digits10 = std::numeric_limits<T>::digits10;
	};
	template <typename T>
	struct float_value_traits : float_value_limits<T>
	{
		using native_type = double;
		static constexpr bool is_native = std::is_same_v<T, native_type>;
		static constexpr bool is_signed = true;
		static constexpr bool is_losslessly_convertible_to_native
			= float_value_limits<T>::is_iec559
			&& float_value_limits<T>::digits <= 53
			&& float_value_limits<T>::digits10 <= 15;
		static constexpr bool can_represent_native
			= float_value_limits<T>::is_iec559
			&& float_value_limits<T>::digits >= 53 // DBL_MANT_DIG
			&& float_value_limits<T>::digits10 >= 15; // DBL_DIG
		static constexpr bool can_partially_represent_native //32-bit float values
			= float_value_limits<T>::is_iec559
			&& float_value_limits<T>::digits >= 24
			&& float_value_limits<T>::digits10 >= 6;
		static constexpr auto type = node_type::floating_point;
	};
	template <> struct value_traits<float> : float_value_traits<float> {};
	template <> struct value_traits<double> : float_value_traits<double> {};
	template <> struct value_traits<long double> : float_value_traits<long double> {};
	template <int mant_dig, int dig>
	struct extended_float_value_limits
	{
		static constexpr bool is_iec559 = true;
		static constexpr int digits = mant_dig;
		static constexpr int digits10 = dig;
	};
	#ifdef TOML_FP16
	template <> struct float_value_limits<TOML_FP16> : extended_float_value_limits<__FLT16_MANT_DIG__, __FLT16_DIG__> {};
	template <> struct value_traits<TOML_FP16> : float_value_traits<TOML_FP16> {};
	#endif
	#ifdef TOML_FLOAT16
	template <> struct float_value_limits<TOML_FLOAT16> : extended_float_value_limits<__FLT16_MANT_DIG__, __FLT16_DIG__> {};
	template <> struct value_traits<TOML_FLOAT16> : float_value_traits<TOML_FLOAT16> {};
	#endif
	#ifdef TOML_FLOAT128
	template <> struct float_value_limits<TOML_FLOAT128> : extended_float_value_limits<__FLT128_MANT_DIG__, __FLT128_DIG__> {};
	template <> struct value_traits<TOML_FLOAT128> : float_value_traits<TOML_FLOAT128> {};
	#endif
	#ifdef TOML_SMALL_FLOAT_TYPE
	template <> struct value_traits<TOML_SMALL_FLOAT_TYPE> : float_value_traits<TOML_SMALL_FLOAT_TYPE> {};
	#endif
	static_assert(value_traits<double>::is_native);
	static_assert(value_traits<double>::is_losslessly_convertible_to_native);
	static_assert(value_traits<double>::can_represent_native);
	static_assert(value_traits<double>::can_partially_represent_native);

	// string value traits
	template <typename T>
	struct string_value_traits
	{
		using native_type = std::string;
		static constexpr bool is_native = std::is_same_v<T, native_type>;
		static constexpr bool is_losslessly_convertible_to_native = true;
		static constexpr bool can_represent_native
			= !std::is_array_v<T>
			&& (!std::is_pointer_v<T> || std::is_const_v<std::remove_pointer_t<T>>);
		static constexpr bool can_partially_represent_native = can_represent_native;
		static constexpr auto type = node_type::string;
	};
	template <>         struct value_traits<std::string>		: string_value_traits<std::string> {};
	template <>         struct value_traits<std::string_view>	: string_value_traits<std::string_view> {};
	template <>         struct value_traits<const char*>		: string_value_traits<const char *> {};
	template <size_t N> struct value_traits<const char[N]>		: string_value_traits<const char[N]> {};
	template <>         struct value_traits<char*>				: string_value_traits<char*> {};
	template <size_t N> struct value_traits<char[N]>			: string_value_traits<char[N]> {};
	#ifdef __cpp_lib_char8_t
	template <>         struct value_traits<std::u8string>		: string_value_traits<std::u8string> {};
	template <>         struct value_traits<std::u8string_view>	: string_value_traits<std::u8string_view> {};
	template <>         struct value_traits<const char8_t*>		: string_value_traits<const char8_t*> {};
	template <size_t N> struct value_traits<const char8_t[N]>	: string_value_traits<const char8_t[N]> {};
	template <>         struct value_traits<char8_t*>			: string_value_traits<char8_t*> {};
	template <size_t N> struct value_traits<char8_t[N]>			: string_value_traits<char8_t[N]> {};
	#endif
	#if TOML_WINDOWS_COMPAT
	template <typename T>
	struct wstring_value_traits
	{
		using native_type = std::string;
		static constexpr bool is_native = false;
		static constexpr bool is_losslessly_convertible_to_native = true; //narrow
		static constexpr bool can_represent_native = std::is_same_v<T, std::wstring>; //widen
		static constexpr bool can_partially_represent_native = can_represent_native;
		static constexpr auto type = node_type::string;
	};
	template <>         struct value_traits<std::wstring>		: wstring_value_traits<std::wstring> {};
	template <>         struct value_traits<std::wstring_view>	: wstring_value_traits<std::wstring_view> {};
	template <>         struct value_traits<const wchar_t*>		: wstring_value_traits<const wchar_t*> {};
	template <size_t N> struct value_traits<const wchar_t[N]>	: wstring_value_traits<const wchar_t[N]> {};
	template <>         struct value_traits<wchar_t*>			: wstring_value_traits<wchar_t*> {};
	template <size_t N> struct value_traits<wchar_t[N]>			: wstring_value_traits<wchar_t[N]> {};
	#endif

	// other native value traits
	template <typename T, node_type NodeType>
	struct native_value_traits
	{
		using native_type = T;
		static constexpr bool is_native = true;
		static constexpr bool is_losslessly_convertible_to_native = true;
		static constexpr bool can_represent_native = true;
		static constexpr bool can_partially_represent_native = true;
		static constexpr auto type = NodeType;
	};
	template <> struct value_traits<bool>		: native_value_traits<bool,		 node_type::boolean> {};
	template <> struct value_traits<date>		: native_value_traits<date,		 node_type::date> {};
	template <> struct value_traits<time>		: native_value_traits<time,		 node_type::time> {};
	template <> struct value_traits<date_time>	: native_value_traits<date_time, node_type::date_time> {};

	// native value category queries
	template <typename T>
	using native_type_of = typename value_traits<T>::native_type;
	template <typename T>
	inline constexpr bool is_native = value_traits<T>::is_native;
	template <typename T>
	inline constexpr bool can_represent_native = value_traits<T>::can_represent_native;
	template <typename T>
	inline constexpr bool can_partially_represent_native = value_traits<T>::can_partially_represent_native;
	template <typename T>
	inline constexpr bool is_losslessly_convertible_to_native = value_traits<T>::is_losslessly_convertible_to_native;
	template <typename T, typename... U>
	inline constexpr bool is_natively_one_of = is_one_of<native_type_of<T>, U...>;

	// native <=> node conversions
	template <typename T> struct node_wrapper { using type = T; };
	template <>           struct node_wrapper<std::string> { using type = value<std::string>; };
	template <>           struct node_wrapper<int64_t> { using type = value<int64_t>; };
	template <>           struct node_wrapper<double> { using type = value<double>; };
	template <>           struct node_wrapper<bool> { using type = value<bool>; };
	template <>           struct node_wrapper<date> { using type = value<date>; };
	template <>           struct node_wrapper<time> { using type = value<time>; };
	template <>           struct node_wrapper<date_time> { using type = value<date_time>; };
	template <typename T> using wrap_node = typename node_wrapper<T>::type;

	template <typename T> struct node_unwrapper { using type = T; };
	template <typename T> struct node_unwrapper<value<T>> { using type = T; };
	template <typename T> using unwrap_node = typename node_unwrapper<T>::type;

	template <typename T> struct node_type_getter { static constexpr auto value = value_traits<T>::type; };
	template <>           struct node_type_getter<table> { static constexpr auto value = node_type::table; };
	template <>           struct node_type_getter<array> { static constexpr auto value = node_type::array; };
	template <>           struct node_type_getter<void>  { static constexpr auto value = node_type::none; };
	template <typename T>
	inline constexpr node_type node_type_of = node_type_getter<unwrap_node<remove_cvref_t<T>>>::value;

	template <typename T>
	inline constexpr bool is_node_view = is_one_of<impl::remove_cvref_t<T>, node_view<node>, node_view<const node>>;
}
TOML_IMPL_NAMESPACE_END

TOML_NAMESPACE_START
{
	template <typename T>
	inline constexpr bool is_table = std::is_same_v<impl::remove_cvref_t<T>, table>;

	template <typename T>
	inline constexpr bool is_array = std::is_same_v<impl::remove_cvref_t<T>, array>;

	template <typename T>
	inline constexpr bool is_string = std::is_same_v<impl::wrap_node<impl::remove_cvref_t<T>>, value<std::string>>;

	template <typename T>
	inline constexpr bool is_integer = std::is_same_v<impl::wrap_node<impl::remove_cvref_t<T>>, value<int64_t>>;

	template <typename T>
	inline constexpr bool is_floating_point = std::is_same_v<impl::wrap_node<impl::remove_cvref_t<T>>, value<double>>;

	template <typename T>
	inline constexpr bool is_number = is_integer<T> || is_floating_point<T>;

	template <typename T>
	inline constexpr bool is_boolean = std::is_same_v<impl::wrap_node<impl::remove_cvref_t<T>>, value<bool>>;

	template <typename T>
	inline constexpr bool is_date = std::is_same_v<impl::wrap_node<impl::remove_cvref_t<T>>, value<date>>;

	template <typename T>
	inline constexpr bool is_time = std::is_same_v<impl::wrap_node<impl::remove_cvref_t<T>>, value<time>>;

	template <typename T>
	inline constexpr bool is_date_time = std::is_same_v<impl::wrap_node<impl::remove_cvref_t<T>>, value<date_time>>;
}
TOML_NAMESPACE_END

TOML_IMPL_NAMESPACE_START
{
	template <typename T>
	[[nodiscard]]
	TOML_ATTR(const)
	TOML_ALWAYS_INLINE
	constexpr std::underlying_type_t<T> unwrap_enum(T val) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(val);
	}

	// Q: "why not use the built-in fpclassify?"
	// A: Because it gets broken by -ffast-math and friends
	enum class fp_class : unsigned { ok, neg_inf, pos_inf, nan };
	[[nodiscard]]
	TOML_ATTR(pure)
	inline fp_class fpclassify(const double& val) noexcept
	{
		constexpr uint64_t sign     = 0b1000000000000000000000000000000000000000000000000000000000000000ull;
		constexpr uint64_t exponent = 0b0111111111110000000000000000000000000000000000000000000000000000ull;
		constexpr uint64_t mantissa = 0b0000000000001111111111111111111111111111111111111111111111111111ull;

		uint64_t val_bits;
		memcpy(&val_bits, &val, sizeof(val));
		if ((val_bits & exponent) != exponent)
			return fp_class::ok;
		if ((val_bits & mantissa))
			return fp_class::nan;
		return (val_bits & sign) ? fp_class::neg_inf : fp_class::pos_inf;
	}

	// Q: "why not use std::find??"
	// A: Because <algorithm> is _huge_ and std::find would be the only thing I used from it.
	//    I don't want to impose such a heavy compile-time burden on users.
	template <typename T>
	[[nodiscard]]
	inline const T* find(const std::vector<T>& haystack, const T& needle) noexcept
	{
		for (size_t i = 0, e = haystack.size(); i < e; i++)
			if (haystack[i] == needle)
				return haystack.data() + i;
		return nullptr;
	}

	inline constexpr std::string_view low_character_escape_table[] =
	{
		"\\u0000"sv,
		"\\u0001"sv,
		"\\u0002"sv,
		"\\u0003"sv,
		"\\u0004"sv,
		"\\u0005"sv,
		"\\u0006"sv,
		"\\u0007"sv,
		"\\b"sv,
		"\\t"sv,
		"\\n"sv,
		"\\u000B"sv,
		"\\f"sv,
		"\\r"sv,
		"\\u000E"sv,
		"\\u000F"sv,
		"\\u0010"sv,
		"\\u0011"sv,
		"\\u0012"sv,
		"\\u0013"sv,
		"\\u0014"sv,
		"\\u0015"sv,
		"\\u0016"sv,
		"\\u0017"sv,
		"\\u0018"sv,
		"\\u0019"sv,
		"\\u001A"sv,
		"\\u001B"sv,
		"\\u001C"sv,
		"\\u001D"sv,
		"\\u001E"sv,
		"\\u001F"sv,
	};

	inline constexpr std::string_view node_type_friendly_names[] =
	{
		"none"sv,
		"table"sv,
		"array"sv,
		"string"sv,
		"integer"sv,
		"floating-point"sv,
		"boolean"sv,
		"date"sv,
		"time"sv,
		"date-time"sv
	};
}
TOML_IMPL_NAMESPACE_END

TOML_NAMESPACE_START
{
	[[nodiscard]]
	TOML_ATTR(const)
	TOML_ALWAYS_INLINE
	TOML_CONSTEVAL size_t operator"" _sz(unsigned long long n) noexcept
	{
		return static_cast<size_t>(n);
	}

	TOML_ABI_NAMESPACE_BOOL(TOML_LARGE_FILES, lf, sf)

	#if TOML_LARGE_FILES
	using source_index = uint32_t;
	#else
	using source_index = uint16_t;
	#endif

	struct TOML_TRIVIAL_ABI source_position
	{
		source_index line;
		source_index column;

		[[nodiscard]]
		explicit constexpr operator bool () const noexcept
		{
			return line > source_index{} && column > source_index{};
		}

		[[nodiscard]]
		friend constexpr bool operator == (const source_position& lhs, const source_position& rhs) noexcept
		{
			return lhs.line == rhs.line
				&& lhs.column == rhs.column;
		}

		[[nodiscard]]
		friend constexpr bool operator != (const source_position& lhs, const source_position& rhs) noexcept
		{
			return lhs.line != rhs.line
				|| lhs.column != rhs.column;
		}

		[[nodiscard]]
		friend constexpr bool operator < (const source_position& lhs, const source_position& rhs) noexcept
		{
			return lhs.line < rhs.line
				|| (lhs.line == rhs.line && lhs.column < rhs.column);
		}

		[[nodiscard]]
		friend constexpr bool operator <= (const source_position& lhs, const source_position& rhs) noexcept
		{
			return lhs.line < rhs.line
				|| (lhs.line == rhs.line && lhs.column <= rhs.column);
		}
	};

	struct source_region
	{
		source_position begin;
		source_position end;
		source_path_ptr path;

		#if TOML_WINDOWS_COMPAT

		[[nodiscard]]
		optional<std::wstring> wide_path() const noexcept
		{
			if (!path || path->empty())
				return {};
			return { impl::widen(*path) };
		}

		#endif
	};

	TOML_ABI_NAMESPACE_END // TOML_LARGE_FILES

	enum class value_flags : uint8_t
	{
		none,

		format_as_binary = 1,

		format_as_octal = 2,

		format_as_hexadecimal = 3,
	};
	TOML_MAKE_BITOPS(value_flags)

	enum class format_flags : uint8_t
	{
		none,

		quote_dates_and_times = 1,

		allow_literal_strings = 2,

		allow_multi_line_strings = 4,

		allow_value_format_flags = 8,
	};
	TOML_MAKE_BITOPS(format_flags)

	template <typename Char>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, node_type rhs)
	{
		using underlying_t = std::underlying_type_t<node_type>;
		const auto str = impl::node_type_friendly_names[static_cast<underlying_t>(rhs)];
		if constexpr (std::is_same_v<Char, char>)
			return lhs << str;
		else
		{
			if constexpr (sizeof(Char) == 1)
				return lhs << std::basic_string_view<Char>{ reinterpret_cast<const Char*>(str.data()), str.length() };
			else
				return lhs << str.data();
		}
	}

	#ifndef DOXYGEN

	namespace impl
	{
		#define TOML_P2S_DECL(Type)															\
			template <typename Char>														\
			inline void print_to_stream(Type, std::basic_ostream<Char>&, value_flags = {})
		TOML_P2S_DECL(int8_t);
		TOML_P2S_DECL(int16_t);
		TOML_P2S_DECL(int32_t);
		TOML_P2S_DECL(int64_t);
		TOML_P2S_DECL(uint8_t);
		TOML_P2S_DECL(uint16_t);
		TOML_P2S_DECL(uint32_t);
		TOML_P2S_DECL(uint64_t);
		#undef TOML_P2S_DECL

		#define TOML_P2S_DECL(Type)											\
			template <typename Char>										\
			inline void print_to_stream(Type, std::basic_ostream<Char>&)
		TOML_P2S_DECL(double);
		TOML_P2S_DECL(const date&);
		TOML_P2S_DECL(const time&);
		TOML_P2S_DECL(time_offset);
		TOML_P2S_DECL(const date_time&);
		#undef TOML_P2S_DECL
	}

	#if !TOML_HEADER_ONLY
		extern template TOML_API std::ostream& operator << (std::ostream&, node_type);
	#endif // !TOML_HEADER_ONLY

	#endif // !DOXYGEN
}
TOML_NAMESPACE_END

#endif //----------------------------------  ↑ toml_common.h  ----------------------------------------------------------

#if 1  //---------------------------------------------------------  ↓ toml_date_time.h  --------------------------------

TOML_NAMESPACE_START
{
	struct TOML_TRIVIAL_ABI date
	{
		uint16_t year;
		uint8_t month;
		uint8_t day;

		[[nodiscard]]
		friend constexpr bool operator == (date lhs, date rhs) noexcept
		{
			return lhs.year == rhs.year
				&& lhs.month == rhs.month
				&& lhs.day == rhs.day;
		}

		[[nodiscard]]
		friend constexpr bool operator != (date lhs, date rhs) noexcept
		{
			return lhs.year != rhs.year
				|| lhs.month != rhs.month
				|| lhs.day != rhs.day;
		}

	private:

		[[nodiscard]] TOML_ALWAYS_INLINE
		static constexpr uint32_t pack(date d) noexcept
		{
			return static_cast<uint32_t>(d.year) << 16
				| static_cast<uint32_t>(d.month) << 8
				| static_cast<uint32_t>(d.day);
		}

	public:

		[[nodiscard]]
		friend constexpr bool operator <  (date lhs, date rhs) noexcept
		{
			return pack(lhs) < pack(rhs);
		}

		[[nodiscard]]
		friend constexpr bool operator <= (date lhs, date rhs) noexcept
		{
			return pack(lhs) <= pack(rhs);
		}

		[[nodiscard]]
		friend constexpr bool operator >  (date lhs, date rhs) noexcept
		{
			return pack(lhs) > pack(rhs);
		}

		[[nodiscard]]
		friend constexpr bool operator >= (date lhs, date rhs) noexcept
		{
			return pack(lhs) >= pack(rhs);
		}
	};

	template <typename Char>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const date& rhs)
	{
		impl::print_to_stream(rhs, lhs);
		return lhs;
	}

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template TOML_API std::ostream& operator << (std::ostream&, const date&);
	#endif

	struct TOML_TRIVIAL_ABI time
	{
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		uint32_t nanosecond;

		[[nodiscard]]
		friend constexpr bool operator == (const time& lhs, const time& rhs) noexcept
		{
			return lhs.hour == rhs.hour
				&& lhs.minute == rhs.minute
				&& lhs.second == rhs.second
				&& lhs.nanosecond == rhs.nanosecond;
		}

		[[nodiscard]]
		friend constexpr bool operator != (const time& lhs, const time& rhs) noexcept
		{
			return !(lhs == rhs);
		}

	private:

		[[nodiscard]]
		TOML_ALWAYS_INLINE
		static constexpr uint64_t pack(time t) noexcept
		{
			return static_cast<uint64_t>(t.hour) << 48
				| static_cast<uint64_t>(t.minute) << 40
				| static_cast<uint64_t>(t.second) << 32
				| static_cast<uint64_t>(t.nanosecond);
		}

	public:

		[[nodiscard]]
		friend constexpr bool operator <  (time lhs, time rhs) noexcept
		{
			return pack(lhs) < pack(rhs);
		}

		[[nodiscard]]
		friend constexpr bool operator <= (time lhs, time rhs) noexcept
		{
			return pack(lhs) <= pack(rhs);
		}

		[[nodiscard]]
		friend constexpr bool operator >  (time lhs, time rhs) noexcept
		{
			return pack(lhs) > pack(rhs);
		}

		[[nodiscard]]
		friend constexpr bool operator >= (time lhs, time rhs) noexcept
		{
			return pack(lhs) >= pack(rhs);
		}
	};

	template <typename Char>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const time& rhs)
	{
		impl::print_to_stream(rhs, lhs);
		return lhs;
	}

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template TOML_API std::ostream& operator << (std::ostream&, const time&);
	#endif

	struct TOML_TRIVIAL_ABI time_offset
	{
		int16_t minutes;

		TOML_NODISCARD_CTOR
		constexpr time_offset() noexcept
			: minutes{}
		{}

		TOML_NODISCARD_CTOR
		constexpr time_offset(int8_t h, int8_t m) noexcept
			: minutes{ static_cast<int16_t>(h * 60 + m) }
		{}

		[[nodiscard]]
		friend constexpr bool operator == (time_offset lhs, time_offset rhs) noexcept
		{
			return lhs.minutes == rhs.minutes;
		}

		[[nodiscard]]
		friend constexpr bool operator != (time_offset lhs, time_offset rhs) noexcept
		{
			return lhs.minutes != rhs.minutes;
		}

		[[nodiscard]]
		friend constexpr bool operator <  (time_offset lhs, time_offset rhs) noexcept
		{
			return lhs.minutes < rhs.minutes;
		}

		[[nodiscard]]
		friend constexpr bool operator <= (time_offset lhs, time_offset rhs) noexcept
		{
			return lhs.minutes <= rhs.minutes;
		}

		[[nodiscard]]
		friend constexpr bool operator >  (time_offset lhs, time_offset rhs) noexcept
		{
			return lhs.minutes > rhs.minutes;
		}

		[[nodiscard]]
		friend constexpr bool operator >= (time_offset lhs, time_offset rhs) noexcept
		{
			return lhs.minutes >= rhs.minutes;
		}
	};

	template <typename Char>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const time_offset& rhs)
	{
		impl::print_to_stream(rhs, lhs);
		return lhs;
	}

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template TOML_API std::ostream& operator << (std::ostream&, const time_offset&);
	#endif

	TOML_ABI_NAMESPACE_BOOL(TOML_HAS_CUSTOM_OPTIONAL_TYPE, custopt, stdopt)

	struct date_time
	{
		toml::date date;
		toml::time time;
		optional<toml::time_offset> offset;

		TOML_NODISCARD_CTOR
		constexpr date_time() noexcept
			: date{},
			time{}
		{}

		TOML_NODISCARD_CTOR
		constexpr date_time(toml::date d, toml::time t) noexcept
			: date{ d },
			time{ t }
		{}

		TOML_NODISCARD_CTOR
			constexpr date_time(toml::date d, toml::time t, toml::time_offset off) noexcept
			: date{ d },
			time{ t },
			offset{ off }
		{}

		[[nodiscard]]
		constexpr bool is_local() const noexcept
		{
			return !offset.has_value();
		}

		[[nodiscard]]
		friend constexpr bool operator == (const date_time& lhs, const date_time& rhs) noexcept
		{
			return lhs.date == rhs.date
				&& lhs.time == rhs.time
				&& lhs.offset == rhs.offset;
		}

		[[nodiscard]]
		friend constexpr bool operator != (const date_time& lhs, const date_time& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		[[nodiscard]]
		friend constexpr bool operator <  (const date_time& lhs, const date_time& rhs) noexcept
		{
			if (lhs.date != rhs.date)
				return lhs.date < rhs.date;
			if (lhs.time != rhs.time)
				return lhs.time < rhs.time;
			return lhs.offset < rhs.offset;
		}

		[[nodiscard]]
		friend constexpr bool operator <= (const date_time& lhs, const date_time& rhs) noexcept
		{
			if (lhs.date != rhs.date)
				return lhs.date < rhs.date;
			if (lhs.time != rhs.time)
				return lhs.time < rhs.time;
			return lhs.offset <= rhs.offset;
		}

		[[nodiscard]]
		friend constexpr bool operator >  (const date_time& lhs, const date_time& rhs) noexcept
		{
			return !(lhs <= rhs);
		}

		[[nodiscard]]
		friend constexpr bool operator >= (const date_time& lhs, const date_time& rhs) noexcept
		{
			return !(lhs < rhs);
		}
	};

	TOML_ABI_NAMESPACE_END // TOML_HAS_CUSTOM_OPTIONAL_TYPE

	template <typename Char>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const date_time& rhs)
	{
		impl::print_to_stream(rhs, lhs);
		return lhs;
	}

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template TOML_API std::ostream& operator << (std::ostream&, const date_time&);
	#endif
}
TOML_NAMESPACE_END

#endif //---------------------------------------------------------  ↑ toml_date_time.h  --------------------------------

#if 1  //-------------------------------------------------------------------------------  ↓ toml_print_to_stream.h  ----

TOML_DISABLE_WARNINGS
#include <cmath>
#if TOML_INT_CHARCONV || TOML_FLOAT_CHARCONV
	#include <charconv>
#endif
#if !TOML_INT_CHARCONV || !TOML_FLOAT_CHARCONV
	#include <sstream>
#endif
#if !TOML_INT_CHARCONV
	#include <iomanip>
#endif
TOML_ENABLE_WARNINGS

TOML_PUSH_WARNINGS
TOML_DISABLE_SWITCH_WARNINGS

TOML_IMPL_NAMESPACE_START
{
	// Q: "why does print_to_stream() exist? why not just use ostream::write(), ostream::put() etc?"
	// A: - I'm supporting C++20's char8_t as well; wrapping streams allows switching string modes transparently.
	//    - I'm using <charconv> to format numerics. Faster and locale-independent.
	//    - I can avoid forcing users to drag in <sstream> and <iomanip>.

	// Q: "there's a bit of reinterpret_casting here, is any of it UB?"
	// A: - If the source string data is char and the output string is char8_t, then technically yes,
	//      but not in the other direction. I test in both modes on Clang, GCC and MSVC and have yet to
	//      see it actually causing an issue, but in the event it does present a problem it's not going to
	//      be a show-stopper since all it means is I need to do duplicate some code.
	//    - Strings in C++. Honestly.

	template <typename Char1, typename Char2>
	inline void print_to_stream(std::basic_string_view<Char1> str, std::basic_ostream<Char2>& stream)
	{
		static_assert(sizeof(Char1) == 1);
		static_assert(sizeof(Char2) == 1);
		stream.write(reinterpret_cast<const Char2*>(str.data()), static_cast<std::streamsize>(str.length()));
	}

	template <typename Char1, typename Char2>
	inline void print_to_stream(const std::basic_string<Char1>& str, std::basic_ostream<Char2>& stream)
	{
		static_assert(sizeof(Char1) == 1);
		static_assert(sizeof(Char2) == 1);
		stream.write(reinterpret_cast<const Char2*>(str.data()), static_cast<std::streamsize>(str.length()));
	}

	template <typename Char>
	inline void print_to_stream(char character, std::basic_ostream<Char>& stream)
	{
		static_assert(sizeof(Char) == 1);
		stream.put(static_cast<Char>(character));
	}

	template <typename Char>
	TOML_ATTR(nonnull)
	inline void print_to_stream(const char* str, size_t len, std::basic_ostream<Char>& stream)
	{
		static_assert(sizeof(Char) == 1);
		stream.write(reinterpret_cast<const Char*>(str), static_cast<std::streamsize>(len));
	}

	#ifdef __cpp_lib_char8_t

	template <typename Char>
	inline void print_to_stream(char8_t character, std::basic_ostream<Char>& stream)
	{
		static_assert(sizeof(Char) == 1);
		stream.put(static_cast<Char>(character));
	}

	template <typename Char>
	TOML_ATTR(nonnull)
	inline void print_to_stream(const char8_t* str, size_t len, std::basic_ostream<Char>& stream)
	{
		static_assert(sizeof(Char) == 1);
		stream.write(reinterpret_cast<const Char*>(str), static_cast<std::streamsize>(len));
	}

	#endif

	template <typename T> inline constexpr size_t charconv_buffer_length = 0;
	template <> inline constexpr size_t charconv_buffer_length<double> = 60;
	template <> inline constexpr size_t charconv_buffer_length<float> = 40;
	template <> inline constexpr size_t charconv_buffer_length<uint64_t> = 20; // strlen("18446744073709551615")
	template <> inline constexpr size_t charconv_buffer_length<int64_t> = 20;  // strlen("-9223372036854775808")
	template <> inline constexpr size_t charconv_buffer_length<int32_t> = 11;  // strlen("-2147483648")
	template <> inline constexpr size_t charconv_buffer_length<int16_t> = 6;   // strlen("-32768")
	template <> inline constexpr size_t charconv_buffer_length<int8_t> = 4;    // strlen("-128")
	template <> inline constexpr size_t charconv_buffer_length<uint32_t> = 10; // strlen("4294967295")
	template <> inline constexpr size_t charconv_buffer_length<uint16_t> = 5;  // strlen("65535")
	template <> inline constexpr size_t charconv_buffer_length<uint8_t> = 3;   // strlen("255")

	template <typename T, typename Char>
	inline void print_integer_to_stream(T val, std::basic_ostream<Char>& stream, value_flags format = {})
	{
		static_assert(
			sizeof(Char) == 1,
			"The stream's underlying character type must be 1 byte in size."
		);

		if (!val)
		{
			print_to_stream('0', stream);
			return;
		}

		int base = 10;
		if (format != value_flags::none && val >= T{})
		{
			switch (format)
			{
				case value_flags::format_as_binary: base = 2; break;
				case value_flags::format_as_octal: base = 8; break;
				case value_flags::format_as_hexadecimal: base = 16; break;
				default: break;
			}
		}

		#if TOML_INT_CHARCONV
		{
			char buf[(sizeof(T) * CHAR_BIT)];
			const auto res = std::to_chars(buf, buf + sizeof(buf), val, base);
			const auto len = static_cast<size_t>(res.ptr - buf);
			if (base == 16)
			{
				for (size_t i = 0; i < len; i++)
					if (buf[i] >= 'a')
						buf[i] -= 32;
			}
			print_to_stream(buf, len, stream);
		}
		#else
		{
			using unsigned_type = std::conditional_t<(sizeof(T) > sizeof(unsigned)), std::make_unsigned_t<T>, unsigned>;
			using cast_type = std::conditional_t<std::is_signed_v<T>, std::make_signed_t<unsigned_type>, unsigned_type>;

			if TOML_UNLIKELY(format == value_flags::format_as_binary)
			{
				bool found_one = false;
				const auto v = static_cast<unsigned_type>(val);
				unsigned_type mask = unsigned_type{ 1 } << (sizeof(unsigned_type) * CHAR_BIT - 1u);
				for (unsigned i = 0; i < sizeof(unsigned_type) * CHAR_BIT; i++)
				{
					if ((v & mask))
					{
						print_to_stream('1', stream);
						found_one = true;
					}
					else if (found_one)
						print_to_stream('0', stream);
					mask >>= 1;
				}
			}
			else
			{
				std::ostringstream ss;
				ss.imbue(std::locale::classic());
				ss << std::uppercase << std::setbase(base);
				ss << static_cast<cast_type>(val);
				const auto str = std::move(ss).str();
				print_to_stream(str, stream);
			}
		}
		#endif
	}

	#define TOML_P2S_OVERLOAD(Type)																	\
		template <typename Char>																	\
		inline void print_to_stream(Type val, std::basic_ostream<Char>& stream, value_flags format)	\
		{																							\
			static_assert(sizeof(Char) == 1);														\
			print_integer_to_stream(val, stream, format);											\
		}

	TOML_P2S_OVERLOAD(int8_t)
	TOML_P2S_OVERLOAD(int16_t)
	TOML_P2S_OVERLOAD(int32_t)
	TOML_P2S_OVERLOAD(int64_t)
	TOML_P2S_OVERLOAD(uint8_t)
	TOML_P2S_OVERLOAD(uint16_t)
	TOML_P2S_OVERLOAD(uint32_t)
	TOML_P2S_OVERLOAD(uint64_t)

	#undef TOML_P2S_OVERLOAD

	template <typename T, typename Char>
	inline void print_floating_point_to_stream(T val, std::basic_ostream<Char>& stream, bool hexfloat = false)
	{
		static_assert(
			sizeof(Char) == 1,
			"The stream's underlying character type must be 1 byte in size."
		);

		switch (impl::fpclassify(val))
		{
			case fp_class::neg_inf:
				print_to_stream("-inf"sv, stream);
				break;

			case fp_class::pos_inf:
				print_to_stream("inf"sv, stream);
				break;

			case fp_class::nan:
				print_to_stream("nan"sv, stream);
				break;

			case fp_class::ok:
			{
				static constexpr auto needs_decimal_point = [](auto&& s) noexcept
				{
					for (auto c : s)
						if (c == '.' || c == 'E' || c == 'e')
							return false;
					return true;
				};

				#if TOML_FLOAT_CHARCONV
				{
					char buf[charconv_buffer_length<T>];
					const auto res = hexfloat
						? std::to_chars(buf, buf + sizeof(buf), val, std::chars_format::hex)
						: std::to_chars(buf, buf + sizeof(buf), val);
					const auto str = std::string_view{ buf, static_cast<size_t>(res.ptr - buf) };
					print_to_stream(str, stream);
					if (!hexfloat && needs_decimal_point(str))
						print_to_stream(".0"sv, stream);
				}
				#else
				{
					std::ostringstream ss;
					ss.imbue(std::locale::classic());
					ss.precision(std::numeric_limits<T>::digits10 + 1);
					if (hexfloat)
						ss << std::hexfloat;
					ss << val;
					const auto str = std::move(ss).str();
					print_to_stream(str, stream);
					if (!hexfloat && needs_decimal_point(str))
						print_to_stream(".0"sv, stream);
				}
				#endif

				break;
			}

			TOML_NO_DEFAULT_CASE;
		}
	}

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template TOML_API void print_floating_point_to_stream(double, std::ostream&, bool);
	#endif

	#define TOML_P2S_OVERLOAD(Type)											\
		template <typename Char>											\
		inline void print_to_stream(Type val, std::basic_ostream<Char>& stream)	\
		{																	\
			static_assert(sizeof(Char) == 1);								\
			print_floating_point_to_stream(val, stream);					\
		}

	TOML_P2S_OVERLOAD(double)

	#undef TOML_P2S_OVERLOAD

	template <typename Char>
	inline void print_to_stream(bool val, std::basic_ostream<Char>& stream)
	{
		static_assert(sizeof(Char) == 1);
		print_to_stream(val ? "true"sv : "false"sv, stream);
	}

	template <typename T, typename Char>
	inline void print_to_stream(T val, std::basic_ostream<Char>& stream, size_t zero_pad_to_digits)
	{
		static_assert(sizeof(Char) == 1);
		#if TOML_INT_CHARCONV

			char buf[charconv_buffer_length<T>];
			const auto res = std::to_chars(buf, buf + sizeof(buf), val);
			const auto len = static_cast<size_t>(res.ptr - buf);
			for (size_t i = len; i < zero_pad_to_digits; i++)
				print_to_stream('0', stream);
			print_to_stream(buf, static_cast<size_t>(res.ptr - buf), stream);

		#else

			std::ostringstream ss;
			ss.imbue(std::locale::classic());
			using cast_type = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>;
			ss << std::setfill('0') << std::setw(static_cast<int>(zero_pad_to_digits)) << static_cast<cast_type>(val);
			const auto str = std::move(ss).str();
			print_to_stream(str, stream);

		#endif
	}

	template <typename Char>
	inline void print_to_stream(const toml::date& val, std::basic_ostream<Char>& stream)
	{
		static_assert(sizeof(Char) == 1);
		print_to_stream(val.year, stream, 4_sz);
		print_to_stream('-', stream);
		print_to_stream(val.month, stream, 2_sz);
		print_to_stream('-', stream);
		print_to_stream(val.day, stream, 2_sz);
	}

	template <typename Char>
	inline void print_to_stream(const toml::time& val, std::basic_ostream<Char>& stream)
	{
		static_assert(sizeof(Char) == 1);
		print_to_stream(val.hour, stream, 2_sz);
		print_to_stream(':', stream);
		print_to_stream(val.minute, stream, 2_sz);
		print_to_stream(':', stream);
		print_to_stream(val.second, stream, 2_sz);
		if (val.nanosecond && val.nanosecond <= 999999999u)
		{
			print_to_stream('.', stream);
			auto ns = val.nanosecond;
			size_t digits = 9_sz;
			while (ns % 10u == 0u)
			{
				ns /= 10u;
				digits--;
			}
			print_to_stream(ns, stream, digits);
		}
	}

	template <typename Char>
	inline void print_to_stream(toml::time_offset val, std::basic_ostream<Char>& stream)
	{
		static_assert(sizeof(Char) == 1);
		if (!val.minutes)
			print_to_stream('Z', stream);
		else
		{
			auto mins = static_cast<int>(val.minutes);
			if (mins < 0)
			{
				print_to_stream('-', stream);
				mins = -mins;
			}
			else
				print_to_stream('+', stream);
			const auto hours = mins / 60;
			if (hours)
			{
				print_to_stream(static_cast<unsigned int>(hours), stream, 2_sz);
				mins -= hours * 60;
			}
			else
				print_to_stream("00"sv, stream);
			print_to_stream(':', stream);
			print_to_stream(static_cast<unsigned int>(mins), stream, 2_sz);
		}
	}

	template <typename Char>
	inline void print_to_stream(const toml::date_time& val, std::basic_ostream<Char>& stream)
	{
		static_assert(sizeof(Char) == 1);
		print_to_stream(val.date, stream);
		print_to_stream('T', stream);
		print_to_stream(val.time, stream);
		if (val.offset)
			print_to_stream(*val.offset, stream);
	}

	TOML_PUSH_WARNINGS
	TOML_DISABLE_ARITHMETIC_WARNINGS

	template <typename T, typename Char>
	void print_to_stream_with_escapes(T && str, std::basic_ostream<Char>& stream)
	{
		static_assert(sizeof(Char) == 1);
		for (auto c : str)
		{
			if TOML_UNLIKELY(c >= '\x00' && c <= '\x1F')
				print_to_stream(low_character_escape_table[c], stream);
			else if TOML_UNLIKELY(c == '\x7F')
				print_to_stream("\\u007F"sv, stream);
			else if TOML_UNLIKELY(c == '"')
				print_to_stream("\\\""sv, stream);
			else if TOML_UNLIKELY(c == '\\')
				print_to_stream("\\\\"sv, stream);
			else
				print_to_stream(c, stream);
		}
	}

	TOML_POP_WARNINGS // TOML_DISABLE_ARITHMETIC_WARNINGS
}
TOML_IMPL_NAMESPACE_END

TOML_NAMESPACE_START
{
	template <typename Char>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const source_position& rhs)
	{
		static_assert(
			sizeof(Char) == 1,
			"The stream's underlying character type must be 1 byte in size."
		);
		impl::print_to_stream("line "sv, lhs);
		impl::print_to_stream(rhs.line, lhs);
		impl::print_to_stream(", column "sv, lhs);
		impl::print_to_stream(rhs.column, lhs);
		return lhs;
	}

	template <typename Char>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const source_region& rhs)
	{
		static_assert(
			sizeof(Char) == 1,
			"The stream's underlying character type must be 1 byte in size."
		);
		lhs << rhs.begin;
		if (rhs.path)
		{
			impl::print_to_stream(" of '"sv, lhs);
			impl::print_to_stream(*rhs.path, lhs);
			impl::print_to_stream('\'', lhs);
		}
		return lhs;
	}

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template TOML_API std::ostream& operator << (std::ostream&, const source_position&);
		extern template TOML_API std::ostream& operator << (std::ostream&, const source_region&);
	#endif
}
TOML_NAMESPACE_END

TOML_POP_WARNINGS // TOML_DISABLE_SWITCH_WARNINGS

#endif //-------------------------------------------------------------------------------  ↑ toml_print_to_stream.h  ----

#if 1  //----------  ↓ toml_node.h  ------------------------------------------------------------------------------------

#if defined(DOXYGEN) || TOML_SIMPLE_STATIC_ASSERT_MESSAGES

#define TOML_SA_NEWLINE		" "
#define TOML_SA_LIST_SEP	", "
#define TOML_SA_LIST_BEG	" ("
#define TOML_SA_LIST_END	")"
#define TOML_SA_LIST_NEW	" "
#define TOML_SA_LIST_NXT	", "

#else

#define TOML_SA_NEWLINE			"\n| "
#define TOML_SA_LIST_SEP		TOML_SA_NEWLINE "  - "
#define TOML_SA_LIST_BEG		TOML_SA_LIST_SEP
#define TOML_SA_LIST_END
#define TOML_SA_LIST_NEW		TOML_SA_NEWLINE TOML_SA_NEWLINE
#define TOML_SA_LIST_NXT		TOML_SA_LIST_NEW

#endif

#define TOML_SA_NATIVE_VALUE_TYPE_LIST							\
	TOML_SA_LIST_BEG	"std::string"							\
	TOML_SA_LIST_SEP	"int64_t"								\
	TOML_SA_LIST_SEP	"double"								\
	TOML_SA_LIST_SEP	"bool"									\
	TOML_SA_LIST_SEP	"toml::date"							\
	TOML_SA_LIST_SEP	"toml::time"							\
	TOML_SA_LIST_SEP	"toml::date_time"						\
	TOML_SA_LIST_END

#define TOML_SA_NODE_TYPE_LIST									\
	TOML_SA_LIST_BEG	"toml::table"							\
	TOML_SA_LIST_SEP	"toml::array"							\
	TOML_SA_LIST_SEP	"toml::value<std::string>"				\
	TOML_SA_LIST_SEP	"toml::value<int64_t>"					\
	TOML_SA_LIST_SEP	"toml::value<double>"					\
	TOML_SA_LIST_SEP	"toml::value<bool>"						\
	TOML_SA_LIST_SEP	"toml::value<toml::date>"				\
	TOML_SA_LIST_SEP	"toml::value<toml::time>"				\
	TOML_SA_LIST_SEP	"toml::value<toml::date_time>"			\
	TOML_SA_LIST_END

#define TOML_SA_UNWRAPPED_NODE_TYPE_LIST						\
	TOML_SA_LIST_NEW	"A native TOML value type"				\
	TOML_SA_NATIVE_VALUE_TYPE_LIST								\
																\
	TOML_SA_LIST_NXT	"A TOML node type"						\
	TOML_SA_NODE_TYPE_LIST

TOML_NAMESPACE_START
{
	class TOML_INTERFACE TOML_API node
	{
		private:
			friend class TOML_PARSER_TYPENAME;
			source_region source_{};

		protected:

			node() noexcept = default;
			node(const node&) noexcept;
			node(node&&) noexcept;
			node& operator= (const node&) noexcept;
			node& operator= (node&&) noexcept;

			template <typename T>
			[[nodiscard]]
			TOML_ALWAYS_INLINE
			impl::wrap_node<T>& ref_cast() & noexcept
			{
				return *reinterpret_cast<impl::wrap_node<T>*>(this);
			}

			template <typename T>
			[[nodiscard]]
			TOML_ALWAYS_INLINE
			impl::wrap_node<T>&& ref_cast() && noexcept
			{
				return std::move(*reinterpret_cast<impl::wrap_node<T>*>(this));
			}

			template <typename T>
			[[nodiscard]]
			TOML_ALWAYS_INLINE
			const impl::wrap_node<T>& ref_cast() const & noexcept
			{
				return *reinterpret_cast<const impl::wrap_node<T>*>(this);
			}

			template <typename N, typename T>
			using ref_cast_type = decltype(std::declval<N>().template ref_cast<T>());

		public:

			virtual ~node() noexcept = default;

			[[nodiscard]] virtual node_type type() const noexcept = 0;
			[[nodiscard]] virtual bool is_table() const noexcept = 0;
			[[nodiscard]] virtual bool is_array() const noexcept = 0;
			[[nodiscard]] virtual bool is_value() const noexcept = 0;
			[[nodiscard]] virtual bool is_string() const noexcept;
			[[nodiscard]] virtual bool is_integer() const noexcept;
			[[nodiscard]] virtual bool is_floating_point() const noexcept;
			[[nodiscard]] virtual bool is_number() const noexcept;
			[[nodiscard]] virtual bool is_boolean() const noexcept;
			[[nodiscard]] virtual bool is_date() const noexcept;
			[[nodiscard]] virtual bool is_time() const noexcept;
			[[nodiscard]] virtual bool is_date_time() const noexcept;
			[[nodiscard]] virtual bool is_array_of_tables() const noexcept;

			template <typename T>
			[[nodiscard]]
			bool is() const noexcept
			{
				using type = impl::unwrap_node<T>;
				static_assert(
					(impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>,
					"The template type argument of node::is() must be one of:"
					TOML_SA_UNWRAPPED_NODE_TYPE_LIST
				);

					 if constexpr (std::is_same_v<type, table>) return is_table();
				else if constexpr (std::is_same_v<type, array>) return is_array();
				else if constexpr (std::is_same_v<type, std::string>) return is_string();
				else if constexpr (std::is_same_v<type, int64_t>) return is_integer();
				else if constexpr (std::is_same_v<type, double>) return is_floating_point();
				else if constexpr (std::is_same_v<type, bool>) return is_boolean();
				else if constexpr (std::is_same_v<type, date>) return is_date();
				else if constexpr (std::is_same_v<type, time>) return is_time();
				else if constexpr (std::is_same_v<type, date_time>) return is_date_time();
			}

			[[nodiscard]] virtual table* as_table() noexcept;
			[[nodiscard]] virtual array* as_array() noexcept;
			[[nodiscard]] virtual toml::value<std::string>* as_string() noexcept;
			[[nodiscard]] virtual toml::value<int64_t>* as_integer() noexcept;
			[[nodiscard]] virtual toml::value<double>* as_floating_point() noexcept;
			[[nodiscard]] virtual toml::value<bool>* as_boolean() noexcept;
			[[nodiscard]] virtual toml::value<date>* as_date() noexcept;
			[[nodiscard]] virtual toml::value<time>* as_time() noexcept;
			[[nodiscard]] virtual toml::value<date_time>* as_date_time() noexcept;
			[[nodiscard]] virtual const table* as_table() const noexcept;
			[[nodiscard]] virtual const array* as_array() const noexcept;
			[[nodiscard]] virtual const toml::value<std::string>* as_string() const noexcept;
			[[nodiscard]] virtual const toml::value<int64_t>* as_integer() const noexcept;
			[[nodiscard]] virtual const toml::value<double>* as_floating_point() const noexcept;
			[[nodiscard]] virtual const toml::value<bool>* as_boolean() const noexcept;
			[[nodiscard]] virtual const toml::value<date>* as_date() const noexcept;
			[[nodiscard]] virtual const toml::value<time>* as_time() const noexcept;
			[[nodiscard]] virtual const toml::value<date_time>* as_date_time() const noexcept;
			[[nodiscard]] virtual bool is_homogeneous(node_type ntype, node*& first_nonmatch) noexcept = 0;
			[[nodiscard]] virtual bool is_homogeneous(node_type ntype, const node*& first_nonmatch) const noexcept = 0;
			[[nodiscard]] virtual bool is_homogeneous(node_type ntype) const noexcept = 0;

			template <typename ElemType = void>
			[[nodiscard]]
			bool is_homogeneous() const noexcept
			{
				using type = impl::unwrap_node<ElemType>;
				static_assert(
					std::is_void_v<type>
					|| ((impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>),
					"The template type argument of node::is_homogeneous() must be void or one of:"
					TOML_SA_UNWRAPPED_NODE_TYPE_LIST
				);
				return is_homogeneous(impl::node_type_of<type>);
			}

		private:

			#ifndef DOXYGEN

			template <typename T>
			[[nodiscard]]
			decltype(auto) get_value_exact() const noexcept;

			#endif // !DOXYGEN

		public:

			template <typename T>
			[[nodiscard]]
			optional<T> value_exact() const noexcept;

			template <typename T>
			[[nodiscard]]
			optional<T> value() const noexcept;

			template <typename T>
			[[nodiscard]]
			auto value_or(T&& default_value) const noexcept;

			//template <typename T>
			//[[nodiscard]]
			//std::vector<T> select_exact() const noexcept;

			//template <typename T>
			//[[nodiscard]]
			//std::vector<T> select() const noexcept;

			template <typename T>
			[[nodiscard]]
			impl::wrap_node<T>* as() noexcept
			{
				using type = impl::unwrap_node<T>;
				static_assert(
					(impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>,
					"The template type argument of node::as() must be one of:"
					TOML_SA_UNWRAPPED_NODE_TYPE_LIST
				);

					 if constexpr (std::is_same_v<type, table>) return as_table();
				else if constexpr (std::is_same_v<type, array>) return as_array();
				else if constexpr (std::is_same_v<type, std::string>) return as_string();
				else if constexpr (std::is_same_v<type, int64_t>) return as_integer();
				else if constexpr (std::is_same_v<type, double>) return as_floating_point();
				else if constexpr (std::is_same_v<type, bool>) return as_boolean();
				else if constexpr (std::is_same_v<type, date>) return as_date();
				else if constexpr (std::is_same_v<type, time>) return as_time();
				else if constexpr (std::is_same_v<type, date_time>) return as_date_time();
			}

			template <typename T>
			[[nodiscard]]
			const impl::wrap_node<T>* as() const noexcept
			{
				using type = impl::unwrap_node<T>;
				static_assert(
					(impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>,
					"The template type argument of node::as() must be one of:"
					TOML_SA_UNWRAPPED_NODE_TYPE_LIST
				);

					 if constexpr (std::is_same_v<type, table>) return as_table();
				else if constexpr (std::is_same_v<type, array>) return as_array();
				else if constexpr (std::is_same_v<type, std::string>) return as_string();
				else if constexpr (std::is_same_v<type, int64_t>) return as_integer();
				else if constexpr (std::is_same_v<type, double>) return as_floating_point();
				else if constexpr (std::is_same_v<type, bool>) return as_boolean();
				else if constexpr (std::is_same_v<type, date>) return as_date();
				else if constexpr (std::is_same_v<type, time>) return as_time();
				else if constexpr (std::is_same_v<type, date_time>) return as_date_time();
			}

			[[nodiscard]] const source_region& source() const noexcept;

		private:

			template <typename Func, typename N, typename T>
			static constexpr bool can_visit = std::is_invocable_v<Func, ref_cast_type<N, T>>;

			template <typename Func, typename N>
			static constexpr bool can_visit_any =
				can_visit<Func, N, table>
				|| can_visit<Func, N, array>
				|| can_visit<Func, N, std::string>
				|| can_visit<Func, N, int64_t>
				|| can_visit<Func, N, double>
				|| can_visit<Func, N, bool>
				|| can_visit<Func, N, date>
				|| can_visit<Func, N, time>
				|| can_visit<Func, N, date_time>;

			template <typename Func, typename N>
			static constexpr bool can_visit_all =
				can_visit<Func, N, table>
				&& can_visit<Func, N, array>
				&& can_visit<Func, N, std::string>
				&& can_visit<Func, N, int64_t>
				&& can_visit<Func, N, double>
				&& can_visit<Func, N, bool>
				&& can_visit<Func, N, date>
				&& can_visit<Func, N, time>
				&& can_visit<Func, N, date_time>;

			template <typename Func, typename N, typename T>
			static constexpr bool visit_is_nothrow_one =
				!can_visit<Func, N, T>
				|| std::is_nothrow_invocable_v<Func, ref_cast_type<N, T>>;

			template <typename Func, typename N>
			static constexpr bool visit_is_nothrow =
				visit_is_nothrow_one<Func, N, table>
				&& visit_is_nothrow_one<Func, N, array>
				&& visit_is_nothrow_one<Func, N, std::string>
				&& visit_is_nothrow_one<Func, N, int64_t>
				&& visit_is_nothrow_one<Func, N, double>
				&& visit_is_nothrow_one<Func, N, bool>
				&& visit_is_nothrow_one<Func, N, date>
				&& visit_is_nothrow_one<Func, N, time>
				&& visit_is_nothrow_one<Func, N, date_time>;

			template <typename Func, typename N, typename T, bool = can_visit<Func, N, T>>
			struct visit_return_type final
			{
				using type = decltype(std::declval<Func>()(std::declval<ref_cast_type<N, T>>()));
			};
			template <typename Func, typename N, typename T>
			struct visit_return_type<Func, N, T, false> final
			{
				using type = void;
			};

			template <typename A, typename B>
			using nonvoid = std::conditional_t<std::is_void_v<A>, B, A>;

			template <typename N, typename Func>
			static decltype(auto) do_visit(N&& n, Func&& visitor)
				noexcept(visit_is_nothrow<Func&&, N&&>)
			{
				static_assert(
					can_visit_any<Func&&, N&&>,
					"TOML node visitors must be invocable for at least one of the toml::node specializations:"
					TOML_SA_NODE_TYPE_LIST
				);

				switch (n.type())
				{
					case node_type::table:
						if constexpr (can_visit<Func&&, N&&, table>)
							return std::forward<Func>(visitor)(std::forward<N>(n).template ref_cast<table>());
						break;

					case node_type::array:
						if constexpr (can_visit<Func&&, N&&, array>)
							return std::forward<Func>(visitor)(std::forward<N>(n).template ref_cast<array>());
						break;

					case node_type::string:
						if constexpr (can_visit<Func&&, N&&, std::string>)
							return std::forward<Func>(visitor)(std::forward<N>(n).template ref_cast<std::string>());
						break;

					case node_type::integer:
						if constexpr (can_visit<Func&&, N&&, int64_t>)
							return std::forward<Func>(visitor)(std::forward<N>(n).template ref_cast<int64_t>());
						break;

					case node_type::floating_point:
						if constexpr (can_visit<Func&&, N&&, double>)
							return std::forward<Func>(visitor)(std::forward<N>(n).template ref_cast<double>());
						break;

					case node_type::boolean:
						if constexpr (can_visit<Func&&, N&&, bool>)
							return std::forward<Func>(visitor)(std::forward<N>(n).template ref_cast<bool>());
						break;

					case node_type::date:
						if constexpr (can_visit<Func&&, N&&, date>)
							return std::forward<Func>(visitor)(std::forward<N>(n).template ref_cast<date>());
						break;

					case node_type::time:
						if constexpr (can_visit<Func&&, N&&, time>)
							return std::forward<Func>(visitor)(std::forward<N>(n).template ref_cast<time>());
						break;

					case node_type::date_time:
						if constexpr (can_visit<Func&&, N&&, date_time>)
							return std::forward<Func>(visitor)(std::forward<N>(n).template ref_cast<date_time>());
						break;

					case node_type::none: TOML_UNREACHABLE;
					TOML_NO_DEFAULT_CASE;
				}

				if constexpr (can_visit_all<Func&&, N&&>)
					TOML_UNREACHABLE;
				else
				{
					using return_type =
						nonvoid<typename visit_return_type<Func&&, N&&, table>::type,
						nonvoid<typename visit_return_type<Func&&, N&&, array>::type,
						nonvoid<typename visit_return_type<Func&&, N&&, std::string>::type,
						nonvoid<typename visit_return_type<Func&&, N&&, int64_t>::type,
						nonvoid<typename visit_return_type<Func&&, N&&, double>::type,
						nonvoid<typename visit_return_type<Func&&, N&&, bool>::type,
						nonvoid<typename visit_return_type<Func&&, N&&, date>::type,
						nonvoid<typename visit_return_type<Func&&, N&&, time>::type,
								typename visit_return_type<Func&&, N&&, date_time>::type
					>>>>>>>>;

					if constexpr (!std::is_void_v<return_type>)
					{
						static_assert(
							std::is_default_constructible_v<return_type>,
							"Non-exhaustive visitors must return a default-constructible type, or void"
						);
						return return_type{};
					}
				}
			}

			template <typename T, typename N>
			[[nodiscard]]
			static decltype(auto) do_ref(N&& n) noexcept
			{
				using type = impl::unwrap_node<T>;
				static_assert(
					(impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>,
					"The template type argument of node::ref() must be one of:"
					TOML_SA_UNWRAPPED_NODE_TYPE_LIST
				);
				TOML_ASSERT(
					n.template is<T>()
					&& "template type argument T provided to toml::node::ref() didn't match the node's actual type"
				);
				if constexpr (impl::is_native<type>)
					return std::forward<N>(n).template ref_cast<type>().get();
				else
					return std::forward<N>(n).template ref_cast<type>();
			}

		public:

			template <typename Func>
			decltype(auto) visit(Func&& visitor) &
				noexcept(visit_is_nothrow<Func&&, node&>)
			{
				return do_visit(*this, std::forward<Func>(visitor));
			}

			template <typename Func>
			decltype(auto) visit(Func&& visitor) &&
				noexcept(visit_is_nothrow<Func&&, node&&>)
			{
				return do_visit(std::move(*this), std::forward<Func>(visitor));
			}

			template <typename Func>
			decltype(auto) visit(Func&& visitor) const&
				noexcept(visit_is_nothrow<Func&&, const node&>)
			{
				return do_visit(*this, std::forward<Func>(visitor));
			}

			template <typename T>
			[[nodiscard]]
			impl::unwrap_node<T>& ref() & noexcept
			{
				return do_ref<T>(*this);
			}

			template <typename T>
			[[nodiscard]]
			impl::unwrap_node<T>&& ref() && noexcept
			{
				return do_ref<T>(std::move(*this));
			}

			template <typename T>
			[[nodiscard]]
			const impl::unwrap_node<T>& ref() const& noexcept
			{
				return do_ref<T>(*this);
			}

			[[nodiscard]] explicit operator node_view<node>() noexcept;
			[[nodiscard]] explicit operator node_view<const node>() const noexcept;
	};
}
TOML_NAMESPACE_END

#endif //----------  ↑ toml_node.h  ------------------------------------------------------------------------------------

#if 1  //----------------------------------  ↓ toml_value.h  -----------------------------------------------------------

#ifndef DOXYGEN
	#if TOML_WINDOWS_COMPAT
		#define TOML_SA_VALUE_MESSAGE_WSTRING			TOML_SA_LIST_SEP "std::wstring"
	#else
		#define TOML_SA_VALUE_MESSAGE_WSTRING
	#endif

	#ifdef __cpp_lib_char8_t
		#define TOML_SA_VALUE_MESSAGE_U8STRING_VIEW		TOML_SA_LIST_SEP "std::u8string_view"
		#define TOML_SA_VALUE_MESSAGE_CONST_CHAR8		TOML_SA_LIST_SEP "const char8_t*"
	#else
		#define TOML_SA_VALUE_MESSAGE_U8STRING_VIEW
		#define TOML_SA_VALUE_MESSAGE_CONST_CHAR8
	#endif

	#define TOML_SA_VALUE_EXACT_FUNC_MESSAGE(type_arg)															\
		"The " type_arg " must be one of:"																		\
		TOML_SA_LIST_NEW "A native TOML value type"																\
		TOML_SA_NATIVE_VALUE_TYPE_LIST																			\
																												\
		TOML_SA_LIST_NXT "A non-view type capable of losslessly representing a native TOML value type"			\
		TOML_SA_LIST_BEG "std::string"																			\
		TOML_SA_VALUE_MESSAGE_WSTRING																			\
		TOML_SA_LIST_SEP "any signed integer type >= 64 bits"													\
		TOML_SA_LIST_SEP "any floating-point type >= 64 bits"													\
		TOML_SA_LIST_END																						\
																												\
		TOML_SA_LIST_NXT "An immutable view type not requiring additional temporary storage"					\
		TOML_SA_LIST_BEG "std::string_view"																		\
		TOML_SA_VALUE_MESSAGE_U8STRING_VIEW																		\
		TOML_SA_LIST_SEP "const char*"																			\
		TOML_SA_VALUE_MESSAGE_CONST_CHAR8																		\
		TOML_SA_LIST_END

	#define TOML_SA_VALUE_FUNC_MESSAGE(type_arg)																\
		"The " type_arg " must be one of:"																		\
		TOML_SA_LIST_NEW "A native TOML value type"																\
		TOML_SA_NATIVE_VALUE_TYPE_LIST																			\
																												\
		TOML_SA_LIST_NXT "A non-view type capable of losslessly representing a native TOML value type"			\
		TOML_SA_LIST_BEG "std::string"																			\
		TOML_SA_VALUE_MESSAGE_WSTRING																			\
		TOML_SA_LIST_SEP "any signed integer type >= 64 bits"													\
		TOML_SA_LIST_SEP "any floating-point type >= 64 bits"													\
		TOML_SA_LIST_END																						\
																												\
		TOML_SA_LIST_NXT "A non-view type capable of (reasonably) representing a native TOML value type"		\
		TOML_SA_LIST_BEG "any other integer type"																\
		TOML_SA_LIST_SEP "any floating-point type >= 32 bits"													\
		TOML_SA_LIST_END																						\
																												\
		TOML_SA_LIST_NXT "An immutable view type not requiring additional temporary storage"					\
		TOML_SA_LIST_BEG "std::string_view"																		\
		TOML_SA_VALUE_MESSAGE_U8STRING_VIEW																		\
		TOML_SA_LIST_SEP "const char*"																			\
		TOML_SA_VALUE_MESSAGE_CONST_CHAR8																		\
		TOML_SA_LIST_END
#endif // !DOXYGEN

TOML_PUSH_WARNINGS
TOML_DISABLE_ARITHMETIC_WARNINGS

TOML_IMPL_NAMESPACE_START
{
	template <typename T, typename...>
	struct native_value_maker
	{
		template <typename... Args>
		[[nodiscard]]
		static T make(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>)
		{
			return T(std::forward<Args>(args)...);
		}
	};

	template <typename T>
	struct native_value_maker<T, T>
	{
		template <typename U>
		[[nodiscard]]
		TOML_ALWAYS_INLINE
		static U&& make(U&& val) noexcept
		{
			return std::forward<U>(val);
		}
	};

	#if defined(__cpp_lib_char8_t) || TOML_WINDOWS_COMPAT

	struct string_maker
	{
		template <typename T>
		[[nodiscard]]
		static std::string make(T&& arg) noexcept
		{
			#ifdef __cpp_lib_char8_t
			if constexpr (is_one_of<std::decay_t<T>, char8_t*, const char8_t*>)
				return std::string(reinterpret_cast<const char*>(static_cast<const char8_t*>(arg)));
			else if constexpr (is_one_of<remove_cvref_t<T>, std::u8string, std::u8string_view>)
				return std::string(reinterpret_cast<const char*>(static_cast<const char8_t*>(arg.data())), arg.length());
			#endif // __cpp_lib_char8_t

			#if TOML_WINDOWS_COMPAT
			if constexpr (is_wide_string<T>)
				return narrow(std::forward<T>(arg));
			#endif // TOML_WINDOWS_COMPAT
		}
	};
	#ifdef __cpp_lib_char8_t
	template <>	struct native_value_maker<std::string, char8_t*>			: string_maker {};
	template <>	struct native_value_maker<std::string, const char8_t*>		: string_maker {};
	template <>	struct native_value_maker<std::string, std::u8string>		: string_maker {};
	template <>	struct native_value_maker<std::string, std::u8string_view>	: string_maker {};
	#endif // __cpp_lib_char8_t
	#if TOML_WINDOWS_COMPAT
	template <>	struct native_value_maker<std::string, wchar_t*>			: string_maker {};
	template <>	struct native_value_maker<std::string, const wchar_t*>		: string_maker {};
	template <>	struct native_value_maker<std::string, std::wstring>		: string_maker {};
	template <>	struct native_value_maker<std::string, std::wstring_view>	: string_maker {};
	#endif // TOML_WINDOWS_COMPAT

	#endif // defined(__cpp_lib_char8_t) || TOML_WINDOWS_COMPAT

	template <typename T>
	[[nodiscard]]
	TOML_ATTR(const)
	inline optional<T> node_integer_cast(int64_t val) noexcept
	{
		static_assert(node_type_of<T> == node_type::integer);
		static_assert(!is_cvref<T>);

		using traits = value_traits<T>;
		if constexpr (!traits::is_signed)
		{
			if constexpr ((sizeof(T) * CHAR_BIT) < 63) // 63 bits == int64_max
			{
				using common_t = decltype(int64_t{} + T{});
				if (val < int64_t{} || static_cast<common_t>(val) > static_cast<common_t>(traits::max))
					return {};
			}
			else
			{
				if (val < int64_t{})
					return {};
			}
		}
		else
		{
			if (val < traits::min || val > traits::max)
				return {};
		}
		return { static_cast<T>(val) };
	}
}
TOML_IMPL_NAMESPACE_END

TOML_NAMESPACE_START
{
	template <typename ValueType>
	class TOML_API value final : public node
	{
		static_assert(
			impl::is_native<ValueType> && !impl::is_cvref<ValueType>,
			"A toml::value<> must model one of the native TOML value types:"
			TOML_SA_NATIVE_VALUE_TYPE_LIST
		);

		private:
			friend class TOML_PARSER_TYPENAME;

			template <typename T, typename U>
			[[nodiscard]]
			TOML_ALWAYS_INLINE
			TOML_ATTR(const)
			static auto as_value([[maybe_unused]] U* ptr) noexcept
			{
				if constexpr (std::is_same_v<value_type, T>)
					return ptr;
				else
					return nullptr;
			}

			ValueType val_;
			value_flags flags_ = value_flags::none;

		public:

			using value_type = ValueType;
			using value_arg = std::conditional_t<
				std::is_same_v<value_type, std::string>,
				std::string_view,
				std::conditional_t<impl::is_one_of<value_type, double, int64_t, bool>, value_type, const value_type&>
			>;

			template <typename... Args>
			TOML_NODISCARD_CTOR
			explicit value(Args&&... args)
				noexcept(noexcept(value_type(
					impl::native_value_maker<value_type, std::decay_t<Args>...>::make(std::forward<Args>(args)...)
				)))
				: val_(impl::native_value_maker<value_type, std::decay_t<Args>...>::make(std::forward<Args>(args)...))
			{}

			TOML_NODISCARD_CTOR
			value(const value& other) noexcept
				: node{ other },
				val_{ other.val_ },
				flags_{ other.flags_ }
			{}

			TOML_NODISCARD_CTOR
			value(value&& other) noexcept
				: node{ std::move(other) },
				val_{ std::move(other.val_) },
				flags_{ other.flags_ }
			{}

			value& operator= (const value& rhs) noexcept
			{
				node::operator=(rhs);
				val_ = rhs.val_;
				flags_ = rhs.flags_;
				return *this;
			}

			value& operator= (value&& rhs) noexcept
			{
				if (&rhs != this)
				{
					node::operator=(std::move(rhs));
					val_ = std::move(rhs.val_);
					flags_ = rhs.flags_;
				}
				return *this;
			}

			[[nodiscard]] node_type type() const noexcept override { return impl::node_type_of<value_type>; }
			[[nodiscard]] bool is_table() const noexcept override { return false; }
			[[nodiscard]] bool is_array() const noexcept override { return false; }
			[[nodiscard]] bool is_value() const noexcept override { return true; }
			[[nodiscard]] bool is_string() const noexcept override { return std::is_same_v<value_type, std::string>; }
			[[nodiscard]] bool is_integer() const noexcept override { return std::is_same_v<value_type, int64_t>; }
			[[nodiscard]] bool is_floating_point() const noexcept override { return std::is_same_v<value_type, double>; }
			[[nodiscard]] bool is_number() const noexcept override { return impl::is_one_of<value_type, int64_t, double>; }
			[[nodiscard]] bool is_boolean() const noexcept override { return std::is_same_v<value_type, bool>; }
			[[nodiscard]] bool is_date() const noexcept override { return std::is_same_v<value_type, date>; }
			[[nodiscard]] bool is_time() const noexcept override { return std::is_same_v<value_type, time>; }
			[[nodiscard]] bool is_date_time() const noexcept override { return std::is_same_v<value_type, date_time>; }
			[[nodiscard]] value<std::string>* as_string() noexcept override { return as_value<std::string>(this); }
			[[nodiscard]] value<int64_t>* as_integer() noexcept override { return as_value<int64_t>(this); }
			[[nodiscard]] value<double>* as_floating_point() noexcept override { return as_value<double>(this); }
			[[nodiscard]] value<bool>* as_boolean() noexcept override { return as_value<bool>(this); }
			[[nodiscard]] value<date>* as_date() noexcept override { return as_value<date>(this); }
			[[nodiscard]] value<time>* as_time() noexcept override { return as_value<time>(this); }
			[[nodiscard]] value<date_time>* as_date_time() noexcept override { return as_value<date_time>(this); }
			[[nodiscard]] const value<std::string>* as_string() const noexcept override { return as_value<std::string>(this); }
			[[nodiscard]] const value<int64_t>* as_integer() const noexcept override { return as_value<int64_t>(this); }
			[[nodiscard]] const value<double>* as_floating_point() const noexcept override { return as_value<double>(this); }
			[[nodiscard]] const value<bool>* as_boolean() const noexcept override { return as_value<bool>(this); }
			[[nodiscard]] const value<date>* as_date() const noexcept override { return as_value<date>(this); }
			[[nodiscard]] const value<time>* as_time() const noexcept override { return as_value<time>(this); }
			[[nodiscard]] const value<date_time>* as_date_time() const noexcept override { return as_value<date_time>(this); }
			[[nodiscard]]
			bool is_homogeneous(node_type ntype) const noexcept override
			{
				return ntype == node_type::none || ntype == impl::node_type_of<value_type>;
			}
			[[nodiscard]]
			bool is_homogeneous(node_type ntype, toml::node*& first_nonmatch) noexcept override
			{
				if (ntype != node_type::none && ntype != impl::node_type_of<value_type>)
				{
					first_nonmatch = this;
					return false;
				}
				return true;
			}
			[[nodiscard]]
			bool is_homogeneous(node_type ntype, const toml::node*& first_nonmatch) const noexcept override
			{
				if (ntype != node_type::none && ntype != impl::node_type_of<value_type>)
				{
					first_nonmatch = this;
					return false;
				}
				return true;
			}
			template <typename ElemType = void>
			[[nodiscard]]
			bool is_homogeneous() const noexcept
			{
				using type = impl::unwrap_node<ElemType>;
				static_assert(
					std::is_void_v<type>
					|| ((impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>),
					"The template type argument of value::is_homogeneous() must be void or one of:"
					TOML_SA_UNWRAPPED_NODE_TYPE_LIST
				);

				using type = impl::unwrap_node<ElemType>;
				if constexpr (std::is_void_v<type>)
					return true;
				else
					return impl::node_type_of<type> == impl::node_type_of<value_type>;
			}

			[[nodiscard]] value_type& get() & noexcept { return val_; }
			[[nodiscard]] value_type&& get() && noexcept { return std::move(val_); }
			[[nodiscard]] const value_type& get() const & noexcept { return val_; }

			[[nodiscard]] value_type& operator* () & noexcept { return val_; }
			[[nodiscard]] value_type&& operator* () && noexcept { return std::move(val_); }
			[[nodiscard]] const value_type& operator* () const& noexcept { return val_; }

			[[nodiscard]] explicit operator value_type& () & noexcept { return val_; }
			[[nodiscard]] explicit operator value_type && () && noexcept { return std::move(val_); }
			[[nodiscard]] explicit operator const value_type& () const& noexcept { return val_; }

			[[nodiscard]] value_flags flags() const noexcept
			{
				return flags_;
			}

			value& flags(value_flags new_flags) noexcept
			{
				flags_ = new_flags;
				return *this;
			}

			template <typename Char, typename T>
			friend std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const value<T>& rhs);
			// implemented in toml_default_formatter.h

			value& operator= (value_arg rhs) noexcept
			{
				if constexpr (std::is_same_v<value_type, std::string>)
					val_.assign(rhs);
				else
					val_ = rhs;
				return *this;
			}

			template <typename T = value_type, typename = std::enable_if_t<std::is_same_v<T, std::string>>>
			value& operator= (std::string&& rhs) noexcept
			{
				val_ = std::move(rhs);
				return *this;
			}

			[[nodiscard]]
			friend bool operator == (const value& lhs, value_arg rhs) noexcept
			{
				if constexpr (std::is_same_v<value_type, double>)
				{
					const auto lhs_class = impl::fpclassify(lhs.val_);
					const auto rhs_class = impl::fpclassify(rhs);
					if (lhs_class == impl::fp_class::nan && rhs_class == impl::fp_class::nan)
						return true;
					if ((lhs_class == impl::fp_class::nan) != (rhs_class == impl::fp_class::nan))
						return false;
				}
				return lhs.val_ == rhs;
			}
			TOML_ASYMMETRICAL_EQUALITY_OPS(const value&, value_arg, )
			[[nodiscard]] friend bool operator <  (const value& lhs, value_arg rhs) noexcept { return lhs.val_ < rhs; }
			[[nodiscard]] friend bool operator <  (value_arg lhs, const value& rhs) noexcept { return lhs < rhs.val_; }
			[[nodiscard]] friend bool operator <= (const value& lhs, value_arg rhs) noexcept { return lhs.val_ <= rhs; }
			[[nodiscard]] friend bool operator <= (value_arg lhs, const value& rhs) noexcept { return lhs <= rhs.val_; }
			[[nodiscard]] friend bool operator >  (const value& lhs, value_arg rhs) noexcept { return lhs.val_ > rhs; }
			[[nodiscard]] friend bool operator >  (value_arg lhs, const value& rhs) noexcept { return lhs > rhs.val_; }
			[[nodiscard]] friend bool operator >= (const value& lhs, value_arg rhs) noexcept { return lhs.val_ >= rhs; }
			[[nodiscard]] friend bool operator >= (value_arg lhs, const value& rhs) noexcept { return lhs >= rhs.val_; }

			template <typename T>
			[[nodiscard]]
			friend bool operator == (const value& lhs, const value<T>& rhs) noexcept
			{
				if constexpr (std::is_same_v<value_type, T>)
					return lhs == rhs.val_; //calls asymmetrical value-equality operator defined above
				else
					return false;
			}

			template <typename T>
			[[nodiscard]]
			friend bool operator != (const value& lhs, const value<T>& rhs) noexcept
			{
				return !(lhs == rhs);
			}

			template <typename T>
			[[nodiscard]]
			friend bool operator < (const value& lhs, const value<T>& rhs) noexcept
			{
				if constexpr (std::is_same_v<value_type, T>)
					return lhs.val_ < rhs.val_;
				else
					return impl::node_type_of<value_type> < impl::node_type_of<T>;
			}

			template <typename T>
			[[nodiscard]]
			friend bool operator <= (const value& lhs, const value<T>& rhs) noexcept
			{
				if constexpr (std::is_same_v<value_type, T>)
					return lhs.val_ <= rhs.val_;
				else
					return impl::node_type_of<value_type> <= impl::node_type_of<T>;
			}

			template <typename T>
			[[nodiscard]]
			friend bool operator > (const value& lhs, const value<T>& rhs) noexcept
			{
				if constexpr (std::is_same_v<value_type, T>)
					return lhs.val_ > rhs.val_;
				else
					return impl::node_type_of<value_type> > impl::node_type_of<T>;
			}

			template <typename T>
			[[nodiscard]]
			friend bool operator >= (const value& lhs, const value<T>& rhs) noexcept
			{
				if constexpr (std::is_same_v<value_type, T>)
					return lhs.val_ >= rhs.val_;
				else
					return impl::node_type_of<value_type> >= impl::node_type_of<T>;
			}
	};
	template <typename T>
	value(T) -> value<impl::native_type_of<impl::remove_cvref_t<T>>>;

	#ifndef DOXYGEN
	TOML_PUSH_WARNINGS
	TOML_DISABLE_INIT_WARNINGS
	TOML_DISABLE_SWITCH_WARNINGS

	#if !TOML_HEADER_ONLY
		extern template class TOML_API value<std::string>;
		extern template class TOML_API value<int64_t>;
		extern template class TOML_API value<double>;
		extern template class TOML_API value<bool>;
		extern template class TOML_API value<date>;
		extern template class TOML_API value<time>;
		extern template class TOML_API value<date_time>;
	#endif

	template <typename T>
	[[nodiscard]]
	inline decltype(auto) node::get_value_exact() const noexcept
	{
		using namespace impl;

		static_assert(node_type_of<T> != node_type::none);
		static_assert(node_type_of<T> != node_type::table);
		static_assert(node_type_of<T> != node_type::array);
		static_assert(is_native<T> || can_represent_native<T>);
		static_assert(!is_cvref<T>);
		TOML_ASSERT(this->type() == node_type_of<T>);

		if constexpr (node_type_of<T> == node_type::string)
		{
			const auto& str = *ref_cast<std::string>();
			if constexpr (std::is_same_v<T, std::string>)
				return str;
			else if constexpr (std::is_same_v<T, std::string_view>)
				return T{ str };
			else if constexpr (std::is_same_v<T, const char*>)
				return str.c_str();

			else if constexpr (std::is_same_v<T, std::wstring>)
			{
				#if TOML_WINDOWS_COMPAT
				return widen(str);
				#else
				static_assert(dependent_false<T>, "Evaluated unreachable branch!");
				#endif
			}

			#ifdef __cpp_lib_char8_t

			// char -> char8_t (potentially unsafe - the feature is 'experimental'!)
			else if constexpr (is_one_of<T, std::u8string, std::u8string_view>)
				return T(reinterpret_cast<const char8_t*>(str.c_str()), str.length());
			else if constexpr (std::is_same_v<T, const char8_t*>)
				return reinterpret_cast<const char8_t*>(str.c_str());
			else
				static_assert(dependent_false<T>, "Evaluated unreachable branch!");

			#endif
		}
		else
			return static_cast<T>(*ref_cast<native_type_of<T>>());
	}

	template <typename T>
	inline optional<T> node::value_exact() const noexcept
	{
		using namespace impl;

		static_assert(
			!is_wide_string<T> || TOML_WINDOWS_COMPAT,
			"Retrieving values as wide-character strings with node::value_exact() is only "
			"supported on Windows with TOML_WINDOWS_COMPAT enabled."
		);

		static_assert(
			(is_native<T> || can_represent_native<T>) && !is_cvref<T>,
			TOML_SA_VALUE_EXACT_FUNC_MESSAGE("return type of node::value_exact()")
		);

		// prevent additional compiler error spam when the static_assert fails by gating behind if constexpr
		if constexpr ((is_native<T> || can_represent_native<T>) && !is_cvref<T>)
		{
			if (type() == node_type_of<T>)
				return { this->get_value_exact<T>() };
			else
				return {};
		}
	}

	template <typename T>
	inline optional<T> node::value() const noexcept
	{
		using namespace impl;

		static_assert(
			!is_wide_string<T> || TOML_WINDOWS_COMPAT,
			"Retrieving values as wide-character strings with node::value() is only "
			"supported on Windows with TOML_WINDOWS_COMPAT enabled."
		);
		static_assert(
			(is_native<T> || can_represent_native<T> || can_partially_represent_native<T>) && !is_cvref<T>,
			TOML_SA_VALUE_FUNC_MESSAGE("return type of node::value()")
		);

		// when asking for strings, dates, times and date_times there's no 'fuzzy' conversion
		// semantics to be mindful of so the exact retrieval is enough.
		if constexpr (is_natively_one_of<T, std::string, time, date, date_time>)
		{
			if (type() == node_type_of<T>)
				return { this->get_value_exact<T>() };
			else
				return {};
		}

		// everything else requires a bit of logicking.
		else
		{
			switch (type())
			{
				// int -> *
				case node_type::integer:
				{
					// int -> int
					if constexpr (is_natively_one_of<T, int64_t>)
					{
						if constexpr (is_native<T> || can_represent_native<T>)
							return static_cast<T>(*ref_cast<int64_t>());
						else
							return node_integer_cast<T>(*ref_cast<int64_t>());
					}

					// int -> float
					else if constexpr (is_natively_one_of<T, double>)
					{
						const int64_t val = *ref_cast<int64_t>();
						if constexpr (std::numeric_limits<T>::digits < 64)
						{
							constexpr auto largest_whole_float = (int64_t{ 1 } << std::numeric_limits<T>::digits);
							if (val < -largest_whole_float || val > largest_whole_float)
								return {};
						}
						return static_cast<T>(val);
					}

					// int -> bool
					else if constexpr (is_natively_one_of<T, bool>)
						return static_cast<bool>(*ref_cast<int64_t>());

					// int -> anything else
					else
						return {};
				}

				// float -> *
				case node_type::floating_point:
				{
					// float -> float
					if constexpr (is_natively_one_of<T, double>)
					{
						if constexpr (is_native<T> || can_represent_native<T>)
							return { static_cast<T>(*ref_cast<double>()) };
						else
						{
							const double val = *ref_cast<double>();
							if (val < (std::numeric_limits<T>::lowest)() || val > (std::numeric_limits<T>::max)())
								return {};
							return { static_cast<T>(val) };
						}
					}

					// float -> int
					else if constexpr (is_natively_one_of<T, int64_t>)
					{
						const double val = *ref_cast<double>();
						if (static_cast<double>(static_cast<int64_t>(val)) == val)
							return node_integer_cast<T>(static_cast<int64_t>(val));
						else
							return {};
					}

					// float -> anything else
					else
						return {};
				}

				// bool -> *
				case node_type::boolean:
				{
					// bool -> bool
					if constexpr (is_natively_one_of<T, bool>)
						return { *ref_cast<bool>() };

					// bool -> int
					else if constexpr (is_natively_one_of<T, int64_t>)
						return { static_cast<T>(*ref_cast<bool>()) };

					// bool -> anything else
					else
						return {};
				}
			}

			// non-values, or 'exact' types covered above
			return {};
		}
	}

	template <typename T>
	inline auto node::value_or(T&& default_value) const noexcept
	{
		using namespace impl;

		static_assert(
			!is_wide_string<T> || TOML_WINDOWS_COMPAT,
			"Retrieving values as wide-character strings with node::value_or() is only "
			"supported on Windows with TOML_WINDOWS_COMPAT enabled."
		);

		if constexpr (is_wide_string<T>)
		{
			#if TOML_WINDOWS_COMPAT

			if (type() == node_type::string)
				return widen(*ref_cast<std::string>());
			return std::wstring{ std::forward<T>(default_value) };

			#else

			static_assert(dependent_false<T>, "Evaluated unreachable branch!");

			#endif
		}
		else
		{
			using value_type = std::conditional_t<
				std::is_pointer_v<std::decay_t<T>>,
				std::add_pointer_t<std::add_const_t<std::remove_pointer_t<std::decay_t<T>>>>,
				std::decay_t<T>
			>;
			using traits = value_traits<value_type>;

			static_assert(
				traits::is_native || traits::can_represent_native || traits::can_partially_represent_native,
				"The default value type of node::value_or() must be one of:"
				TOML_SA_LIST_NEW "A native TOML value type"
				TOML_SA_NATIVE_VALUE_TYPE_LIST

				TOML_SA_LIST_NXT "A non-view type capable of losslessly representing a native TOML value type"
				TOML_SA_LIST_BEG "std::string"
				#if TOML_WINDOWS_COMPAT
				TOML_SA_LIST_SEP "std::wstring"
				#endif
				TOML_SA_LIST_SEP "any signed integer type >= 64 bits"
				TOML_SA_LIST_SEP "any floating-point type >= 64 bits"
				TOML_SA_LIST_END

				TOML_SA_LIST_NXT "A non-view type capable of (reasonably) representing a native TOML value type"
				TOML_SA_LIST_BEG "any other integer type"
				TOML_SA_LIST_SEP "any floating-point type >= 32 bits"
				TOML_SA_LIST_END

				TOML_SA_LIST_NXT "A compatible view type"
				TOML_SA_LIST_BEG "std::string_view"
				#ifdef __cpp_lib_char8_t
				TOML_SA_LIST_SEP "std::u8string_view"
				#endif
				#if TOML_WINDOWS_COMPAT
				TOML_SA_LIST_SEP "std::wstring_view"
				#endif
				TOML_SA_LIST_SEP "const char*"
				#ifdef __cpp_lib_char8_t
				TOML_SA_LIST_SEP "const char8_t*"
				#endif
				#if TOML_WINDOWS_COMPAT
				TOML_SA_LIST_SEP "const wchar_t*"
				#endif
				TOML_SA_LIST_END
			);

			// prevent additional compiler error spam when the static_assert fails by gating behind if constexpr
			if constexpr (traits::is_native || traits::can_represent_native || traits::can_partially_represent_native)
			{
				if constexpr (traits::is_native)
				{
					if (type() == node_type_of<value_type>)
						return *ref_cast<typename traits::native_type>();
				}
				if (auto val = this->value<value_type>())
					return *val;
				if constexpr (std::is_pointer_v<value_type>)
					return value_type{ default_value };
				else
					return std::forward<T>(default_value);
			}
		}
	}

	#if !TOML_HEADER_ONLY

	#define TOML_EXTERN(name, T)	\
		extern template TOML_API optional<T>		node::name<T>() const noexcept
	TOML_EXTERN(value_exact, std::string_view);
	TOML_EXTERN(value_exact, std::string);
	TOML_EXTERN(value_exact, const char*);
	TOML_EXTERN(value_exact, int64_t);
	TOML_EXTERN(value_exact, double);
	TOML_EXTERN(value_exact, date);
	TOML_EXTERN(value_exact, time);
	TOML_EXTERN(value_exact, date_time);
	TOML_EXTERN(value_exact, bool);
	TOML_EXTERN(value, std::string_view);
	TOML_EXTERN(value, std::string);
	TOML_EXTERN(value, const char*);
	TOML_EXTERN(value, signed char);
	TOML_EXTERN(value, signed short);
	TOML_EXTERN(value, signed int);
	TOML_EXTERN(value, signed long);
	TOML_EXTERN(value, signed long long);
	TOML_EXTERN(value, unsigned char);
	TOML_EXTERN(value, unsigned short);
	TOML_EXTERN(value, unsigned int);
	TOML_EXTERN(value, unsigned long);
	TOML_EXTERN(value, unsigned long long);
	TOML_EXTERN(value, double);
	TOML_EXTERN(value, float);
	TOML_EXTERN(value, date);
	TOML_EXTERN(value, time);
	TOML_EXTERN(value, date_time);
	TOML_EXTERN(value, bool);
	#ifdef __cpp_lib_char8_t
	TOML_EXTERN(value_exact, std::u8string_view);
	TOML_EXTERN(value_exact, std::u8string);
	TOML_EXTERN(value_exact, const char8_t*);
	TOML_EXTERN(value, std::u8string_view);
	TOML_EXTERN(value, std::u8string);
	TOML_EXTERN(value, const char8_t*);
	#endif
	#if TOML_WINDOWS_COMPAT
	TOML_EXTERN(value_exact, std::wstring);
	TOML_EXTERN(value, std::wstring);
	#endif
	#undef TOML_EXTERN

	#endif // !TOML_HEADER_ONLY

	TOML_POP_WARNINGS // TOML_DISABLE_INIT_WARNINGS, TOML_DISABLE_SWITCH_WARNINGS
	#endif // !DOXYGEN
}
TOML_NAMESPACE_END

TOML_POP_WARNINGS // TOML_DISABLE_ARITHMETIC_WARNINGS

#endif //----------------------------------  ↑ toml_value.h  -----------------------------------------------------------

#if 1  //-----------------------------------------------------------  ↓ toml_array.h  ----------------------------------

TOML_IMPL_NAMESPACE_START
{
	template <bool IsConst>
	class TOML_TRIVIAL_ABI array_iterator final
	{
		private:
			friend class TOML_NAMESPACE::array;

			using raw_mutable_iterator = std::vector<std::unique_ptr<node>>::iterator;
			using raw_const_iterator = std::vector<std::unique_ptr<node>>::const_iterator;
			using raw_iterator = std::conditional_t<IsConst, raw_const_iterator, raw_mutable_iterator>;

			mutable raw_iterator raw_;

			array_iterator(raw_mutable_iterator raw) noexcept
				: raw_{ raw }
			{}

			template <bool C = IsConst, typename = std::enable_if_t<C>>
			TOML_NODISCARD_CTOR
			array_iterator(raw_const_iterator raw) noexcept
				: raw_{ raw }
			{}

		public:

			using value_type = std::conditional_t<IsConst, const node, node>;
			using reference = value_type&;
			using pointer = value_type*;
			using difference_type = ptrdiff_t;

			array_iterator() noexcept = default;
			array_iterator(const array_iterator&) noexcept = default;
			array_iterator& operator = (const array_iterator&) noexcept = default;

			array_iterator& operator++() noexcept // ++pre
			{
				++raw_;
				return *this;
			}

			array_iterator operator++(int) noexcept // post++
			{
				array_iterator out{ raw_ };
				++raw_;
				return out;
			}

			array_iterator& operator--() noexcept // --pre
			{
				--raw_;
				return *this;
			}

			array_iterator operator--(int) noexcept // post--
			{
				array_iterator out{ raw_ };
				--raw_;
				return out;
			}

			[[nodiscard]]
			reference operator * () const noexcept
			{
				return *raw_->get();
			}

			[[nodiscard]]
			pointer operator -> () const noexcept
			{
				return raw_->get();
			}

			array_iterator& operator += (ptrdiff_t rhs) noexcept
			{
				raw_ += rhs;
				return *this;
			}

			array_iterator& operator -= (ptrdiff_t rhs) noexcept
			{
				raw_ -= rhs;
				return *this;
			}

			[[nodiscard]]
			friend array_iterator operator + (const array_iterator& lhs, ptrdiff_t rhs) noexcept
			{
				return { lhs.raw_ + rhs };
			}

			[[nodiscard]]
			friend array_iterator operator + (ptrdiff_t lhs, const array_iterator& rhs) noexcept
			{
				return { rhs.raw_ + lhs };
			}

			[[nodiscard]]
			friend array_iterator operator - (const array_iterator& lhs, ptrdiff_t rhs) noexcept
			{
				return { lhs.raw_ - rhs };
			}

			[[nodiscard]]
			friend ptrdiff_t operator - (const array_iterator& lhs, const array_iterator& rhs) noexcept
			{
				return lhs.raw_ - rhs.raw_;
			}

			[[nodiscard]]
			friend bool operator == (const array_iterator& lhs, const array_iterator& rhs) noexcept
			{
				return lhs.raw_ == rhs.raw_;
			}

			[[nodiscard]]
			friend bool operator != (const array_iterator& lhs, const array_iterator& rhs) noexcept
			{
				return lhs.raw_ != rhs.raw_;
			}

			[[nodiscard]]
			friend bool operator < (const array_iterator& lhs, const array_iterator& rhs) noexcept
			{
				return lhs.raw_ < rhs.raw_;
			}

			[[nodiscard]]
			friend bool operator <= (const array_iterator& lhs, const array_iterator& rhs) noexcept
			{
				return lhs.raw_ <= rhs.raw_;
			}

			[[nodiscard]]
			friend bool operator > (const array_iterator& lhs, const array_iterator& rhs) noexcept
			{
				return lhs.raw_ > rhs.raw_;
			}

			[[nodiscard]]
			friend bool operator >= (const array_iterator& lhs, const array_iterator& rhs) noexcept
			{
				return lhs.raw_ >= rhs.raw_;
			}

			[[nodiscard]]
			reference operator[] (ptrdiff_t idx) const noexcept
			{
				return *(raw_ + idx)->get();
			}

			TOML_DISABLE_WARNINGS

			template <bool C = IsConst, typename = std::enable_if_t<!C>>
			operator array_iterator<true>() const noexcept
			{
				return array_iterator<true>{ raw_ };
			}

			TOML_ENABLE_WARNINGS
	};

	template <typename T>
	[[nodiscard]]
	TOML_ATTR(returns_nonnull)
	auto* make_node_specialized(T&& val) noexcept
	{
		using type = unwrap_node<remove_cvref_t<T>>;
		static_assert(!std::is_same_v<type, node>);
		static_assert(!is_node_view<type>);

		if constexpr (is_one_of<type, array, table>)
		{
			return new type{ std::forward<T>(val) };
		}
		else
		{
			static_assert(
				!is_wide_string<T> || TOML_WINDOWS_COMPAT,
				"Instantiating values from wide-character strings is only "
				"supported on Windows with TOML_WINDOWS_COMPAT enabled."
			);
			static_assert(
				is_native<type> || is_losslessly_convertible_to_native<type>,
				"Value initializers must be (or be promotable to) one of the TOML value types"
			);
			if constexpr (is_wide_string<T>)
			{
				#if TOML_WINDOWS_COMPAT
				return new value{ narrow(std::forward<T>(val)) };
				#else
				static_assert(dependent_false<T>, "Evaluated unreachable branch!");
				#endif
			}
			else
				return new value{ std::forward<T>(val) };
		}
	}

	template <typename T>
	[[nodiscard]]
	auto* make_node(T&& val) noexcept
	{
		using type = unwrap_node<remove_cvref_t<T>>;
		if constexpr (std::is_same_v<type, node> || is_node_view<type>)
		{
			if constexpr (is_node_view<type>)
			{
				if (!val)
					return static_cast<toml::node*>(nullptr);
			}

			return std::forward<T>(val).visit([](auto&& concrete) noexcept
			{
				return static_cast<toml::node*>(make_node_specialized(std::forward<decltype(concrete)>(concrete)));
			});
		}
		else
			return make_node_specialized(std::forward<T>(val));
	}

	template <typename T>
	[[nodiscard]]
	auto* make_node(inserter<T>&& val) noexcept
	{
		return make_node(std::move(val.value));
	}
}
TOML_IMPL_NAMESPACE_END

TOML_NAMESPACE_START
{
	using array_iterator = impl::array_iterator<false>;
	using const_array_iterator = impl::array_iterator<true>;

	class TOML_API array final
		: public node
	{
		private:
			friend class TOML_PARSER_TYPENAME;
			std::vector<std::unique_ptr<node>> elements;

			void preinsertion_resize(size_t idx, size_t count) noexcept;

			template <typename T>
			void emplace_back_if_not_empty_view(T&& val) noexcept
			{
				if constexpr (impl::is_node_view<T>)
				{
					if (!val)
						return;
				}
				elements.emplace_back(impl::make_node(std::forward<T>(val)));
			}

		public:

			using value_type = node;
			using size_type = size_t;
			using difference_type = ptrdiff_t;
			using reference = node&;
			using const_reference = const node&;
			using iterator = array_iterator;
			using const_iterator = const_array_iterator;

			TOML_NODISCARD_CTOR
			array() noexcept;

			TOML_NODISCARD_CTOR
			array(const array&) noexcept;

			TOML_NODISCARD_CTOR
			array(array&& other) noexcept;
			array& operator= (const array&) noexcept;
			array& operator= (array&& rhs) noexcept;

			template <typename ElemType, typename... ElemTypes, typename = std::enable_if_t<
				(sizeof...(ElemTypes) > 0_sz)
				|| !std::is_same_v<impl::remove_cvref_t<ElemType>, array>
			>>
			TOML_NODISCARD_CTOR
			explicit array(ElemType&& val, ElemTypes&&... vals)
			{
				elements.reserve(sizeof...(ElemTypes) + 1_sz);
				emplace_back_if_not_empty_view(std::forward<ElemType>(val));
				if constexpr (sizeof...(ElemTypes) > 0)
				{
					(
						emplace_back_if_not_empty_view(std::forward<ElemTypes>(vals)),
						...
					);
				}
			}

			[[nodiscard]] node_type type() const noexcept override;
			[[nodiscard]] bool is_table() const noexcept override;
			[[nodiscard]] bool is_array() const noexcept override;
			[[nodiscard]] bool is_value() const noexcept override;
			[[nodiscard]] array* as_array() noexcept override;
			[[nodiscard]] const array* as_array() const noexcept override;
			[[nodiscard]] bool is_array_of_tables() const noexcept override;
			[[nodiscard]] bool is_homogeneous(node_type ntype) const noexcept override;
			[[nodiscard]] bool is_homogeneous(node_type ntype, node*& first_nonmatch) noexcept override;
			[[nodiscard]] bool is_homogeneous(node_type ntype, const node*& first_nonmatch) const noexcept override;
			template <typename ElemType = void>
			[[nodiscard]]
			bool is_homogeneous() const noexcept
			{
				using type = impl::unwrap_node<ElemType>;
				static_assert(
					std::is_void_v<type>
					|| ((impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>),
					"The template type argument of array::is_homogeneous() must be void or one of:"
					TOML_SA_UNWRAPPED_NODE_TYPE_LIST
				);
				return is_homogeneous(impl::node_type_of<type>);
			}

			[[nodiscard]] node& operator[] (size_t index) noexcept;
			[[nodiscard]] const node& operator[] (size_t index) const noexcept;
			[[nodiscard]] node& front() noexcept;
			[[nodiscard]] const node& front() const noexcept;
			[[nodiscard]] node& back() noexcept;
			[[nodiscard]] const node& back() const noexcept;
			[[nodiscard]] iterator begin() noexcept;
			[[nodiscard]] const_iterator begin() const noexcept;
			[[nodiscard]] const_iterator cbegin() const noexcept;
			[[nodiscard]] iterator end() noexcept;
			[[nodiscard]] const_iterator end() const noexcept;
			[[nodiscard]] const_iterator cend() const noexcept;
			[[nodiscard]] bool empty() const noexcept;
			[[nodiscard]] size_t size() const noexcept;
			void reserve(size_t new_capacity);
			void clear() noexcept;

			[[nodiscard]] size_t max_size() const noexcept;
			[[nodiscard]] size_t capacity() const noexcept;
			void shrink_to_fit();

			template <typename ElemType>
			iterator insert(const_iterator pos, ElemType&& val) noexcept
			{
				if constexpr (impl::is_node_view<ElemType>)
				{
					if (!val)
						return end();
				}
				return { elements.emplace(pos.raw_, impl::make_node(std::forward<ElemType>(val))) };
			}

			template <typename ElemType>
			iterator insert(const_iterator pos, size_t count, ElemType&& val) noexcept
			{
				if constexpr (impl::is_node_view<ElemType>)
				{
					if (!val)
						return end();
				}
				switch (count)
				{
					case 0: return { elements.begin() + (pos.raw_ - elements.cbegin()) };
					case 1: return insert(pos, std::forward<ElemType>(val));
					default:
					{
						const auto start_idx = static_cast<size_t>(pos.raw_ - elements.cbegin());
						preinsertion_resize(start_idx, count);
						size_t i = start_idx;
						for (size_t e = start_idx + count - 1_sz; i < e; i++)
							elements[i].reset(impl::make_node(val));

						elements[i].reset(impl::make_node(std::forward<ElemType>(val)));
						return { elements.begin() + static_cast<ptrdiff_t>(start_idx) };
					}
				}
			}

			template <typename Iter>
			iterator insert(const_iterator pos, Iter first, Iter last) noexcept
			{
				const auto distance = std::distance(first, last);
				if (distance <= 0)
					return { elements.begin() + (pos.raw_ - elements.cbegin()) };
				else
				{
					auto count = distance;
					using deref_type = decltype(*first);
					if constexpr (impl::is_node_view<deref_type>)
					{
						for (auto it = first; it != last; it++)
							if (!(*it))
								count--;
						if (!count)
							return { elements.begin() + (pos.raw_ - elements.cbegin()) };
					}
					const auto start_idx = static_cast<size_t>(pos.raw_ - elements.cbegin());
					preinsertion_resize(start_idx, static_cast<size_t>(count));
					size_t i = start_idx;
					for (auto it = first; it != last; it++)
					{
						if constexpr (impl::is_node_view<deref_type>)
						{
							if (!(*it))
								continue;
						}
						if constexpr (std::is_rvalue_reference_v<deref_type>)
							elements[i++].reset(impl::make_node(std::move(*it)));
						else
							elements[i++].reset(impl::make_node(*it));
					}
					return { elements.begin() + static_cast<ptrdiff_t>(start_idx) };
				}
			}

			template <typename ElemType>
			iterator insert(const_iterator pos, std::initializer_list<ElemType> ilist) noexcept
			{
				return insert(pos, ilist.begin(), ilist.end());
			}

			template <typename ElemType, typename... Args>
			iterator emplace(const_iterator pos, Args&&... args) noexcept
			{
				using type = impl::unwrap_node<ElemType>;
				static_assert(
					(impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>,
					"Emplacement type parameter must be one of:"
					TOML_SA_UNWRAPPED_NODE_TYPE_LIST
				);

				return { elements.emplace(pos.raw_, new impl::wrap_node<type>{ std::forward<Args>(args)...} ) };
			}

			iterator erase(const_iterator pos) noexcept;
			iterator erase(const_iterator first, const_iterator last) noexcept;

			template <typename ElemType>
			void resize(size_t new_size, ElemType&& default_init_val) noexcept
			{
				static_assert(
					!impl::is_node_view<ElemType>,
					"The default element type argument to toml::array::resize may not be toml::node_view."
				);

				if (!new_size)
					elements.clear();
				else if (new_size < elements.size())
					elements.resize(new_size);
				else if (new_size > elements.size())
					insert(cend(), new_size - elements.size(), std::forward<ElemType>(default_init_val));
			}

			void truncate(size_t new_size);

			template <typename ElemType>
			void push_back(ElemType&& val) noexcept
			{
				emplace_back_if_not_empty_view(std::forward<ElemType>(val));
			}

			template <typename ElemType, typename... Args>
			decltype(auto) emplace_back(Args&&... args) noexcept
			{
				using type = impl::unwrap_node<ElemType>;
				static_assert(
					(impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>,
					"Emplacement type parameter must be one of:"
					TOML_SA_UNWRAPPED_NODE_TYPE_LIST
				);

				auto nde = new impl::wrap_node<type>{ std::forward<Args>(args)... };
				elements.emplace_back(nde);
				return *nde;
			}

			void pop_back() noexcept;
			[[nodiscard]] node* get(size_t index) noexcept;
			[[nodiscard]] const node* get(size_t index) const noexcept;

			template <typename ElemType>
			[[nodiscard]]
			impl::wrap_node<ElemType>* get_as(size_t index) noexcept
			{
				if (auto val = get(index))
					return val->as<ElemType>();
				return nullptr;
			}

			template <typename ElemType>
			[[nodiscard]]
			const impl::wrap_node<ElemType>* get_as(size_t index) const noexcept
			{
				if (auto val = get(index))
					return val->as<ElemType>();
				return nullptr;
			}

			friend bool operator == (const array& lhs, const array& rhs) noexcept;
			friend bool operator != (const array& lhs, const array& rhs) noexcept;

		private:

			template <typename T>
			[[nodiscard]]
			static bool container_equality(const array& lhs, const T& rhs) noexcept
			{
				using element_type = std::remove_const_t<typename T::value_type>;
				static_assert(
					impl::is_native<element_type> || impl::is_losslessly_convertible_to_native<element_type>,
					"Container element type must be (or be promotable to) one of the TOML value types"
				);

				if (lhs.size() != rhs.size())
					return false;
				if (rhs.size() == 0_sz)
					return true;

				size_t i{};
				for (auto& list_elem : rhs)
				{
					const auto elem = lhs.get_as<impl::native_type_of<element_type>>(i++);
					if (!elem || *elem != list_elem)
						return false;
				}

				return true;
			}

			[[nodiscard]] size_t total_leaf_count() const noexcept;
			void flatten_child(array&& child, size_t& dest_index) noexcept;

		public:

			template <typename T>
			[[nodiscard]]
			friend bool operator == (const array& lhs, const std::initializer_list<T>& rhs) noexcept
			{
				return container_equality(lhs, rhs);
			}
			TOML_ASYMMETRICAL_EQUALITY_OPS(const array&, const std::initializer_list<T>&, template <typename T>)

			template <typename T>
			[[nodiscard]]
			friend bool operator == (const array& lhs, const std::vector<T>& rhs) noexcept
			{
				return container_equality(lhs, rhs);
			}
			TOML_ASYMMETRICAL_EQUALITY_OPS(const array&, const std::vector<T>&, template <typename T>)
			array& flatten() &;
			array&& flatten() &&
			{
				return static_cast<toml::array&&>(static_cast<toml::array&>(*this).flatten());
			}

			template <typename Char>
			friend std::basic_ostream<Char>& operator << (std::basic_ostream<Char>&, const array&);
			// implemented in toml_default_formatter.h
	};
}
TOML_NAMESPACE_END

#endif //-----------------------------------------------------------  ↑ toml_array.h  ----------------------------------

#if 1  //------------------------------------------------------------------------------------  ↓ toml_table.h  ---------

TOML_IMPL_NAMESPACE_START
{
	template <bool IsConst>
	struct table_proxy_pair final
	{
		using value_type = std::conditional_t<IsConst, const node, node>;

		const std::string& first;
		value_type& second;
	};

	template <bool IsConst>
	class table_iterator final
	{
		private:
			friend class TOML_NAMESPACE::table;

			using proxy_type = table_proxy_pair<IsConst>;
			using raw_mutable_iterator = string_map<std::unique_ptr<node>>::iterator;
			using raw_const_iterator = string_map<std::unique_ptr<node>>::const_iterator;
			using raw_iterator = std::conditional_t<IsConst, raw_const_iterator, raw_mutable_iterator>;

			mutable raw_iterator raw_;
			mutable std::aligned_storage_t<sizeof(proxy_type), alignof(proxy_type)> proxy;
			mutable bool proxy_instantiated = false;

			[[nodiscard]]
			proxy_type* get_proxy() const noexcept
			{
				if (!proxy_instantiated)
				{
					auto p = new (&proxy) proxy_type{ raw_->first, *raw_->second.get() };
					proxy_instantiated = true;
					return p;
				}
				else
					return TOML_LAUNDER(reinterpret_cast<proxy_type*>(&proxy));
			}

			table_iterator(raw_mutable_iterator raw) noexcept
				: raw_{ raw }
			{}

			template <bool C = IsConst, typename = std::enable_if_t<C>>
			TOML_NODISCARD_CTOR
			table_iterator(raw_const_iterator raw) noexcept
				: raw_{ raw }
			{}

		public:

			table_iterator() noexcept = default;

			table_iterator(const table_iterator& other) noexcept
				: raw_{ other.raw_ }
			{}

			table_iterator& operator = (const table_iterator& rhs) noexcept
			{
				raw_ = rhs.raw_;
				proxy_instantiated = false;
				return *this;
			}

			using value_type = table_proxy_pair<IsConst>;
			using reference = value_type&;
			using pointer = value_type*;

			table_iterator& operator++() noexcept // ++pre
			{
				++raw_;
				proxy_instantiated = false;
				return *this;
			}

			table_iterator operator++(int) noexcept // post++
			{
				table_iterator out{ raw_ };
				++raw_;
				proxy_instantiated = false;
				return out;
			}

			table_iterator& operator--() noexcept // --pre
			{
				--raw_;
				proxy_instantiated = false;
				return *this;
			}

			table_iterator operator--(int) noexcept // post--
			{
				table_iterator out{ raw_ };
				--raw_;
				proxy_instantiated = false;
				return out;
			}

			[[nodiscard]]
			reference operator* () const noexcept
			{
				return *get_proxy();
			}

			[[nodiscard]]
			pointer operator -> () const noexcept
			{
				return get_proxy();
			}

			[[nodiscard]]
			friend bool operator == (const table_iterator& lhs, const table_iterator& rhs) noexcept
			{
				return lhs.raw_ == rhs.raw_;
			}

			[[nodiscard]]
			friend bool operator != (const table_iterator& lhs, const table_iterator& rhs) noexcept
			{
				return lhs.raw_ != rhs.raw_;
			}

			TOML_DISABLE_WARNINGS

			template <bool C = IsConst, typename = std::enable_if_t<!C>>
			operator table_iterator<true>() const noexcept
			{
				return table_iterator<true>{ raw_ };
			}

			TOML_ENABLE_WARNINGS
	};

	struct table_init_pair final
	{
		std::string key;
		std::unique_ptr<node> value;

		template <typename V>
		table_init_pair(std::string&& k, V&& v) noexcept
			: key{ std::move(k) },
			value{ make_node(std::forward<V>(v)) }
		{}

		template <typename V>
		table_init_pair(std::string_view k, V&& v) noexcept
			: key{ k },
			value{ make_node(std::forward<V>(v)) }
		{}

		template <typename V>
		table_init_pair(const char* k, V&& v) noexcept
			: key{ k },
			value{ make_node(std::forward<V>(v)) }
		{}

		#if TOML_WINDOWS_COMPAT

		template <typename V>
		table_init_pair(std::wstring&& k, V&& v) noexcept
			: key{ narrow(k) },
			value{ make_node(std::forward<V>(v)) }
		{}

		template <typename V>
		table_init_pair(std::wstring_view k, V&& v) noexcept
			: key{ narrow(k) },
			value{ make_node(std::forward<V>(v)) }
		{}

		template <typename V>
		table_init_pair(const wchar_t* k, V&& v) noexcept
			: key{ narrow(std::wstring_view{ k }) },
			value{ make_node(std::forward<V>(v)) }
		{}

		#endif
	};
}
TOML_IMPL_NAMESPACE_END

TOML_NAMESPACE_START
{
	using table_iterator = impl::table_iterator<false>;
	using const_table_iterator = impl::table_iterator<true>;

	class TOML_API table final
		: public node
	{
		private:
			friend class TOML_PARSER_TYPENAME;

			impl::string_map<std::unique_ptr<node>> map;
			bool inline_ = false;

			table(impl::table_init_pair*, size_t) noexcept;

		public:

			using iterator = table_iterator;
			using const_iterator = const_table_iterator;

			TOML_NODISCARD_CTOR
			table() noexcept;

			TOML_NODISCARD_CTOR
			table(const table&) noexcept;

			TOML_NODISCARD_CTOR
			table(table&& other) noexcept;
			table& operator= (const table&) noexcept;
			table& operator= (table&& rhs) noexcept;

			template <size_t N>
			TOML_NODISCARD_CTOR
			explicit table(impl::table_init_pair(&& arr)[N]) noexcept
				: table{ arr, N }
			{}

			[[nodiscard]] node_type type() const noexcept override;
			[[nodiscard]] bool is_table() const noexcept override;
			[[nodiscard]] bool is_array() const noexcept override;
			[[nodiscard]] bool is_value() const noexcept override;
			[[nodiscard]] table* as_table() noexcept override;
			[[nodiscard]] const table* as_table() const noexcept override;
			[[nodiscard]] bool is_homogeneous(node_type ntype) const noexcept override;
			[[nodiscard]] bool is_homogeneous(node_type ntype, node*& first_nonmatch) noexcept override;
			[[nodiscard]] bool is_homogeneous(node_type ntype, const node*& first_nonmatch) const noexcept override;
			template <typename ElemType = void>
			[[nodiscard]]
			bool is_homogeneous() const noexcept
			{
				using type = impl::unwrap_node<ElemType>;
				static_assert(
					std::is_void_v<type>
					|| ((impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>),
					"The template type argument of table::is_homogeneous() must be void or one of:"
					TOML_SA_UNWRAPPED_NODE_TYPE_LIST
				);
				return is_homogeneous(impl::node_type_of<type>);
			}

			[[nodiscard]] bool is_inline() const noexcept;
			void is_inline(bool val) noexcept;
			[[nodiscard]] node_view<node> operator[] (std::string_view key) noexcept;
			[[nodiscard]] node_view<const node> operator[] (std::string_view key) const noexcept;

			#if TOML_WINDOWS_COMPAT

			[[nodiscard]] node_view<node> operator[] (std::wstring_view key) noexcept;
			[[nodiscard]] node_view<const node> operator[] (std::wstring_view key) const noexcept;

			#endif // TOML_WINDOWS_COMPAT

			[[nodiscard]] iterator begin() noexcept;
			[[nodiscard]] const_iterator begin() const noexcept;
			[[nodiscard]] const_iterator cbegin() const noexcept;
			[[nodiscard]] iterator end() noexcept;
			[[nodiscard]] const_iterator end() const noexcept;
			[[nodiscard]] const_iterator cend() const noexcept;
			[[nodiscard]] bool empty() const noexcept;
			[[nodiscard]] size_t size() const noexcept;
			void clear() noexcept;

			template <typename KeyType, typename ValueType, typename = std::enable_if_t<
				std::is_convertible_v<KeyType&&, std::string_view>
				|| impl::is_wide_string<KeyType>
			>>
			std::pair<iterator, bool> insert(KeyType&& key, ValueType&& val) noexcept
			{
				static_assert(
					!impl::is_wide_string<KeyType> || TOML_WINDOWS_COMPAT,
					"Insertion using wide-character keys is only supported on Windows with TOML_WINDOWS_COMPAT enabled."
				);

				if constexpr (impl::is_node_view<ValueType>)
				{
					if (!val)
						return { end(), false };
				}

				if constexpr (impl::is_wide_string<KeyType>)
				{
					#if TOML_WINDOWS_COMPAT
					return insert(impl::narrow(std::forward<KeyType>(key)), std::forward<ValueType>(val));
					#else
					static_assert(impl::dependent_false<KeyType>, "Evaluated unreachable branch!");
					#endif
				}
				else
				{
					auto ipos = map.lower_bound(key);
					if (ipos == map.end() || ipos->first != key)
					{
						ipos = map.emplace_hint(ipos, std::forward<KeyType>(key), impl::make_node(std::forward<ValueType>(val)));
						return { iterator{ ipos }, true };
					}
					return { iterator{ ipos }, false };
				}
			}

			template <typename Iter, typename = std::enable_if_t<
				!std::is_convertible_v<Iter, std::string_view>
				&& !impl::is_wide_string<Iter>
			>>
			void insert(Iter first, Iter last) noexcept
			{
				if (first == last)
					return;
				for (auto it = first; it != last; it++)
				{
					if constexpr (std::is_rvalue_reference_v<decltype(*it)>)
						insert(std::move((*it).first), std::move((*it).second));
					else
						insert((*it).first, (*it).second);
				}
			}

			template <typename KeyType, typename ValueType>
			std::pair<iterator, bool> insert_or_assign(KeyType&& key, ValueType&& val) noexcept
			{
				static_assert(
					!impl::is_wide_string<KeyType> || TOML_WINDOWS_COMPAT,
					"Insertion using wide-character keys is only supported on Windows with TOML_WINDOWS_COMPAT enabled."
				);

				if constexpr (impl::is_node_view<ValueType>)
				{
					if (!val)
						return { end(), false };
				}

				if constexpr (impl::is_wide_string<KeyType>)
				{
					#if TOML_WINDOWS_COMPAT
					return insert_or_assign(impl::narrow(std::forward<KeyType>(key)), std::forward<ValueType>(val));
					#else
					static_assert(impl::dependent_false<KeyType>, "Evaluated unreachable branch!");
					#endif
				}
				else
				{
					auto ipos = map.lower_bound(key);
					if (ipos == map.end() || ipos->first != key)
					{
						ipos = map.emplace_hint(ipos, std::forward<KeyType>(key), impl::make_node(std::forward<ValueType>(val)));
						return { iterator{ ipos }, true };
					}
					else
					{
						(*ipos).second.reset(impl::make_node(std::forward<ValueType>(val)));
						return { iterator{ ipos }, false };
					}
				}
			}

			template <typename ValueType, typename KeyType, typename... ValueArgs>
			std::pair<iterator, bool> emplace(KeyType&& key, ValueArgs&&... args) noexcept
			{
				static_assert(
					!impl::is_wide_string<KeyType> || TOML_WINDOWS_COMPAT,
					"Emplacement using wide-character keys is only supported on Windows with TOML_WINDOWS_COMPAT enabled."
				);

				if constexpr (impl::is_wide_string<KeyType>)
				{
					#if TOML_WINDOWS_COMPAT
					return emplace<ValueType>(impl::narrow(std::forward<KeyType>(key)), std::forward<ValueArgs>(args)...);
					#else
					static_assert(impl::dependent_false<KeyType>, "Evaluated unreachable branch!");
					#endif
				}
				else
				{
					using type = impl::unwrap_node<ValueType>;
					static_assert(
						(impl::is_native<type> || impl::is_one_of<type, table, array>) && !impl::is_cvref<type>,
						"The emplacement type argument of table::emplace() must be one of:"
						TOML_SA_UNWRAPPED_NODE_TYPE_LIST
					);

					auto ipos = map.lower_bound(key);
					if (ipos == map.end() || ipos->first != key)
					{
						ipos = map.emplace_hint(
							ipos,
							std::forward<KeyType>(key),
							new impl::wrap_node<type>{ std::forward<ValueArgs>(args)... }
						);
						return { iterator{ ipos }, true };
					}
					return { iterator{ ipos }, false };
				}
			}

			iterator erase(iterator pos) noexcept;
			iterator erase(const_iterator pos) noexcept;
			iterator erase(const_iterator first, const_iterator last) noexcept;
			bool erase(std::string_view key) noexcept;

			#if TOML_WINDOWS_COMPAT

			bool erase(std::wstring_view key) noexcept;

			#endif

		private:

			template <typename Map, typename Key>
			[[nodiscard]]
			static auto do_get(Map& vals, const Key& key) noexcept
				-> std::conditional_t<std::is_const_v<Map>, const node*, node*>
			{
				static_assert(
					!impl::is_wide_string<Key> || TOML_WINDOWS_COMPAT,
					"Retrieval using wide-character keys is only supported on Windows with TOML_WINDOWS_COMPAT enabled."
				);

				if constexpr (impl::is_wide_string<Key>)
				{
					#if TOML_WINDOWS_COMPAT
					return do_get(vals, impl::narrow(key));
					#else
					static_assert(impl::dependent_false<Key>, "Evaluated unreachable branch!");
					#endif
				}
				else
				{
					if (auto it = vals.find(key); it != vals.end())
						return { it->second.get() };
					return {};
				}
			}

			template <typename T, typename Map, typename Key>
			[[nodiscard]]
			static auto do_get_as(Map& vals, const Key& key) noexcept
			{
				const auto node = do_get(vals, key);
				return node ? node->template as<T>() : nullptr;
			}

			template <typename Map, typename Key>
			[[nodiscard]]
			TOML_ALWAYS_INLINE
			static bool do_contains(Map& vals, const Key& key) noexcept
			{
				return do_get(vals, key) != nullptr;
			}

		public:

			[[nodiscard]] node* get(std::string_view key) noexcept;
			[[nodiscard]] const node* get(std::string_view key) const noexcept;
			[[nodiscard]] iterator find(std::string_view key) noexcept;
			[[nodiscard]] const_iterator find(std::string_view key) const noexcept;
			[[nodiscard]] bool contains(std::string_view key) const noexcept;

			#if TOML_WINDOWS_COMPAT

			[[nodiscard]] node* get(std::wstring_view key) noexcept;
			[[nodiscard]] const node* get(std::wstring_view key) const noexcept;
			[[nodiscard]] iterator find(std::wstring_view key) noexcept;
			[[nodiscard]] const_iterator find(std::wstring_view key) const noexcept;
			[[nodiscard]] bool contains(std::wstring_view key) const noexcept;

			#endif // TOML_WINDOWS_COMPAT

			template <typename ValueType>
			[[nodiscard]]
			impl::wrap_node<ValueType>* get_as(std::string_view key) noexcept
			{
				return do_get_as<ValueType>(map, key);
			}

			template <typename ValueType>
			[[nodiscard]]
			const impl::wrap_node<ValueType>* get_as(std::string_view key) const noexcept
			{
				return do_get_as<ValueType>(map, key);
			}

			#if TOML_WINDOWS_COMPAT

			template <typename ValueType>
			[[nodiscard]]
			impl::wrap_node<ValueType>* get_as(std::wstring_view key) noexcept
			{
				return get_as<ValueType>(impl::narrow(key));
			}

			template <typename ValueType>
			[[nodiscard]]
			const impl::wrap_node<ValueType>* get_as(std::wstring_view key) const noexcept
			{
				return get_as<ValueType>(impl::narrow(key));
			}

			#endif // TOML_WINDOWS_COMPAT

			friend bool operator == (const table& lhs, const table& rhs) noexcept;
			friend bool operator != (const table& lhs, const table& rhs) noexcept;

			template <typename Char>
			friend std::basic_ostream<Char>& operator << (std::basic_ostream<Char>&, const table&);
			// implemented in toml_default_formatter.h
	};

	#ifndef DOXYGEN

	//template <typename T>
	//inline std::vector<T> node::select_exact() const noexcept
	//{
	//	using namespace impl;

	//	static_assert(
	//		!is_wide_string<T> || TOML_WINDOWS_COMPAT,
	//		"Retrieving values as wide-character strings with node::select_exact() is only "
	//		"supported on Windows with TOML_WINDOWS_COMPAT enabled."
	//	);

	//	static_assert(
	//		(is_native<T> || can_represent_native<T>) && !is_cvref<T>,
	//		TOML_SA_VALUE_EXACT_FUNC_MESSAGE("return type of node::select_exact()")
	//	);
	//}

	//template <typename T>
	//inline std::vector<T> node::select() const noexcept
	//{
	//	using namespace impl;

	//	static_assert(
	//		!is_wide_string<T> || TOML_WINDOWS_COMPAT,
	//		"Retrieving values as wide-character strings with node::select() is only "
	//		"supported on Windows with TOML_WINDOWS_COMPAT enabled."
	//	);
	//	static_assert(
	//		(is_native<T> || can_represent_native<T> || can_partially_represent_native<T>) && !is_cvref<T>,
	//		TOML_SA_VALUE_FUNC_MESSAGE("return type of node::select()")
	//	);
	//}

	#endif // !DOXYGEN
}
TOML_NAMESPACE_END

#endif //------------------------------------------------------------------------------------  ↑ toml_table.h  ---------

#if 1  //-------  ↓ toml_node_view.h  ----------------------------------------------------------------------------------

TOML_PUSH_WARNINGS
TOML_DISABLE_ARITHMETIC_WARNINGS

TOML_NAMESPACE_START
{
	template <typename ViewedType>
	class TOML_API TOML_TRIVIAL_ABI node_view
	{
		static_assert(
			impl::is_one_of<ViewedType, toml::node, const toml::node>,
			"A toml::node_view<> must wrap toml::node or const toml::node."
		);

		public:
			using viewed_type = ViewedType;

		private:
			template <typename T> friend class TOML_NAMESPACE::node_view;

			mutable viewed_type* node_ = nullptr;

			template <typename Func>
			static constexpr bool visit_is_nothrow
				= noexcept(std::declval<viewed_type*>()->visit(std::declval<Func&&>()));

		public:

			TOML_NODISCARD_CTOR
			node_view() noexcept = default;

			TOML_NODISCARD_CTOR
			explicit node_view(viewed_type* node) noexcept
				: node_{ node }
			{}

			TOML_NODISCARD_CTOR
			explicit node_view(viewed_type& node) noexcept
				: node_{ &node }
			{}

			TOML_NODISCARD_CTOR
			node_view(const node_view&) noexcept = default;

			node_view& operator= (const node_view&) & noexcept = default;

			TOML_NODISCARD_CTOR
			node_view(node_view&&) noexcept = default;

			node_view& operator= (node_view&&) & noexcept = default;
			[[nodiscard]] explicit operator bool() const noexcept { return node_ != nullptr; }
			[[nodiscard]] viewed_type* node() const noexcept { return node_; }

			[[nodiscard, deprecated("use node_view::node() instead")]]
			viewed_type* get() const noexcept { return node_; }

			[[nodiscard]] node_type type() const noexcept { return node_ ? node_->type() : node_type::none; }
			[[nodiscard]] bool is_table() const noexcept { return node_ && node_->is_table(); }
			[[nodiscard]] bool is_array() const noexcept { return node_ && node_->is_array(); }
			[[nodiscard]] bool is_value() const noexcept { return node_ && node_->is_value(); }
			[[nodiscard]] bool is_string() const noexcept { return node_ && node_->is_string(); }
			[[nodiscard]] bool is_integer() const noexcept { return node_ && node_->is_integer(); }
			[[nodiscard]] bool is_floating_point() const noexcept { return node_ && node_->is_floating_point(); }
			[[nodiscard]] bool is_number() const noexcept { return node_ && node_->is_number(); }
			[[nodiscard]] bool is_boolean() const noexcept { return node_ && node_->is_boolean(); }
			[[nodiscard]] bool is_date() const noexcept { return node_ && node_->is_date(); }
			[[nodiscard]] bool is_time() const noexcept { return node_ && node_->is_time(); }
			[[nodiscard]] bool is_date_time() const noexcept { return node_ && node_->is_date_time(); }
			[[nodiscard]] bool is_array_of_tables() const noexcept { return node_ && node_->is_array_of_tables(); }

			template <typename T>
			[[nodiscard]]
			bool is() const noexcept
			{
				return node_ ? node_->template is<T>() : false;
			}

			template <typename T>
			[[nodiscard]]
			auto as() const noexcept
			{
				return node_ ? node_->template as<T>() : nullptr;
			}

			[[nodiscard]] auto as_table() const noexcept { return as<table>(); }
			[[nodiscard]] auto as_array() const noexcept { return as<array>(); }
			[[nodiscard]] auto as_string() const noexcept { return as<std::string>(); }
			[[nodiscard]] auto as_integer() const noexcept { return as<int64_t>(); }
			[[nodiscard]] auto as_floating_point() const noexcept { return as<double>(); }
			[[nodiscard]] auto as_boolean() const noexcept { return as<bool>(); }
			[[nodiscard]] auto as_date() const noexcept { return as<date>(); }
			[[nodiscard]] auto as_time() const noexcept { return as<time>(); }
			[[nodiscard]] auto as_date_time() const noexcept { return as<date_time>(); }
			[[nodiscard]]
			bool is_homogeneous(node_type ntype, viewed_type*& first_nonmatch) const noexcept
			{
				if (!node_)
				{
					first_nonmatch = {};
					return false;
				}
				return node_->is_homogeneous(ntype, first_nonmatch);
			}

			[[nodiscard]]
			bool is_homogeneous(node_type ntype) const noexcept
			{
				return node_ ? node_->is_homogeneous(ntype) : false;
			}

			template <typename ElemType = void>
			[[nodiscard]]
			bool is_homogeneous() const noexcept
			{
				return node_ ? node_->template is_homogeneous<impl::unwrap_node<ElemType>>() : false;
			}

			template <typename T>
			[[nodiscard]]
			optional<T> value_exact() const noexcept
			{
				if (node_)
					return node_->template value_exact<T>();
				return {};
			}

			TOML_PUSH_WARNINGS
			TOML_DISABLE_INIT_WARNINGS

			template <typename T>
			[[nodiscard]]
			optional<T> value() const noexcept
			{
				if (node_)
					return node_->template value<T>();
				return {};
			}

			TOML_POP_WARNINGS

			template <typename T>
			[[nodiscard]]
			auto value_or(T&& default_value) const noexcept
			{
				using namespace ::toml::impl;

				static_assert(
					!is_wide_string<T> || TOML_WINDOWS_COMPAT,
					"Retrieving values as wide-character strings is only "
					"supported on Windows with TOML_WINDOWS_COMPAT enabled."
				);

				if constexpr (is_wide_string<T>)
				{
					#if TOML_WINDOWS_COMPAT

					if (node_)
						return node_->value_or(std::forward<T>(default_value));
					return std::wstring{ std::forward<T>(default_value) };

					#else

					static_assert(impl::dependent_false<T>, "Evaluated unreachable branch!");

					#endif
				}
				else
				{
					using value_type = std::conditional_t<
						std::is_pointer_v<std::decay_t<T>>,
						std::add_pointer_t<std::add_const_t<std::remove_pointer_t<std::decay_t<T>>>>,
						std::decay_t<T>
					>;

					if (node_)
						return node_->value_or(std::forward<T>(default_value));
					if constexpr (std::is_pointer_v<value_type>)
						return value_type{ default_value };
					else
						return std::forward<T>(default_value);
				}
			}

			template <typename Func>
			decltype(auto) visit(Func&& visitor) const
				noexcept(visit_is_nothrow<Func&&>)
			{
				using return_type = decltype(node_->visit(std::forward<Func>(visitor)));
				if (node_)
					return node_->visit(std::forward<Func>(visitor));
				if constexpr (!std::is_void_v<return_type>)
					return return_type{};
			}

			template <typename T>
			[[nodiscard]]
			decltype(auto) ref() const noexcept
			{
				TOML_ASSERT(
					node_
					&& "toml::node_view::ref() called on a node_view that did not reference a node"
				);
				return node_->template ref<impl::unwrap_node<T>>();
			}

			[[nodiscard]]
			friend bool operator == (const node_view& lhs, const table& rhs) noexcept
			{
				if (lhs.node_ == &rhs)
					return true;
				const auto tbl = lhs.as<table>();
				return tbl && *tbl == rhs;
			}
			TOML_ASYMMETRICAL_EQUALITY_OPS(const node_view&, const table&, )
			[[nodiscard]]
			friend bool operator == (const node_view& lhs, const array& rhs) noexcept
			{
				if (lhs.node_ == &rhs)
					return true;
				const auto arr = lhs.as<array>();
				return arr && *arr == rhs;
			}
			TOML_ASYMMETRICAL_EQUALITY_OPS(const node_view&, const array&, )

			template <typename T>
			[[nodiscard]]
			friend bool operator == (const node_view& lhs, const toml::value<T>& rhs) noexcept
			{
				if (lhs.node_ == &rhs)
					return true;
				const auto val = lhs.as<T>();
				return val && *val == rhs;
			}
			TOML_ASYMMETRICAL_EQUALITY_OPS(const node_view&, const toml::value<T>&, template <typename T>)

			template <typename T, typename = std::enable_if_t<
				impl::is_native<T>
				|| impl::is_losslessly_convertible_to_native<T>
			>>
			[[nodiscard]]
			friend bool operator == (const node_view& lhs, const T& rhs) noexcept
			{
				static_assert(
					!impl::is_wide_string<T> || TOML_WINDOWS_COMPAT,
					"Comparison with wide-character strings is only "
					"supported on Windows with TOML_WINDOWS_COMPAT enabled."
				);

				if constexpr (impl::is_wide_string<T>)
				{
					#if TOML_WINDOWS_COMPAT
					return lhs == impl::narrow(rhs);
					#else
					static_assert(impl::dependent_false<T>, "Evaluated unreachable branch!");
					#endif
				}
				else
				{
					const auto val = lhs.as<impl::native_type_of<T>>();
					return val && *val == rhs;
				}
			}
			TOML_ASYMMETRICAL_EQUALITY_OPS(
				const node_view&,
				const T&,
				template <typename T, typename = std::enable_if_t<
					impl::is_native<T>
					|| impl::is_losslessly_convertible_to_native<T>
				>>
			)

			template <typename T>
			[[nodiscard]]
			friend bool operator == (const node_view& lhs, const std::initializer_list<T>& rhs) noexcept
			{
				const auto arr = lhs.as<array>();
				return arr && *arr == rhs;
			}
			TOML_ASYMMETRICAL_EQUALITY_OPS(const node_view&, const std::initializer_list<T>&, template <typename T>)

			template <typename T>
			[[nodiscard]]
			friend bool operator == (const node_view& lhs, const std::vector<T>& rhs) noexcept
			{
				const auto arr = lhs.as<array>();
				return arr && *arr == rhs;
			}
			TOML_ASYMMETRICAL_EQUALITY_OPS(const node_view&, const std::vector<T>&, template <typename T>)

			[[nodiscard]]
			node_view operator[] (std::string_view key) const noexcept
			{
				if (auto tbl = this->as_table())
					return node_view{ tbl->get(key) };
				return node_view{ nullptr };
			}

			#if TOML_WINDOWS_COMPAT

			[[nodiscard]]
			node_view operator[] (std::wstring_view key) const noexcept
			{
				if (auto tbl = this->as_table())
					return node_view{ tbl->get(key) };
				return node_view{ nullptr };
			}

			#endif // TOML_WINDOWS_COMPAT

			[[nodiscard]]
			node_view operator[] (size_t index) const noexcept
			{
				if (auto arr = this->as_array())
					return node_view{ arr->get(index) };
				return node_view{ nullptr };
			}

			template <typename Char, typename T>
			friend std::basic_ostream<Char>& operator << (std::basic_ostream<Char>&, const node_view<T>&);
	};
	template <typename T> node_view(const value<T>&)	-> node_view<const node>;
	node_view(const table&)								-> node_view<const node>;
	node_view(const array&)								-> node_view<const node>;
	template <typename T> node_view(value<T>&)			-> node_view<node>;
	node_view(table&)									-> node_view<node>;
	node_view(array&)									-> node_view<node>;
	template <typename T> node_view(const T*)			-> node_view<const node>;
	template <typename T> node_view(T*)					-> node_view<node>;

	template <typename Char, typename T>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& os, const node_view<T>& nv)
	{
		if (nv.node_)
		{
			nv.node_->visit([&os](const auto& n)
			{
				os << n;
			});
		}
		return os;
	}

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY

	extern template class TOML_API node_view<node>;
	extern template class TOML_API node_view<const node>;

	extern template TOML_API std::ostream& operator << (std::ostream&, const node_view<node>&);
	extern template TOML_API std::ostream& operator << (std::ostream&, const node_view<const node>&);

	#define TOML_EXTERN(name, T)																	\
		extern template TOML_API optional<T>		node_view<node>::name<T>() const noexcept;		\
		extern template TOML_API optional<T>		node_view<const node>::name<T>() const noexcept
	TOML_EXTERN(value_exact, std::string_view);
	TOML_EXTERN(value_exact, std::string);
	TOML_EXTERN(value_exact, const char*);
	TOML_EXTERN(value_exact, int64_t);
	TOML_EXTERN(value_exact, double);
	TOML_EXTERN(value_exact, date);
	TOML_EXTERN(value_exact, time);
	TOML_EXTERN(value_exact, date_time);
	TOML_EXTERN(value_exact, bool);
	TOML_EXTERN(value, std::string_view);
	TOML_EXTERN(value, std::string);
	TOML_EXTERN(value, const char*);
	TOML_EXTERN(value, signed char);
	TOML_EXTERN(value, signed short);
	TOML_EXTERN(value, signed int);
	TOML_EXTERN(value, signed long);
	TOML_EXTERN(value, signed long long);
	TOML_EXTERN(value, unsigned char);
	TOML_EXTERN(value, unsigned short);
	TOML_EXTERN(value, unsigned int);
	TOML_EXTERN(value, unsigned long);
	TOML_EXTERN(value, unsigned long long);
	TOML_EXTERN(value, double);
	TOML_EXTERN(value, float);
	TOML_EXTERN(value, date);
	TOML_EXTERN(value, time);
	TOML_EXTERN(value, date_time);
	TOML_EXTERN(value, bool);
	#ifdef __cpp_lib_char8_t
	TOML_EXTERN(value_exact, std::u8string_view);
	TOML_EXTERN(value_exact, std::u8string);
	TOML_EXTERN(value_exact, const char8_t*);
	TOML_EXTERN(value, std::u8string_view);
	TOML_EXTERN(value, std::u8string);
	TOML_EXTERN(value, const char8_t*);
	#endif
	#if TOML_WINDOWS_COMPAT
	TOML_EXTERN(value_exact, std::wstring);
	TOML_EXTERN(value, std::wstring);
	#endif
	#undef TOML_EXTERN

	#endif // !TOML_HEADER_ONLY
}
TOML_NAMESPACE_END

TOML_POP_WARNINGS // TOML_DISABLE_ARITHMETIC_WARNINGS

#endif //-------  ↑ toml_node_view.h  ----------------------------------------------------------------------------------

#if 1  //-----------------------------------  ↓ toml_utf8.h  -----------------------------------------------------------

#ifndef DOXYGEN

TOML_IMPL_NAMESPACE_START
{
	template <typename... T>
	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_match(char32_t codepoint, T... vals) noexcept
	{
		static_assert((std::is_same_v<char32_t, T> && ...));
		return ((codepoint == vals) || ...);
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_ascii_whitespace(char32_t codepoint) noexcept
	{
		return codepoint == U'\t' || codepoint == U' ';
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_unicode_whitespace(char32_t codepoint) noexcept
	{
		// see: https://en.wikipedia.org/wiki/Whitespace_character#Unicode
		// (characters that don't say "is a line-break")

		return codepoint == U'\u00A0' // no-break space
			|| codepoint == U'\u1680' // ogham space mark
			|| (codepoint >= U'\u2000' && codepoint <= U'\u200A') // em quad -> hair space
			|| codepoint == U'\u202F' // narrow no-break space
			|| codepoint == U'\u205F' // medium mathematical space
			|| codepoint == U'\u3000' // ideographic space
		;
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_whitespace(char32_t codepoint) noexcept
	{
		return is_ascii_whitespace(codepoint) || is_unicode_whitespace(codepoint);
	}

	template <bool IncludeCarriageReturn = true>
	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_ascii_line_break(char32_t codepoint) noexcept
	{
		constexpr auto low_range_end = IncludeCarriageReturn ? U'\r' : U'\f';
		return (codepoint >= U'\n' && codepoint <= low_range_end);
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_unicode_line_break(char32_t codepoint) noexcept
	{
		// see https://en.wikipedia.org/wiki/Whitespace_character#Unicode
		// (characters that say "is a line-break")

		return codepoint == U'\u0085' // next line
			|| codepoint == U'\u2028' // line separator
			|| codepoint == U'\u2029' // paragraph separator
		;
	}

	template <bool IncludeCarriageReturn = true>
	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_line_break(char32_t codepoint) noexcept
	{
		return is_ascii_line_break<IncludeCarriageReturn>(codepoint) || is_unicode_line_break(codepoint);
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_string_delimiter(char32_t codepoint) noexcept
	{
		return codepoint == U'"' || codepoint == U'\'';
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_ascii_letter(char32_t codepoint) noexcept
	{
		return (codepoint >= U'a' && codepoint <= U'z')
			|| (codepoint >= U'A' && codepoint <= U'Z');
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_binary_digit(char32_t codepoint) noexcept
	{
		return codepoint == U'0' || codepoint == U'1';
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_octal_digit(char32_t codepoint) noexcept
	{
		return (codepoint >= U'0' && codepoint <= U'7');
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_decimal_digit(char32_t codepoint) noexcept
	{
		return (codepoint >= U'0' && codepoint <= U'9');
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_hexadecimal_digit(char32_t c) noexcept
	{
		return U'0' <= c && c <= U'f' && (1ull << (static_cast<uint_least64_t>(c) - 0x30u)) & 0x7E0000007E03FFull;
	}

	template <typename T>
	[[nodiscard]]
	TOML_ATTR(const)
	constexpr std::uint_least32_t hex_to_dec(const T codepoint) noexcept
	{
		if constexpr (std::is_same_v<remove_cvref_t<T>, std::uint_least32_t>)
			return codepoint >= 0x41u // >= 'A'
				? 10u + (codepoint | 0x20u) - 0x61u // - 'a'
				: codepoint - 0x30u // - '0'
			;
		else
			return hex_to_dec(static_cast<std::uint_least32_t>(codepoint));
	}

	#if TOML_LANG_UNRELEASED // toml/issues/687 (unicode bare keys)

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_unicode_letter(char32_t c) noexcept
	{
		if (U'\xAA' > c || c > U'\U0003134A')
			return false;

		const auto child_index_0 = (static_cast<uint_least64_t>(c) - 0xAAull) / 0xC4Bull;
		if ((1ull << child_index_0) & 0x26180C0000ull)
			return false;
		if ((1ull << child_index_0) & 0x8A7FFC004001CFA0ull)
			return true;
		switch (child_index_0)
		{
			case 0x00: // [0] 00AA - 0CF4
			{
				if (c > U'\u0CF2')
					return false;
				TOML_ASSUME(U'\xAA' <= c);

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0xFFFFDFFFFFC10801u,	0xFFFFFFFFFFFFDFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0x07C000FFF0FFFFFFu,	0x0000000000000014u,	0x0000000000000000u,	0xFEFFFFF5D02F37C0u,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFEFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFF00FFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFC09FFFFFFFFFBFu,	0x000000007FFFFFFFu,
					0xFFFFFFC000000000u,	0xFFC00000000001E1u,	0x00000001FFFFFFFFu,	0xFFFFFFFFFFFFFFB0u,
					0x18000BFFFFFFFFFFu,	0xFFFFFF4000270030u,	0xFFFFFFF80000003Fu,	0x0FFFFFFFFFFFFFFFu,
					0xFFFFFFFF00000080u,	0x44010FFFFFC10C01u,	0xFFC07FFFFFC00000u,	0xFFC0000000000001u,
					0x000000003FFFF7FFu,	0xFFFFFFFFFC000000u,	0x00FFC0400008FFFFu,	0x7FFFFE67F87FFF80u,
					0x00EC00100008F17Fu,	0x7FFFFE61F80400C0u,	0x001780000000DB7Fu,	0x7FFFFEEFF8000700u,
					0x00C000400008FB7Fu,	0x7FFFFE67F8008000u,	0x00EC00000008FB7Fu,	0xC6358F71FA000080u,
					0x000000400000FFF1u,	0x7FFFFF77F8000000u,	0x00C1C0000008FFFFu,	0x7FFFFF77F8400000u,
					0x00D000000008FBFFu,	0x0000000000000180u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0xAAull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0xAAull) % 0x40ull));
				// 1922 codepoints from 124 ranges (spanning a search area of 3147)
			}
			case 0x01: // [1] 0CF5 - 193F
			{
				if (U'\u0D04' > c || c > U'\u191E')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x027FFFFFFFFFDDFFu,	0x0FC0000038070400u,	0xF2FFBFFFFFC7FFFEu,	0xE000000000000007u,
					0xF000DFFFFFFFFFFFu,	0x6000000000000007u,	0xF200DFFAFFFFFF7Du,	0x100000000F000005u,
					0xF000000000000000u,	0x000001FFFFFFFFEFu,	0x00000000000001F0u,	0xF000000000000000u,
					0x0800007FFFFFFFFFu,	0x3FFE1C0623C3F000u,	0xFFFFFFFFF0000400u,	0xFF7FFFFFFFFFF20Bu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFF3D7F3DFu,	0xD7F3DFFFFFFFF3DFu,	0xFFFFFFFFFFF7FFF3u,
					0xFFFFFFFFFFF3DFFFu,	0xF0000000007FFFFFu,	0xFFFFFFFFF0000FFFu,	0xE3F3FFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xEFFFF9FFFFFFFFFFu,	0xFFFFFFFFF07FFFFFu,	0xF01FE07FFFFFFFFFu,
					0xF0003FFFF0003DFFu,	0xF0001DFFF0003FFFu,	0x0000FFFFFFFFFFFFu,	0x0000000001080000u,
					0xFFFFFFFFF0000000u,	0xF01FFFFFFFFFFFFFu,	0xFFFFF05FFFFFFFF9u,	0xF003FFFFFFFFFFFFu,
					0x0000000007FFFFFFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0xD04ull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0xD04ull) % 0x40ull));
				// 2239 codepoints from 83 ranges (spanning a search area of 3147)
			}
			case 0x02: // [2] 1940 - 258A
			{
				if (U'\u1950' > c || c > U'\u2184')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0xFFFF001F3FFFFFFFu,	0x03FFFFFF0FFFFFFFu,	0xFFFF000000000000u,	0xFFFFFFFFFFFF007Fu,
					0x000000000000001Fu,	0x0000000000800000u,	0xFFE0000000000000u,	0x0FE0000FFFFFFFFFu,
					0xFFF8000000000000u,	0xFFFFFC00C001FFFFu,	0xFFFF0000003FFFFFu,	0xE0000000000FFFFFu,
					0x01FF3FFFFFFFFC00u,	0x0000E7FFFFFFFFFFu,	0xFFFF046FDE000000u,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0x0000FFFFFFFFFFFFu,	0xFFFF000000000000u,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0x3F3FFFFFFFFF3F3Fu,
					0xFFFF3FFFFFFFAAFFu,	0x1FDC5FDFFFFFFFFFu,	0x00001FDC1FFF0FCFu,	0x0000000000000000u,
					0x0000800200000000u,	0x0000000000001FFFu,	0xFC84000000000000u,	0x43E0F3FFBD503E2Fu,
					0x0018000000000000u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x1950ull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0x1950ull) % 0x40ull));
				// 1184 codepoints from 59 ranges (spanning a search area of 3147)
			}
			case 0x03: // [3] 258B - 31D5
			{
				if (U'\u2C00' > c || c > U'\u31BF')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0xFFFF7FFFFFFFFFFFu,	0xFFFFFFFF7FFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0x000C781FFFFFFFFFu,
					0xFFFF20BFFFFFFFFFu,	0x000080FFFFFFFFFFu,	0x7F7F7F7F007FFFFFu,	0x000000007F7F7F7Fu,
					0x0000800000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x183E000000000060u,	0xFFFFFFFFFFFFFFFEu,	0xFFFFFFFEE07FFFFFu,	0xF7FFFFFFFFFFFFFFu,
					0xFFFEFFFFFFFFFFE0u,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFF00007FFFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x2C00ull) / 0x40ull]
					& (0x1ull << (static_cast<uint_least64_t>(c) % 0x40ull));
				// 771 codepoints from 30 ranges (spanning a search area of 3147)
			}
			case 0x04: return (U'\u31F0' <= c && c <= U'\u31FF') || U'\u3400' <= c;
			case 0x06: return c <= U'\u4DBF' || U'\u4E00' <= c;
			case 0x0C: return c <= U'\u9FFC' || U'\uA000' <= c;
			case 0x0D: // [13] A079 - ACC3
			{
				TOML_ASSUME(U'\uA079' <= c && c <= U'\uACC3');

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0x00000000000FFFFFu,	0xFFFFFFFFFF800000u,	0xFFFFFFFFFFFFFF9Fu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0x0006007FFF8FFFFFu,	0x003FFFFFFFFFFF80u,
					0xFFFFFF9FFFFFFFC0u,	0x00001FFFFFFFFFFFu,	0xFFFFFE7FC0000000u,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFCFFFFu,	0xF00000000003FE7Fu,	0x000003FFFFFBDDFFu,	0x07FFFFFFFFFFFF80u,
					0x07FFFFFFFFFFFE00u,	0x7E00000000000000u,	0xFF801FFFFFFE0034u,	0xFFFFFF8000003FFFu,
					0x03FFFFFFFFFFF80Fu,	0x007FEF8000400000u,	0x0000FFFFFFFFFFBEu,	0x3FFFFF800007FB80u,
					0x317FFFFFFFFFFFE2u,	0x0E03FF9C0000029Fu,	0xFFBFBF803F3F3F00u,	0xFF81FFFBFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0x000003FFFFFFFFFFu,	0xFFFFFFFFFFFFFF80u,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0x00000000000007FFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0xA079ull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0xA079ull) % 0x40ull));
				// 2554 codepoints from 52 ranges (spanning a search area of 3147)
			}
			case 0x11: return c <= U'\uD7A3' || (U'\uD7B0' <= c && c <= U'\uD7C6') || (U'\uD7CB' <= c && c <= U'\uD7FB');
			case 0x14: // [20] F686 - 102D0
			{
				if (U'\uF900' > c)
					return false;
				TOML_ASSUME(c <= U'\U000102D0');

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFF3FFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0x0000000003FFFFFFu,
					0x5F7FFDFFA0F8007Fu,	0xFFFFFFFFFFFFFFDBu,	0x0003FFFFFFFFFFFFu,	0xFFFFFFFFFFF80000u,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0x3FFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFF0000u,	0xFFFFFFFFFFFCFFFFu,	0x0FFF0000000000FFu,
					0x0000000000000000u,	0xFFDF000000000000u,	0xFFFFFFFFFFFFFFFFu,	0x1FFFFFFFFFFFFFFFu,
					0x07FFFFFE00000000u,	0xFFFFFFC007FFFFFEu,	0x7FFFFFFFFFFFFFFFu,	0x000000001CFCFCFCu,
					0xB7FFFF7FFFFFEFFFu,	0x000000003FFF3FFFu,	0xFFFFFFFFFFFFFFFFu,	0x07FFFFFFFFFFFFFFu,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0xFFFFFFFF1FFFFFFFu,	0x000000000001FFFFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0xF900ull) / 0x40ull]
					& (0x1ull << (static_cast<uint_least64_t>(c) % 0x40ull));
				// 1710 codepoints from 34 ranges (spanning a search area of 3147)
			}
			case 0x15: // [21] 102D1 - 10F1B
			{
				if (U'\U00010300' > c)
					return false;
				TOML_ASSUME(c <= U'\U00010F1B');

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0xFFFFE000FFFFFFFFu,	0x003FFFFFFFFF03FDu,	0xFFFFFFFF3FFFFFFFu,	0x000000000000FF0Fu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFF00003FFFFFFFu,	0x0FFFFFFFFF0FFFFFu,
					0xFFFF00FFFFFFFFFFu,	0x0000000FFFFFFFFFu,	0x0000000000000000u,	0x0000000000000000u,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0x007FFFFFFFFFFFFFu,	0x000000FF003FFFFFu,	0x0000000000000000u,	0x0000000000000000u,
					0x91BFFFFFFFFFFD3Fu,	0x007FFFFF003FFFFFu,	0x000000007FFFFFFFu,	0x0037FFFF00000000u,
					0x03FFFFFF003FFFFFu,	0x0000000000000000u,	0xC0FFFFFFFFFFFFFFu,	0x0000000000000000u,
					0x003FFFFFFEEF0001u,	0x1FFFFFFF00000000u,	0x000000001FFFFFFFu,	0x0000001FFFFFFEFFu,
					0x003FFFFFFFFFFFFFu,	0x0007FFFF003FFFFFu,	0x000000000003FFFFu,	0x0000000000000000u,
					0xFFFFFFFFFFFFFFFFu,	0x00000000000001FFu,	0x0007FFFFFFFFFFFFu,	0x0007FFFFFFFFFFFFu,
					0x0000000FFFFFFFFFu,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x000303FFFFFFFFFFu,	0x0000000000000000u,
					0x000000000FFFFFFFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x10300ull) / 0x40ull]
					& (0x1ull << (static_cast<uint_least64_t>(c) % 0x40ull));
				// 1620 codepoints from 48 ranges (spanning a search area of 3147)
			}
			case 0x16: // [22] 10F1C - 11B66
			{
				if (c > U'\U00011AF8')
					return false;
				TOML_ASSUME(U'\U00010F1C' <= c);

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x000003FFFFF00801u,	0x0000000000000000u,	0x000001FFFFF00000u,	0xFFFFFF8007FFFFF0u,
					0x000000000FFFFFFFu,	0xFFFFFF8000000000u,	0xFFF00000000FFFFFu,	0xFFFFFF8000001FFFu,
					0xFFF00900000007FFu,	0xFFFFFF80047FFFFFu,	0x400001E0007FFFFFu,	0xFFBFFFF000000001u,
					0x000000000000FFFFu,	0xFFFBD7F000000000u,	0xFFFFFFFFFFF01FFBu,	0xFF99FE0000000007u,
					0x001000023EDFDFFFu,	0x000000000000003Eu,	0x0000000000000000u,	0xFFFFFFF000000000u,
					0x0000780001FFFFFFu,	0xFFFFFFF000000038u,	0x00000B00000FFFFFu,	0x0000000000000000u,
					0x0000000000000000u,	0xFFFFFFF000000000u,	0xF00000000007FFFFu,	0xFFFFFFF000000000u,
					0x00000100000FFFFFu,	0xFFFFFFF000000000u,	0x0000000010007FFFu,	0x7FFFFFF000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0xFFFFFFF000000000u,
					0x000000000000FFFFu,	0x0000000000000000u,	0xFFFFFFFFFFFFFFF0u,	0xF6FF27F80000000Fu,
					0x00000028000FFFFFu,	0x0000000000000000u,	0x001FFFFFFFFFCFF0u,	0xFFFF8010000000A0u,
					0x00100000407FFFFFu,	0x00003FFFFFFFFFFFu,	0xFFFFFFF000000002u,	0x000000001FFFFFFFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x10F1Cull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0x10F1Cull) % 0x40ull));
				// 1130 codepoints from 67 ranges (spanning a search area of 3147)
			}
			case 0x17: // [23] 11B67 - 127B1
			{
				if (U'\U00011C00' > c || c > U'\U00012543')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x00007FFFFFFFFDFFu,	0xFFFC000000000001u,	0x000000000000FFFFu,	0x0000000000000000u,
					0x0001FFFFFFFFFB7Fu,	0xFFFFFDBF00000040u,	0x00000000010003FFu,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0007FFFF00000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0001000000000000u,	0x0000000000000000u,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0x0000000003FFFFFFu,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0x000000000000000Fu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x11C00ull) / 0x40ull]
					& (0x1ull << (static_cast<uint_least64_t>(c) % 0x40ull));
				// 1304 codepoints from 16 ranges (spanning a search area of 3147)
			}
			case 0x18: return U'\U00013000' <= c;
			case 0x19: return c <= U'\U0001342E';
			case 0x1A: return U'\U00014400' <= c && c <= U'\U00014646';
			case 0x1D: // [29] 16529 - 17173
			{
				if (U'\U00016800' > c)
					return false;
				TOML_ASSUME(c <= U'\U00017173');

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0x01FFFFFFFFFFFFFFu,	0x000000007FFFFFFFu,	0x0000000000000000u,	0x00003FFFFFFF0000u,
					0x0000FFFFFFFFFFFFu,	0xE0FFFFF80000000Fu,	0x000000000000FFFFu,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0xFFFFFFFFFFFFFFFFu,	0x0000000000000000u,	0x0000000000000000u,
					0xFFFFFFFFFFFFFFFFu,	0x00000000000107FFu,	0x00000000FFF80000u,	0x0000000B00000000u,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0x000FFFFFFFFFFFFFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x16800ull) / 0x40ull]
					& (0x1ull << (static_cast<uint_least64_t>(c) % 0x40ull));
				// 1250 codepoints from 14 ranges (spanning a search area of 3147)
			}
			case 0x1F: return c <= U'\U000187F7' || U'\U00018800' <= c;
			case 0x20: return c <= U'\U00018CD5' || (U'\U00018D00' <= c && c <= U'\U00018D08');
			case 0x23: // [35] 1AEEB - 1BB35
			{
				if (U'\U0001B000' > c || c > U'\U0001B2FB')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0x000000007FFFFFFFu,	0xFFFF00F000070000u,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0x0FFFFFFFFFFFFFFFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x1B000ull) / 0x40ull]
					& (0x1ull << (static_cast<uint_least64_t>(c) % 0x40ull));
				// 690 codepoints from 4 ranges (spanning a search area of 3147)
			}
			case 0x24: // [36] 1BB36 - 1C780
			{
				if (U'\U0001BC00' > c || c > U'\U0001BC99')
					return false;

				switch ((static_cast<uint_least64_t>(c) - 0x1BC00ull) / 0x40ull)
				{
					case 0x01: return c <= U'\U0001BC7C' && (1ull << (static_cast<uint_least64_t>(c) - 0x1BC40u)) & 0x1FFF07FFFFFFFFFFull;
					case 0x02: return (1u << (static_cast<uint_least32_t>(c) - 0x1BC80u)) & 0x3FF01FFu;
					default: return true;
				}
				// 139 codepoints from 4 ranges (spanning a search area of 3147)
			}
			case 0x26: // [38] 1D3CC - 1E016
			{
				if (U'\U0001D400' > c || c > U'\U0001D7CB')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFDFFFFFu,	0xEBFFDE64DFFFFFFFu,	0xFFFFFFFFFFFFFFEFu,
					0x7BFFFFFFDFDFE7BFu,	0xFFFFFFFFFFFDFC5Fu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFF3FFFFFFFFFu,	0xF7FFFFFFF7FFFFFDu,
					0xFFDFFFFFFFDFFFFFu,	0xFFFF7FFFFFFF7FFFu,	0xFFFFFDFFFFFFFDFFu,	0x0000000000000FF7u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x1D400ull) / 0x40ull]
					& (0x1ull << (static_cast<uint_least64_t>(c) % 0x40ull));
				// 936 codepoints from 30 ranges (spanning a search area of 3147)
			}
			case 0x27: // [39] 1E017 - 1EC61
			{
				if (U'\U0001E100' > c || c > U'\U0001E94B')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x3F801FFFFFFFFFFFu,	0x0000000000004000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x00000FFFFFFFFFFFu,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0xFFFFFFFFFFFFFFFFu,	0x000000000000001Fu,
					0xFFFFFFFFFFFFFFFFu,	0x000000000000080Fu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x1E100ull) / 0x40ull]
					& (0x1ull << (static_cast<uint_least64_t>(c) % 0x40ull));
				// 363 codepoints from 7 ranges (spanning a search area of 3147)
			}
			case 0x28: // [40] 1EC62 - 1F8AC
			{
				if (U'\U0001EE00' > c || c > U'\U0001EEBB')
					return false;

				switch ((static_cast<uint_least64_t>(c) - 0x1EE00ull) / 0x40ull)
				{
					case 0x00: return c <= U'\U0001EE3B' && (1ull << (static_cast<uint_least64_t>(c) - 0x1EE00u)) & 0xAF7FE96FFFFFFEFull;
					case 0x01: return U'\U0001EE42' <= c && c <= U'\U0001EE7E'
						&& (1ull << (static_cast<uint_least64_t>(c) - 0x1EE42u)) & 0x17BDFDE5AAA5BAA1ull;
					case 0x02: return (1ull << (static_cast<uint_least64_t>(c) - 0x1EE80u)) & 0xFFFFBEE0FFFFBFFull;
					TOML_NO_DEFAULT_CASE;
				}
				// 141 codepoints from 33 ranges (spanning a search area of 3147)
			}
			case 0x29: return U'\U00020000' <= c;
			case 0x37: return c <= U'\U0002A6DD' || U'\U0002A700' <= c;
			case 0x38: return c <= U'\U0002B734' || (U'\U0002B740' <= c && c <= U'\U0002B81D') || U'\U0002B820' <= c;
			case 0x3A: return c <= U'\U0002CEA1' || U'\U0002CEB0' <= c;
			case 0x3C: return c <= U'\U0002EBE0';
			case 0x3D: return U'\U0002F800' <= c && c <= U'\U0002FA1D';
			case 0x3E: return U'\U00030000' <= c;
			TOML_NO_DEFAULT_CASE;
		}
		// 131189 codepoints from 620 ranges (spanning a search area of 1114112)
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_unicode_number(char32_t c) noexcept
	{
		if (U'\u0660' > c || c > U'\U0001FBF9')
			return false;

		const auto child_index_0 = (static_cast<uint_least64_t>(c) - 0x660ull) / 0x7D7ull;
		if ((1ull << child_index_0) & 0x47FFDFE07FCFFFD0ull)
			return false;
		switch (child_index_0)
		{
			case 0x00: // [0] 0660 - 0E36
			{
				if (c > U'\u0DEF')
					return false;
				TOML_ASSUME(U'\u0660' <= c);

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x00000000000003FFu,	0x0000000000000000u,	0x0000000003FF0000u,	0x0000000000000000u,
					0x0000000000000000u,	0x000003FF00000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x000000000000FFC0u,	0x0000000000000000u,	0x000000000000FFC0u,	0x0000000000000000u,
					0x000000000000FFC0u,	0x0000000000000000u,	0x000000000000FFC0u,	0x0000000000000000u,
					0x000000000000FFC0u,	0x0000000000000000u,	0x000000000000FFC0u,	0x0000000000000000u,
					0x000000000000FFC0u,	0x0000000000000000u,	0x000000000000FFC0u,	0x0000000000000000u,
					0x000000000000FFC0u,	0x0000000000000000u,	0x000000000000FFC0u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x660ull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0x660ull) % 0x40ull));
				// 130 codepoints from 13 ranges (spanning a search area of 2007)
			}
			case 0x01: // [1] 0E37 - 160D
			{
				if (U'\u0E50' > c || c > U'\u1099')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x00000000000003FFu,	0x0000000000000000u,	0x00000000000003FFu,	0x0000000003FF0000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x03FF000000000000u,
					0x0000000000000000u,	0x00000000000003FFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0xE50ull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0xE50ull) % 0x40ull));
				// 50 codepoints from 5 ranges (spanning a search area of 2007)
			}
			case 0x02: // [2] 160E - 1DE4
			{
				if (U'\u16EE' > c || c > U'\u1C59')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x0000000000000007u,	0x0000000000000000u,	0x0000000000000000u,	0x0FFC000000000000u,
					0x00000FFC00000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x00000003FF000000u,	0x0000000000000000u,	0x00000FFC00000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x00000FFC0FFC0000u,	0x0000000000000000u,
					0x0000000000000000u,	0x00000FFC00000000u,	0x0000000000000000u,	0x0000000000000FFCu,
					0x0000000000000000u,	0x00000FFC0FFC0000u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x16EEull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0x16EEull) % 0x40ull));
				// 103 codepoints from 11 ranges (spanning a search area of 2007)
			}
			case 0x03: return U'\u2160' <= c && c <= U'\u2188' && (1ull << (static_cast<uint_least64_t>(c) - 0x2160u)) & 0x1E7FFFFFFFFull;
			case 0x05: return U'\u3007' <= c && c <= U'\u303A' && (1ull << (static_cast<uint_least64_t>(c) - 0x3007u)) & 0xE0007FC000001ull;
			case 0x14: // [20] A32C - AB02
			{
				if (U'\uA620' > c || c > U'\uAA59')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x00000000000003FFu,	0x0000000000000000u,	0x0000000000000000u,	0x000000000000FFC0u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x03FF000000000000u,	0x000003FF00000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x03FF000000000000u,	0x0000000003FF0000u,
					0x03FF000000000000u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0xA620ull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0xA620ull) % 0x40ull));
				// 70 codepoints from 7 ranges (spanning a search area of 2007)
			}
			case 0x15: return U'\uABF0' <= c && c <= U'\uABF9';
			case 0x1F: return U'\uFF10' <= c && c <= U'\uFF19';
			case 0x20: // [32] 10140 - 10916
			{
				if (c > U'\U000104A9')
					return false;
				TOML_ASSUME(U'\U00010140' <= c);

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x001FFFFFFFFFFFFFu,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000402u,	0x0000000000000000u,	0x00000000003E0000u,	0x0000000000000000u,
					0x0000000000000000u,	0x000003FF00000000u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x10140ull) / 0x40ull]
					& (0x1ull << (static_cast<uint_least64_t>(c) % 0x40ull));
				// 70 codepoints from 5 ranges (spanning a search area of 2007)
			}
			case 0x21: return (U'\U00010D30' <= c && c <= U'\U00010D39') || (U'\U00011066' <= c && c <= U'\U0001106F');
			case 0x22: // [34] 110EE - 118C4
			{
				if (U'\U000110F0' > c || c > U'\U00011739')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x00000000000003FFu,	0x000000000000FFC0u,	0x0000000000000000u,	0x000003FF00000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x00000000000003FFu,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x000003FF00000000u,	0x0000000000000000u,	0x000003FF00000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x000003FF00000000u,	0x0000000000000000u,	0x0000000003FF0000u,
					0x0000000000000000u,	0x00000000000003FFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x110F0ull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0x110F0ull) % 0x40ull));
				// 90 codepoints from 9 ranges (spanning a search area of 2007)
			}
			case 0x23: // [35] 118C5 - 1209B
			{
				if (U'\U000118E0' > c || c > U'\U00011DA9')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x00000000000003FFu,	0x03FF000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x03FF000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x03FF000000000000u,	0x0000000000000000u,	0x00000000000003FFu,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x118E0ull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0x118E0ull) % 0x40ull));
				// 50 codepoints from 5 ranges (spanning a search area of 2007)
			}
			case 0x24: return U'\U00012400' <= c && c <= U'\U0001246E';
			case 0x2D: return (U'\U00016A60' <= c && c <= U'\U00016A69') || (U'\U00016B50' <= c && c <= U'\U00016B59');
			case 0x3B: return U'\U0001D7CE' <= c && c <= U'\U0001D7FF';
			case 0x3C: return (U'\U0001E140' <= c && c <= U'\U0001E149') || (U'\U0001E2F0' <= c && c <= U'\U0001E2F9');
			case 0x3D: return U'\U0001E950' <= c && c <= U'\U0001E959';
			case 0x3F: return U'\U0001FBF0' <= c;
			TOML_NO_DEFAULT_CASE;
		}
		// 876 codepoints from 72 ranges (spanning a search area of 1114112)
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_unicode_combining_mark(char32_t c) noexcept
	{
		if (U'\u0300' > c || c > U'\U000E01EF')
			return false;

		const auto child_index_0 = (static_cast<uint_least64_t>(c) - 0x300ull) / 0x37FCull;
		if ((1ull << child_index_0) & 0x7FFFFFFFFFFFFE02ull)
			return false;
		switch (child_index_0)
		{
			case 0x00: // [0] 0300 - 3AFB
			{
				if (c > U'\u309A')
					return false;
				TOML_ASSUME(U'\u0300' <= c);

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0xFFFFFFFFFFFFFFFFu,	0x0000FFFFFFFFFFFFu,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x00000000000000F8u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0xBFFFFFFFFFFE0000u,	0x00000000000000B6u,
					0x0000000007FF0000u,	0x00010000FFFFF800u,	0x0000000000000000u,	0x00003D9F9FC00000u,
					0xFFFF000000020000u,	0x00000000000007FFu,	0x0001FFC000000000u,	0x200FF80000000000u,
					0x00003EEFFBC00000u,	0x000000000E000000u,	0x0000000000000000u,	0xFFFFFFFBFFF80000u,
					0xDC0000000000000Fu,	0x0000000C00FEFFFFu,	0xD00000000000000Eu,	0x4000000C0080399Fu,
					0xD00000000000000Eu,	0x0023000000023987u,	0xD00000000000000Eu,	0xFC00000C00003BBFu,
					0xD00000000000000Eu,	0x0000000C00E0399Fu,	0xC000000000000004u,	0x0000000000803DC7u,
					0xC00000000000001Fu,	0x0000000C00603DDFu,	0xD00000000000000Eu,	0x0000000C00603DDFu,
					0xD80000000000000Fu,	0x0000000C00803DDFu,	0x000000000000000Eu,	0x000C0000FF5F8400u,
					0x07F2000000000000u,	0x0000000000007F80u,	0x1FF2000000000000u,	0x0000000000003F00u,
					0xC2A0000003000000u,	0xFFFE000000000000u,	0x1FFFFFFFFEFFE0DFu,	0x0000000000000040u,
					0x7FFFF80000000000u,	0x001E3F9DC3C00000u,	0x000000003C00BFFCu,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x00000000E0000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x001C0000001C0000u,	0x000C0000000C0000u,	0xFFF0000000000000u,	0x00000000200FFFFFu,
					0x0000000000003800u,	0x0000000000000000u,	0x0000020000000060u,	0x0000000000000000u,
					0x0FFF0FFF00000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x000000000F800000u,	0x9FFFFFFF7FE00000u,	0xBFFF000000000000u,	0x0000000000000001u,
					0xFFF000000000001Fu,	0x000FF8000000001Fu,	0x00003FFE00000007u,	0x000FFFC000000000u,
					0x00FFFFF000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x039021FFFFF70000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0xFBFFFFFFFFFFFFFFu,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0001FFE21FFF0000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0003800000000000u,
					0x0000000000000000u,	0x8000000000000000u,	0x0000000000000000u,	0xFFFFFFFF00000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000FC0000000000u,	0x0000000000000000u,	0x0000000006000000u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x300ull) / 0x40ull]
					& (0x1ull << (static_cast<uint_least64_t>(c) % 0x40ull));
				// 1106 codepoints from 156 ranges (spanning a search area of 14332)
			}
			case 0x02: // [2] 72F8 - AAF3
			{
				if (U'\uA66F' > c || c > U'\uAAEF')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x0001800000007FE1u,	0x0000000000000000u,	0x0000000000000006u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x21F0000010880000u,	0x0000000000000000u,
					0x0000000000060000u,	0xFFFE0000007FFFE0u,	0x7F80000000010007u,	0x0000001FFF000000u,
					0x00000000001E0000u,	0x004000000003FFF0u,	0xFC00000000000000u,	0x00000000601000FFu,
					0x0000000000007000u,	0xF00000000005833Au,	0x0000000000000001u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0xA66Full) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0xA66Full) % 0x40ull));
				// 137 codepoints from 28 ranges (spanning a search area of 14332)
			}
			case 0x03: return (U'\uAAF5' <= c && c <= U'\uAAF6') || (U'\uABE3' <= c && c <= U'\uABEA') || (U'\uABEC' <= c && c <= U'\uABED');
			case 0x04: // [4] E2F0 - 11AEB
			{
				if (U'\uFB1E' > c || c > U'\U00011A99')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x0000000000000001u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0003FFFC00000000u,
					0x000000000003FFFCu,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000080000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000004u,
					0x0000000000000000u,	0x000000001F000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0003C1B800000000u,
					0x000000021C000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000180u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x00000000000003C0u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000006000u,	0x0000000000000000u,
					0x0007FF0000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000001C00000000u,
					0x000001FFFC000000u,	0x0000001E00000000u,	0x000000001FFC0000u,	0x0000001C00000000u,
					0x00000180007FFE00u,	0x0000001C00200000u,	0x00037807FFE00000u,	0x0000000000000000u,
					0x0000000103FFC000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000003C00001FFEu,
					0x0200E67F60000000u,	0x00000000007C7F30u,	0x0000000000000000u,	0x0000000000000000u,
					0x000001FFFF800000u,	0x0000000000000001u,	0x0000003FFFFC0000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0xC0000007FCFE0000u,	0x0000000000000000u,
					0x00000007FFFC0000u,	0x0000000000000000u,	0x0000000003FFE000u,	0x8000000000000000u,
					0x0000000000003FFFu,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x000000001FFFC000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x00000035E6FC0000u,	0x0000000000000000u,	0xF3F8000000000000u,	0x00001FF800000047u,
					0x3FF80201EFE00000u,	0x0FFFF00000000000u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0xFB1Eull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0xFB1Eull) % 0x40ull));
				// 402 codepoints from 63 ranges (spanning a search area of 14332)
			}
			case 0x05: // [5] 11AEC - 152E7
			{
				if (U'\U00011C2F' > c || c > U'\U00011EF6')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x000000000001FEFFu,	0xFDFFFFF800000000u,	0x00000000000000FFu,	0x0000000000000000u,
					0x00000000017F68FCu,	0x000001F6F8000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x00000000000000F0u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x11C2Full) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0x11C2Full) % 0x40ull));
				// 85 codepoints from 13 ranges (spanning a search area of 14332)
			}
			case 0x06: // [6] 152E8 - 18AE3
			{
				if (U'\U00016AF0' > c || c > U'\U00016FF1')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x000000000000001Fu,	0x000000000000007Fu,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0xFFFFFFFE80000000u,	0x0000000780FFFFFFu,	0x0010000000000000u,
					0x0000000000000003u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x16AF0ull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0x16AF0ull) % 0x40ull));
				// 75 codepoints from 7 ranges (spanning a search area of 14332)
			}
			case 0x07: return U'\U0001BC9D' <= c && c <= U'\U0001BC9E';
			case 0x08: // [8] 1C2E0 - 1FADB
			{
				if (U'\U0001D165' > c || c > U'\U0001E94A')
					return false;

				constexpr uint_least64_t bitmask_table_1[] =
				{
					0x0000007F3FC03F1Fu,	0x00000000000001E0u,	0x0000000000000000u,	0x00000000E0000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0xFFFFFFFFF8000000u,	0xFFFFFFFFFFC3FFFFu,
					0xF7C00000800100FFu,	0x00000000000007FFu,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0xDFCFFFFBF8000000u,	0x000000000000003Eu,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x000000000003F800u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000780u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,	0x0000000000000000u,
					0x0000000000000000u,	0x0003F80000000000u,	0x0000000000000000u,	0x0000003F80000000u,
				};
				return bitmask_table_1[(static_cast<uint_least64_t>(c) - 0x1D165ull) / 0x40ull]
					& (0x1ull << ((static_cast<uint_least64_t>(c) - 0x1D165ull) % 0x40ull));
				// 223 codepoints from 21 ranges (spanning a search area of 14332)
			}
			case 0x3F: return U'\U000E0100' <= c;
			TOML_NO_DEFAULT_CASE;
		}
		// 2282 codepoints from 293 ranges (spanning a search area of 1114112)
	}

	#endif // TOML_LANG_UNRELEASED

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_bare_key_character(char32_t codepoint) noexcept
	{
		return is_ascii_letter(codepoint)
			|| is_decimal_digit(codepoint)
			|| codepoint == U'-'
			|| codepoint == U'_'
			#if TOML_LANG_UNRELEASED // toml/issues/644 ('+' in bare keys) & toml/issues/687 (unicode bare keys)
			|| codepoint == U'+'
			|| is_unicode_letter(codepoint)
			|| is_unicode_number(codepoint)
			|| is_unicode_combining_mark(codepoint)
			#endif
		;
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_value_terminator(char32_t codepoint) noexcept
	{
		return is_ascii_line_break(codepoint)
			|| is_ascii_whitespace(codepoint)
			|| codepoint == U']'
			|| codepoint == U'}'
			|| codepoint == U','
			|| codepoint == U'#'
			|| is_unicode_line_break(codepoint)
			|| is_unicode_whitespace(codepoint)
		;
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_control_character(char32_t codepoint) noexcept
	{
		return codepoint <= U'\u001F' || codepoint == U'\u007F';
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_nontab_control_character(char32_t codepoint) noexcept
	{
		return codepoint <= U'\u0008'
			|| (codepoint >= U'\u000A' && codepoint <= U'\u001F')
			|| codepoint == U'\u007F';
	}

	[[nodiscard]]
	TOML_ATTR(const)
	constexpr bool is_unicode_surrogate(char32_t codepoint) noexcept
	{
		return codepoint >= 0xD800u && codepoint <= 0xDFFF;
	}

	struct utf8_decoder final
	{
		// utf8_decoder based on this: https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
		// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>

		uint_least32_t state{};
		char32_t codepoint{};

		static constexpr uint8_t state_table[]
		{
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,		9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
			7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
			8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
			10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3,		11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

			0,12,24,36,60,96,84,12,12,12,48,72,		12,12,12,12,12,12,12,12,12,12,12,12,
			12, 0,12,12,12,12,12, 0,12, 0,12,12,	12,24,12,12,12,12,12,24,12,24,12,12,
			12,12,12,12,12,12,12,24,12,12,12,12,	12,24,12,12,12,12,12,12,12,24,12,12,
			12,12,12,12,12,12,12,36,12,36,12,12,	12,36,12,12,12,12,12,36,12,36,12,12,
			12,36,12,12,12,12,12,12,12,12,12,12
		};

		[[nodiscard]]
		constexpr bool error() const noexcept
		{
			return state == uint_least32_t{ 12u };
		}

		[[nodiscard]]
		constexpr bool has_code_point() const noexcept
		{
			return state == uint_least32_t{};
		}

		[[nodiscard]]
		constexpr bool needs_more_input() const noexcept
		{
			return state > uint_least32_t{} && state != uint_least32_t{ 12u };
		}

		constexpr void operator () (uint8_t byte) noexcept
		{
			TOML_ASSERT(!error());

			const auto type = state_table[byte];

			codepoint = static_cast<char32_t>(
				has_code_point()
					? (uint_least32_t{ 255u } >> type) & byte
					: (byte & uint_least32_t{ 63u }) | (static_cast<uint_least32_t>(codepoint) << 6)
				);

			state = state_table[state + uint_least32_t{ 256u } + type];
		}
	};
}
TOML_IMPL_NAMESPACE_END

#endif // !DOXYGEN

#endif //-----------------------------------  ↑ toml_utf8.h  -----------------------------------------------------------

#if 1  //---------------------------------------------------------  ↓ toml_formatter.h  --------------------------------

TOML_PUSH_WARNINGS
TOML_DISABLE_SWITCH_WARNINGS

TOML_IMPL_NAMESPACE_START
{
	template <typename Char = char>
	class TOML_API formatter
	{
		private:
			const toml::node* source_;
			std::basic_ostream<Char>* stream_ = nullptr;
			format_flags flags_;
			int indent_;
			bool naked_newline_;

		protected:

			[[nodiscard]] const toml::node& source() const noexcept { return *source_; }
			[[nodiscard]] std::basic_ostream<Char>& stream() const noexcept { return *stream_; }

			static constexpr size_t indent_columns = 4;
			static constexpr std::string_view indent_string = "    "sv;
			[[nodiscard]] int indent() const noexcept { return indent_; }
			void indent(int level) noexcept { indent_ = level; }
			void increase_indent() noexcept { indent_++; }
			void decrease_indent() noexcept { indent_--; }
			[[nodiscard]]
			bool quote_dates_and_times() const noexcept
			{
				return (flags_ & format_flags::quote_dates_and_times) != format_flags::none;
			}

			[[nodiscard]]
			bool literal_strings_allowed() const noexcept
			{
				return (flags_ & format_flags::allow_literal_strings) != format_flags::none;
			}

			[[nodiscard]]
			bool multi_line_strings_allowed() const noexcept
			{
				return (flags_ & format_flags::allow_multi_line_strings) != format_flags::none;
			}

			[[nodiscard]]
			bool value_format_flags_allowed() const noexcept
			{
				return (flags_ & format_flags::allow_value_format_flags) != format_flags::none;
			}

			[[nodiscard]]
			bool naked_newline() const noexcept
			{
				return naked_newline_;
			}

			void clear_naked_newline() noexcept
			{
				naked_newline_ = false;
			}

			void attach(std::basic_ostream<Char>& stream) noexcept
			{
				indent_ = {};
				naked_newline_ = true;
				stream_ = &stream;
			}

			void detach() noexcept
			{
				stream_ = nullptr;
			}

			void print_newline(bool force = false)
			{
				if (!naked_newline_ || force)
				{
					print_to_stream('\n', *stream_);
					naked_newline_ = true;
				}
			}

			void print_indent()
			{
				for (int i = 0; i < indent_; i++)
				{
					print_to_stream(indent_string, *stream_);
					naked_newline_ = false;
				}
			}

			void print_quoted_string(std::string_view str, bool allow_multi_line = true)
			{
				auto literals = literal_strings_allowed();
				if (str.empty())
				{
					print_to_stream(literals ? "''"sv : "\"\""sv, *stream_);
					clear_naked_newline();
					return;
				}

				auto multi_line = allow_multi_line && multi_line_strings_allowed();
				if (multi_line || literals)
				{
					utf8_decoder decoder;
					bool has_line_breaks = false;
					bool has_control_chars = false;
					bool has_single_quotes = false;
					for (size_t i = 0; i < str.length() && !(has_line_breaks && has_control_chars && has_single_quotes); i++)
					{
						decoder(static_cast<uint8_t>(str[i]));
						if (decoder.error())
						{
							has_line_breaks = false;
							has_control_chars = true; //force ""
							has_single_quotes = true;
							break;
						}
						else if (decoder.has_code_point())
						{
							if (is_line_break(decoder.codepoint))
								has_line_breaks = true;
							else if (is_nontab_control_character(decoder.codepoint))
								has_control_chars = true;
							else if (decoder.codepoint == U'\'')
								has_single_quotes = true;
						}
					}
					multi_line = multi_line && has_line_breaks;
					literals = literals && !has_control_chars && !(!multi_line && has_single_quotes);
				}

				if (literals)
				{
					const auto quot = multi_line ? "'''"sv : "'"sv;
					print_to_stream(quot, *stream_);
					print_to_stream(str, *stream_);
					print_to_stream(quot, *stream_);
				}
				else
				{
					const auto quot = multi_line ? R"(""")"sv : R"(")"sv;
					print_to_stream(quot, *stream_);
					print_to_stream_with_escapes(str, *stream_);
					print_to_stream(quot, *stream_);
				}
				clear_naked_newline();
			}

			template <typename T>
			void print(const value<T>& val)
			{
				if constexpr (std::is_same_v<T, std::string>)
				{
					print_quoted_string(val.get());
				}
				else
				{
					if constexpr (is_one_of<T, date, time, date_time>)
					{
						if (quote_dates_and_times())
						{
							const auto quot = literal_strings_allowed() ? '\'' : '"';
							print_to_stream(quot, *stream_);
							print_to_stream(*val, *stream_);
							print_to_stream(quot, *stream_);
						}
						else
							print_to_stream(*val, *stream_);
					}
					else if constexpr (is_one_of<T, int64_t/*, double*/>)
					{
						if (value_format_flags_allowed() && *val >= 0)
						{
							const auto fmt = val.flags() & value_flags::format_as_hexadecimal;
							if (fmt != value_flags::none)
							{
								switch (fmt)
								{
									case value_flags::format_as_binary: print_to_stream("0b"sv, *stream_); break;
									case value_flags::format_as_octal: print_to_stream("0o"sv, *stream_); break;
									case value_flags::format_as_hexadecimal: print_to_stream("0x"sv, *stream_); break;
								}
								print_to_stream(*val, *stream_, fmt);
							}
							else
								print_to_stream(*val, *stream_);
						}
						else
							print_to_stream(*val, *stream_);
					}
					else
						print_to_stream(*val, *stream_);

					naked_newline_ = false;
				}
			}

			void print_value(const node& val_node, node_type type)
			{
				TOML_ASSUME(type > node_type::array);
				switch (type)
				{
					case node_type::string:			print(*reinterpret_cast<const value<std::string>*>(&val_node)); break;
					case node_type::integer:		print(*reinterpret_cast<const value<int64_t>*>(&val_node)); break;
					case node_type::floating_point:	print(*reinterpret_cast<const value<double>*>(&val_node)); break;
					case node_type::boolean:		print(*reinterpret_cast<const value<bool>*>(&val_node)); break;
					case node_type::date:			print(*reinterpret_cast<const value<date>*>(&val_node)); break;
					case node_type::time:			print(*reinterpret_cast<const value<time>*>(&val_node)); break;
					case node_type::date_time:		print(*reinterpret_cast<const value<date_time>*>(&val_node)); break;
					TOML_NO_DEFAULT_CASE;
				}
			}

			formatter(const toml::node& source, format_flags flags) noexcept
				: source_{ &source },
				flags_{ flags }
			{}
	};

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template class TOML_API formatter<char>;
	#endif

}
TOML_IMPL_NAMESPACE_END

TOML_POP_WARNINGS // TOML_DISABLE_SWITCH_WARNINGS

#endif //---------------------------------------------------------  ↑ toml_formatter.h  --------------------------------

#if 1  //------------------------------------------------------------------------------  ↓ toml_default_formatter.h  ---

TOML_PUSH_WARNINGS
TOML_DISABLE_SWITCH_WARNINGS

TOML_IMPL_NAMESPACE_START
{
	[[nodiscard]] TOML_API std::string default_formatter_make_key_segment(const std::string&) noexcept;
	[[nodiscard]] TOML_API size_t default_formatter_inline_columns(const node&) noexcept;
	[[nodiscard]] TOML_API bool default_formatter_forces_multiline(const node&, size_t = 0) noexcept;
}
TOML_IMPL_NAMESPACE_END

TOML_NAMESPACE_START
{
	template <typename Char = char>
	class TOML_API default_formatter final : impl::formatter<Char>
	{
		private:
			using base = impl::formatter<Char>;
			std::vector<std::string> key_path;

			void print_key_segment(const std::string& str)
			{
				if (str.empty())
					impl::print_to_stream("''"sv, base::stream());
				else
				{
					bool requiresQuotes = false;
					{
						impl::utf8_decoder decoder;
						for (size_t i = 0; i < str.length() && !requiresQuotes; i++)
						{
							decoder(static_cast<uint8_t>(str[i]));
							if (decoder.error())
								requiresQuotes = true;
							else if (decoder.has_code_point())
								requiresQuotes = !impl::is_bare_key_character(decoder.codepoint);
						}
					}

					if (requiresQuotes)
					{
						impl::print_to_stream('"', base::stream());
						impl::print_to_stream_with_escapes(str, base::stream());
						impl::print_to_stream('"', base::stream());
					}
					else
						impl::print_to_stream(str, base::stream());
				}
				base::clear_naked_newline();
			}

			void print_key_path()
			{
				for (const auto& segment : key_path)
				{
					if (std::addressof(segment) > key_path.data())
						impl::print_to_stream('.', base::stream());
					impl::print_to_stream(segment, base::stream());
				}
				base::clear_naked_newline();
			}

			void print_inline(const table& /*tbl*/);
			void print(const array& arr)
			{
				if (arr.empty())
					impl::print_to_stream("[]"sv, base::stream());
				else
				{
					const auto original_indent = base::indent();
					const auto multiline = impl::default_formatter_forces_multiline(
						arr,
						base::indent_columns * static_cast<size_t>(original_indent < 0 ? 0 : original_indent)
					);
					impl::print_to_stream("["sv, base::stream());
					if (multiline)
					{
						if (original_indent < 0)
							base::indent(0);
						base::increase_indent();
					}
					else
						impl::print_to_stream(' ', base::stream());

					for (size_t i = 0; i < arr.size(); i++)
					{
						if (i > 0_sz)
						{
							impl::print_to_stream(',', base::stream());
							if (!multiline)
								impl::print_to_stream(' ', base::stream());
						}

						if (multiline)
						{
							base::print_newline(true);
							base::print_indent();
						}

						auto& v = arr[i];
						const auto type = v.type();
						TOML_ASSUME(type != node_type::none);
						switch (type)
						{
							case node_type::table: print_inline(*reinterpret_cast<const table*>(&v)); break;
							case node_type::array: print(*reinterpret_cast<const array*>(&v)); break;
							default:
								base::print_value(v, type);
						}

					}
					if (multiline)
					{
						base::indent(original_indent);
						base::print_newline(true);
						base::print_indent();
					}
					else
						impl::print_to_stream(' ', base::stream());
					impl::print_to_stream("]"sv, base::stream());
				}
				base::clear_naked_newline();
			}

			void print(const table& tbl)
			{
				static constexpr auto is_non_inline_array_of_tables = [](auto&& nde) noexcept
				{
					auto arr = nde.as_array();
					return arr
						&& arr->is_array_of_tables()
						&& !arr->template get_as<table>(0_sz)->is_inline();
				};

				// values, arrays, and inline tables/table arrays
				for (auto&& [k, v] : tbl)
				{
					const auto type = v.type();
					if ((type == node_type::table && !reinterpret_cast<const table*>(&v)->is_inline())
						|| (type == node_type::array && is_non_inline_array_of_tables(v)))
						continue;

					base::print_newline();
					base::print_indent();
					print_key_segment(k);
					impl::print_to_stream(" = "sv, base::stream());
					TOML_ASSUME(type != node_type::none);
					switch (type)
					{
						case node_type::table: print_inline(*reinterpret_cast<const table*>(&v)); break;
						case node_type::array: print(*reinterpret_cast<const array*>(&v)); break;
						default:
							base::print_value(v, type);
					}
				}

				// non-inline tables
				for (auto&& [k, v] : tbl)
				{
					const auto type = v.type();
					if (type != node_type::table || reinterpret_cast<const table*>(&v)->is_inline())
						continue;
					auto& child_tbl = *reinterpret_cast<const table*>(&v);

					// we can skip indenting and emitting the headers for tables that only contain other tables
					// (so we don't over-nest)
					size_t child_value_count{}; //includes inline tables and non-table arrays
					size_t child_table_count{};
					size_t child_table_array_count{};
					for (auto&& [child_k, child_v] : child_tbl)
					{
						(void)child_k;
						const auto child_type = child_v.type();
						TOML_ASSUME(child_type != node_type::none);
						switch (child_type)
						{
							case node_type::table:
								if (reinterpret_cast<const table*>(&child_v)->is_inline())
									child_value_count++;
								else
									child_table_count++;
								break;

							case node_type::array:
								if (is_non_inline_array_of_tables(child_v))
									child_table_array_count++;
								else
									child_value_count++;
								break;

							default:
								child_value_count++;
						}
					}
					bool skip_self = false;
					if (child_value_count == 0_sz && (child_table_count > 0_sz || child_table_array_count > 0_sz))
						skip_self = true;

					key_path.push_back(impl::default_formatter_make_key_segment(k));

					if (!skip_self)
					{
						if (!base::naked_newline())
						{
							base::print_newline();
							base::print_newline(true);
						}
						base::increase_indent();
						base::print_indent();
						impl::print_to_stream("["sv, base::stream());
						print_key_path();
						impl::print_to_stream("]"sv, base::stream());
						base::print_newline();
					}

					print(child_tbl);

					key_path.pop_back();
					if (!skip_self)
						base::decrease_indent();
				}

				// table arrays
				for (auto&& [k, v] : tbl)
				{
					if (!is_non_inline_array_of_tables(v))
						continue;
					auto& arr = *reinterpret_cast<const array*>(&v);

					base::increase_indent();
					key_path.push_back(impl::default_formatter_make_key_segment(k));

					for (size_t i = 0; i < arr.size(); i++)
					{
						base::print_newline();
						base::print_newline(true);
						base::print_indent();
						impl::print_to_stream("[["sv, base::stream());
						print_key_path();
						impl::print_to_stream("]]"sv, base::stream());
						base::print_newline(true);
						print(*reinterpret_cast<const table*>(&arr[i]));
					}

					key_path.pop_back();
					base::decrease_indent();
				}
			}

			void print()
			{
				switch (auto source_type = base::source().type())
				{
					case node_type::table:
					{
						auto& tbl = *reinterpret_cast<const table*>(&base::source());
						if (tbl.is_inline())
							print_inline(tbl);
						else
						{
							base::decrease_indent(); // so root kvps and tables have the same indent
							print(tbl);
							base::print_newline();
						}
						break;
					}

					case node_type::array:
						print(*reinterpret_cast<const array*>(&base::source()));
						break;

					default:
						base::print_value(base::source(), source_type);
				}
			}

		public:

			static constexpr format_flags default_flags
				= format_flags::allow_literal_strings
				| format_flags::allow_multi_line_strings
				| format_flags::allow_value_format_flags;

			TOML_NODISCARD_CTOR
			explicit default_formatter(const toml::node& source, format_flags flags = default_flags) noexcept
				: base{ source, flags }
			{}

			template <typename T, typename U>
			friend std::basic_ostream<T>& operator << (std::basic_ostream<T>&, default_formatter<U>&);
			template <typename T, typename U>
			friend std::basic_ostream<T>& operator << (std::basic_ostream<T>&, default_formatter<U>&&);
	};

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template class TOML_API default_formatter<char>;
	#endif

	default_formatter(const table&) -> default_formatter<char>;
	default_formatter(const array&) -> default_formatter<char>;
	template <typename T> default_formatter(const value<T>&) -> default_formatter<char>;

	template <typename T, typename U>
	inline std::basic_ostream<T>& operator << (std::basic_ostream<T>& lhs, default_formatter<U>& rhs)
	{
		rhs.attach(lhs);
		rhs.key_path.clear();
		rhs.print();
		rhs.detach();
		return lhs;
	}

	template <typename T, typename U>
	inline std::basic_ostream<T>& operator << (std::basic_ostream<T>& lhs, default_formatter<U>&& rhs)
	{
		return lhs << rhs; //as lvalue
	}

	#ifndef DOXYGEN

	#if !TOML_HEADER_ONLY
		extern template TOML_API std::ostream& operator << (std::ostream&, default_formatter<char>&);
		extern template TOML_API std::ostream& operator << (std::ostream&, default_formatter<char>&&);
		extern template TOML_API std::ostream& operator << (std::ostream&, const table&);
		extern template TOML_API std::ostream& operator << (std::ostream&, const array&);
		extern template TOML_API std::ostream& operator << (std::ostream&, const value<std::string>&);
		extern template TOML_API std::ostream& operator << (std::ostream&, const value<int64_t>&);
		extern template TOML_API std::ostream& operator << (std::ostream&, const value<double>&);
		extern template TOML_API std::ostream& operator << (std::ostream&, const value<bool>&);
		extern template TOML_API std::ostream& operator << (std::ostream&, const value<toml::date>&);
		extern template TOML_API std::ostream& operator << (std::ostream&, const value<toml::time>&);
		extern template TOML_API std::ostream& operator << (std::ostream&, const value<toml::date_time>&);
	#endif

	template <typename Char>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const table& rhs)
	{
		return lhs << default_formatter<Char>{ rhs };
	}

	template <typename Char>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const array& rhs)
	{
		return lhs << default_formatter<Char>{ rhs };
	}

	template <typename Char, typename T>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const value<T>& rhs)
	{
		return lhs << default_formatter<Char>{ rhs };
	}

	#endif // !DOXYGEN
}
TOML_NAMESPACE_END

TOML_POP_WARNINGS // TOML_DISABLE_SWITCH_WARNINGS

#endif //------------------------------------------------------------------------------  ↑ toml_default_formatter.h  ---

#if 1  //-----  ↓ toml_json_formatter.h  -------------------------------------------------------------------------------

TOML_PUSH_WARNINGS
TOML_DISABLE_SWITCH_WARNINGS

TOML_NAMESPACE_START
{
	template <typename Char = char>
	class TOML_API json_formatter final : impl::formatter<Char>
	{
		private:
			using base = impl::formatter<Char>;

			void print(const toml::table& tbl);
			void print(const array& arr)
			{
				if (arr.empty())
					impl::print_to_stream("[]"sv, base::stream());
				else
				{
					impl::print_to_stream('[', base::stream());
					base::increase_indent();
					for (size_t i = 0; i < arr.size(); i++)
					{
						if (i > 0_sz)
							impl::print_to_stream(',', base::stream());
						base::print_newline(true);
						base::print_indent();

						auto& v = arr[i];
						const auto type = v.type();
						TOML_ASSUME(type != node_type::none);
						switch (type)
						{
							case node_type::table: print(*reinterpret_cast<const table*>(&v)); break;
							case node_type::array: print(*reinterpret_cast<const array*>(&v)); break;
							default:
								base::print_value(v, type);
						}

					}
					base::decrease_indent();
					base::print_newline(true);
					base::print_indent();
					impl::print_to_stream(']', base::stream());
				}
				base::clear_naked_newline();
			}

			void print()
			{
				switch (auto source_type = base::source().type())
				{
					case node_type::table:
						print(*reinterpret_cast<const table*>(&base::source()));
						base::print_newline();
						break;

					case node_type::array:
						print(*reinterpret_cast<const array*>(&base::source()));
						break;

					default:
						base::print_value(base::source(), source_type);
				}
			}

		public:

			static constexpr format_flags default_flags = format_flags::quote_dates_and_times;

			TOML_NODISCARD_CTOR
				explicit json_formatter(const toml::node& source, format_flags flags = default_flags) noexcept
				: base{ source, flags }
			{}

			template <typename T, typename U>
			friend std::basic_ostream<T>& operator << (std::basic_ostream<T>&, json_formatter<U>&);
			template <typename T, typename U>
			friend std::basic_ostream<T>& operator << (std::basic_ostream<T>&, json_formatter<U>&&);
	};

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template class TOML_API json_formatter<char>;
	#endif

	json_formatter(const table&) -> json_formatter<char>;
	json_formatter(const array&) -> json_formatter<char>;
	template <typename T> json_formatter(const value<T>&) -> json_formatter<char>;

	template <typename T, typename U>
	inline std::basic_ostream<T>& operator << (std::basic_ostream<T>& lhs, json_formatter<U>& rhs)
	{
		rhs.attach(lhs);
		rhs.print();
		rhs.detach();
		return lhs;
	}

	template <typename T, typename U>
	inline std::basic_ostream<T>& operator << (std::basic_ostream<T>& lhs, json_formatter<U>&& rhs)
	{
		return lhs << rhs; //as lvalue
	}

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template TOML_API std::ostream& operator << (std::ostream&, json_formatter<char>&);
		extern template TOML_API std::ostream& operator << (std::ostream&, json_formatter<char>&&);
	#endif
}
TOML_NAMESPACE_END

TOML_POP_WARNINGS // TOML_DISABLE_SWITCH_WARNINGS

#endif //-----  ↑ toml_json_formatter.h  -------------------------------------------------------------------------------

#if TOML_PARSER

#if 1  //-------------------------------  ↓ toml_parse_error.h  --------------------------------------------------------

TOML_DISABLE_WARNINGS
#if TOML_EXCEPTIONS
	#include <stdexcept>
#endif
TOML_ENABLE_WARNINGS

TOML_PUSH_WARNINGS
TOML_DISABLE_INIT_WARNINGS

TOML_NAMESPACE_START
{
	TOML_ABI_NAMESPACE_BOOL(TOML_EXCEPTIONS, ex, noex)

	#if defined(DOXYGEN) || !TOML_EXCEPTIONS

	class parse_error final
	{
		private:
			std::string description_;
			source_region source_;

		public:

			TOML_NODISCARD_CTOR
			parse_error(std::string&& desc, source_region&& src) noexcept
				: description_{ std::move(desc) },
				source_{ std::move(src) }
			{}

			TOML_NODISCARD_CTOR
			parse_error(std::string&& desc, const source_region& src) noexcept
				: parse_error{ std::move(desc), source_region{ src } }
			{}

			TOML_NODISCARD_CTOR
				parse_error(std::string&& desc, const source_position& position, const source_path_ptr& path = {}) noexcept
				: parse_error{ std::move(desc), source_region{ position, position, path } }
			{}

			[[nodiscard]]
			std::string_view description() const noexcept
			{
				return description_;
			}

			[[nodiscard]]
			const source_region& source() const noexcept
			{
				return source_;
			}
	};

	#else

	class parse_error final
		: public std::runtime_error
	{
		private:
			source_region source_;

		public:

			TOML_NODISCARD_CTOR
			TOML_ATTR(nonnull)
			parse_error(const char* desc, source_region&& src) noexcept
				: std::runtime_error{ desc },
				source_{ std::move(src) }
			{}

			TOML_NODISCARD_CTOR
			TOML_ATTR(nonnull)
			parse_error(const char* desc, const source_region& src) noexcept
				: parse_error{ desc, source_region{ src } }
			{}

			TOML_NODISCARD_CTOR
			TOML_ATTR(nonnull)
			parse_error(const char* desc, const source_position& position, const source_path_ptr& path = {}) noexcept
				: parse_error{ desc, source_region{ position, position, path } }
			{}

			[[nodiscard]]
			std::string_view description() const noexcept
			{
				return std::string_view{ what() };
			}

			[[nodiscard]]
			const source_region& source() const noexcept
			{
				return source_;
			}
	};

	#endif

	TOML_ABI_NAMESPACE_END // TOML_EXCEPTIONS

	template <typename Char>
	inline std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& lhs, const parse_error& rhs)
	{
		lhs << rhs.description();
		lhs << "\n\t(error occurred at "sv;
		lhs << rhs.source();
		lhs << ")"sv;
		return lhs;
	}

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template TOML_API std::ostream& operator << (std::ostream&, const parse_error&);
	#endif
}
TOML_NAMESPACE_END

TOML_POP_WARNINGS // TOML_DISABLE_INIT_WARNINGS

#endif //-------------------------------  ↑ toml_parse_error.h  --------------------------------------------------------

#if 1  //--------------------------------------------------------  ↓ toml_utf8_streams.h  ------------------------------

TOML_IMPL_NAMESPACE_START
{
	template <typename T>
	class utf8_byte_stream;

	inline constexpr auto utf8_byte_order_mark = "\xEF\xBB\xBF"sv;

	template <typename Char>
	class TOML_API utf8_byte_stream<std::basic_string_view<Char>> final
	{
		static_assert(sizeof(Char) == 1_sz);

		private:
			std::basic_string_view<Char> source;
			size_t position = {};

		public:
			explicit constexpr utf8_byte_stream(std::basic_string_view<Char> sv) noexcept
				: source{ sv }
			{
				// trim trailing nulls
				size_t actual_len = source.length();
				for (size_t i = actual_len; i --> 0_sz;)
				{
					if (source[i] != Char{}) // not '\0'
					{
						actual_len = i + 1_sz;
						break;
					}
				}
				if (source.length() != actual_len) // not '\0'
					source = source.substr(0_sz, actual_len);

				// skip bom
				if (source.length() >= 3_sz && memcmp(utf8_byte_order_mark.data(), source.data(), 3_sz) == 0)
					position += 3_sz;
			}

			[[nodiscard]]
			TOML_ALWAYS_INLINE
			constexpr bool eof() const noexcept
			{
				return position >= source.length();
			}

			[[nodiscard]]
			TOML_ALWAYS_INLINE
			constexpr bool peek_eof() const noexcept
			{
				return eof();
			}

			[[nodiscard]]
			TOML_ALWAYS_INLINE
			constexpr bool error() const noexcept
			{
				return false;
			}

			[[nodiscard]]
			constexpr unsigned int operator() () noexcept
			{
				if (position >= source.length())
					return 0xFFFFFFFFu;
				return static_cast<unsigned int>(static_cast<uint8_t>(source[position++]));
			}
	};

	template <typename Char>
	class TOML_API utf8_byte_stream<std::basic_istream<Char>> final
	{
		static_assert(sizeof(Char) == 1_sz);

		private:
			std::basic_istream<Char>* source;

		public:
			explicit utf8_byte_stream(std::basic_istream<Char>& stream)
				: source{ &stream }
			{
				if (!source->good()) // eof, fail, bad
					return;

				const auto initial_pos = source->tellg();
				Char bom[3];
				source->read(bom, 3);
				if (source->bad() || (source->gcount() == 3 && memcmp(utf8_byte_order_mark.data(), bom, 3_sz) == 0))
					return;

				source->clear();
				source->seekg(initial_pos, std::basic_istream<Char>::beg);
			}

			[[nodiscard]]
			TOML_ALWAYS_INLINE
			bool eof() const noexcept
			{
				return source->eof();
			}

			[[nodiscard]]
			TOML_ALWAYS_INLINE
			bool peek_eof() const
			{
				using stream_traits = typename std::remove_pointer_t<decltype(source)>::traits_type;
				return eof() || source->peek() == stream_traits::eof();
			}

			[[nodiscard]]
			TOML_ALWAYS_INLINE
			bool error() const noexcept
			{
				return !(*source);
			}

			[[nodiscard]]
			unsigned int operator() ()
			{
				auto val = source->get();
				if (val == std::basic_istream<Char>::traits_type::eof())
					return 0xFFFFFFFFu;
				return static_cast<unsigned int>(val);
			}
	};

	TOML_ABI_NAMESPACE_BOOL(TOML_LARGE_FILES, lf, sf)

	struct utf8_codepoint final
	{
		char32_t value;
		char bytes[4];
		source_position position;

		[[nodiscard]]
		std::string_view as_view() const noexcept
		{
			return bytes[3]
				? std::string_view{ bytes, 4_sz }
				: std::string_view{ bytes };
		}

		[[nodiscard]] TOML_ATTR(pure) constexpr operator char32_t& () noexcept { return value; }
		[[nodiscard]] TOML_ATTR(pure) constexpr operator const char32_t& () const noexcept { return value; }
		[[nodiscard]] TOML_ATTR(pure) constexpr const char32_t& operator* () const noexcept { return value; }
	};
	static_assert(std::is_trivial_v<utf8_codepoint>);
	static_assert(std::is_standard_layout_v<utf8_codepoint>);

	TOML_ABI_NAMESPACE_END // TOML_LARGE_FILES

	TOML_ABI_NAMESPACE_BOOL(TOML_EXCEPTIONS, ex, noex)

	#if TOML_EXCEPTIONS
		#define TOML_ERROR_CHECK	(void)0
		#define TOML_ERROR			throw parse_error
	#else
		#define TOML_ERROR_CHECK	if (err) return nullptr
		#define TOML_ERROR			err.emplace
	#endif

	struct TOML_INTERFACE utf8_reader_interface
	{
		[[nodiscard]]
		virtual const source_path_ptr& source_path() const noexcept = 0;

		[[nodiscard]]
		virtual const utf8_codepoint* read_next() = 0;
		[[nodiscard]]
		virtual bool peek_eof() const = 0;

		#if !TOML_EXCEPTIONS

		[[nodiscard]]
		virtual optional<parse_error>&& error() noexcept = 0;

		#endif

		virtual ~utf8_reader_interface() noexcept = default;
	};

	template <typename T>
	class TOML_EMPTY_BASES TOML_API utf8_reader final
		: public utf8_reader_interface
	{
		private:
			utf8_byte_stream<T> stream;
			utf8_decoder decoder;
			utf8_codepoint codepoints[2];
			size_t cp_idx = 1;
			uint8_t current_byte_count{};
			source_path_ptr source_path_;
			#if !TOML_EXCEPTIONS
			optional<parse_error> err;
			#endif

		public:

			template <typename U, typename String = std::string_view>
			explicit utf8_reader(U && source, String&& source_path = {})
				noexcept(std::is_nothrow_constructible_v<utf8_byte_stream<T>, U&&>)
				: stream{ std::forward<U>(source) }
			{
				std::memset(codepoints, 0, sizeof(codepoints));
				codepoints[0].position = { 1, 1 };
				codepoints[1].position = { 1, 1 };

				if (!source_path.empty())
					source_path_ = std::make_shared<const std::string>(std::forward<String>(source_path));
			}

			[[nodiscard]]
			const source_path_ptr& source_path() const noexcept override
			{
				return source_path_;
			}

			[[nodiscard]]
			const utf8_codepoint* read_next() override
			{
				TOML_ERROR_CHECK;

				auto& prev = codepoints[(cp_idx - 1_sz) % 2_sz];

				if (stream.eof())
					return nullptr;
				else if (stream.error())
					TOML_ERROR("An error occurred while reading from the underlying stream", prev.position, source_path_ );
				else if (decoder.error())
					TOML_ERROR( "Encountered invalid utf-8 sequence", prev.position, source_path_ );

				TOML_ERROR_CHECK;

				while (true)
				{
					uint8_t next_byte;
					{
						unsigned int next_byte_raw{ 0xFFFFFFFFu };
						if constexpr (noexcept(stream()) || !TOML_EXCEPTIONS)
						{
							next_byte_raw = stream();
						}
						#if TOML_EXCEPTIONS
						else
						{
							try
							{
								next_byte_raw = stream();
							}
							catch (const std::exception& exc)
							{
								throw parse_error{ exc.what(), prev.position, source_path_ };
							}
							catch (...)
							{
								throw parse_error{ "An unspecified error occurred", prev.position, source_path_ };
							}
						}
						#endif

						if (next_byte_raw >= 256u)
						{
							if (stream.eof())
							{
								if (decoder.needs_more_input())
									TOML_ERROR("Encountered EOF during incomplete utf-8 code point sequence",
										prev.position, source_path_);
								return nullptr;
							}
							else
								TOML_ERROR("An error occurred while reading from the underlying stream",
									prev.position, source_path_);
						}

						TOML_ERROR_CHECK;
						next_byte = static_cast<uint8_t>(next_byte_raw);
					}

					decoder(next_byte);
					if (decoder.error())
						TOML_ERROR( "Encountered invalid utf-8 sequence", prev.position, source_path_ );

					TOML_ERROR_CHECK;

					auto& current = codepoints[cp_idx % 2_sz];
					current.bytes[current_byte_count++] = static_cast<char>(next_byte);
					if (decoder.has_code_point())
					{
						//store codepoint
						current.value = decoder.codepoint;

						//reset prev (will be the next 'current')
						std::memset(prev.bytes, 0, sizeof(prev.bytes));
						current_byte_count = {};
						if (is_line_break<false>(current.value))
							prev.position = { static_cast<source_index>(current.position.line + 1), 1 };
						else
							prev.position = { current.position.line, static_cast<source_index>(current.position.column + 1) };
						cp_idx++;
						return &current;
					}
				}

				TOML_UNREACHABLE;
			}

			[[nodiscard]]
			bool peek_eof() const override
			{
				return stream.peek_eof();
			}

			#if !TOML_EXCEPTIONS

			[[nodiscard]]
			optional<parse_error>&& error() noexcept override
			{
				return std::move(err);
			}

			#endif
	};

	template <typename Char>
	utf8_reader(std::basic_string_view<Char>, std::string_view) -> utf8_reader<std::basic_string_view<Char>>;
	template <typename Char>
	utf8_reader(std::basic_string_view<Char>, std::string&&) -> utf8_reader<std::basic_string_view<Char>>;
	template <typename Char>
	utf8_reader(std::basic_istream<Char>&, std::string_view) -> utf8_reader<std::basic_istream<Char>>;
	template <typename Char>
	utf8_reader(std::basic_istream<Char>&, std::string&&) -> utf8_reader<std::basic_istream<Char>>;

	class TOML_EMPTY_BASES TOML_API utf8_buffered_reader final
		: public utf8_reader_interface
	{
		public:
			static constexpr size_t max_history_length = 72;

		private:
			static constexpr size_t history_buffer_size = max_history_length - 1; //'head' is stored in the reader
			utf8_reader_interface& reader;
			struct
			{
				utf8_codepoint buffer[history_buffer_size];
				size_t count, first;
			}
			history = {};
			const utf8_codepoint* head = {};
			size_t negative_offset = {};

		public:
			explicit utf8_buffered_reader(utf8_reader_interface& reader_) noexcept;
			const source_path_ptr& source_path() const noexcept override;
			const utf8_codepoint* read_next() override;
			const utf8_codepoint* step_back(size_t count) noexcept;
			bool peek_eof() const override;
			#if !TOML_EXCEPTIONS
			optional<parse_error>&& error() noexcept override;
			#endif
	};

	TOML_ABI_NAMESPACE_END // TOML_EXCEPTIONS
}
TOML_IMPL_NAMESPACE_END

#endif //--------------------------------------------------------  ↑ toml_utf8_streams.h  ------------------------------

#if 1  //------------------------------------------------------------------------------------  ↓ toml_parser.h  --------

TOML_NAMESPACE_START
{
	TOML_ABI_NAMESPACE_BOOL(TOML_EXCEPTIONS, ex, noex)

	#if defined(DOXYGEN) || !TOML_EXCEPTIONS

	class parse_result final
	{
		private:
			std::aligned_storage_t<
				(sizeof(toml::table) < sizeof(parse_error) ? sizeof(parse_error) : sizeof(toml::table)),
				(alignof(toml::table) < alignof(parse_error) ? alignof(parse_error) : alignof(toml::table))
			> storage;
			bool is_err;
			void destroy() noexcept
			{
				if (is_err)
					TOML_LAUNDER(reinterpret_cast<parse_error*>(&storage))->~parse_error();
				else
					TOML_LAUNDER(reinterpret_cast<toml::table*>(&storage))->~table();
			}

		public:

			using iterator = table_iterator;
			using const_iterator = const_table_iterator;
			[[nodiscard]] bool succeeded() const noexcept { return !is_err; }
			[[nodiscard]] bool failed() const noexcept { return is_err; }
			[[nodiscard]] explicit operator bool() const noexcept { return !is_err; }

			[[nodiscard]]
			toml::table& table() & noexcept
			{
				TOML_ASSERT(!is_err);
				return *TOML_LAUNDER(reinterpret_cast<toml::table*>(&storage));
			}
			[[nodiscard]]
			toml::table&& table() && noexcept
			{
				TOML_ASSERT(!is_err);
				return std::move(*TOML_LAUNDER(reinterpret_cast<toml::table*>(&storage)));
			}
			[[nodiscard]]
			const toml::table& table() const& noexcept
			{
				TOML_ASSERT(!is_err);
				return *TOML_LAUNDER(reinterpret_cast<const toml::table*>(&storage));
			}

			[[nodiscard, deprecated("use parse_result::table() instead")]]
			toml::table& get() & noexcept { return table(); }
			[[nodiscard, deprecated("use parse_result::table() instead")]]
			toml::table&& get() && noexcept { return std::move(table()); }
			[[nodiscard, deprecated("use parse_result::table() instead")]]
			const toml::table& get() const& noexcept { return table(); }
			[[nodiscard]]
			parse_error& error() & noexcept
			{
				TOML_ASSERT(is_err);
				return *TOML_LAUNDER(reinterpret_cast<parse_error*>(&storage));
			}
			[[nodiscard]]
			parse_error&& error() && noexcept
			{
				TOML_ASSERT(is_err);
				return std::move(*TOML_LAUNDER(reinterpret_cast<parse_error*>(&storage)));
			}
			[[nodiscard]]
			const parse_error& error() const& noexcept
			{
				TOML_ASSERT(is_err);
				return *TOML_LAUNDER(reinterpret_cast<const parse_error*>(&storage));
			}

			[[nodiscard]] operator toml::table& () noexcept { return table(); }
			[[nodiscard]] operator toml::table&& () noexcept { return std::move(table()); }
			[[nodiscard]] operator const toml::table& () const noexcept { return table(); }
			[[nodiscard]] explicit operator parse_error& () noexcept { return error(); }
			[[nodiscard]] explicit operator parse_error && () noexcept { return std::move(error()); }
			[[nodiscard]] explicit operator const parse_error& () const noexcept { return error(); }

			TOML_NODISCARD_CTOR
			explicit parse_result(toml::table&& tbl) noexcept
				: is_err{ false }
			{
				::new (&storage) toml::table{ std::move(tbl) };
			}

			TOML_NODISCARD_CTOR
			explicit parse_result(parse_error&& err) noexcept
				: is_err{ true }
			{
				::new (&storage) parse_error{ std::move(err) };
			}

			TOML_NODISCARD_CTOR
			parse_result(parse_result&& res) noexcept
				: is_err{ res.is_err }
			{
				if (is_err)
					::new (&storage) parse_error{ std::move(res).error() };
				else
					::new (&storage) toml::table{ std::move(res).table() };
			}

			parse_result& operator=(parse_result&& rhs) noexcept
			{
				if (is_err != rhs.is_err)
				{
					destroy();
					is_err = rhs.is_err;
					if (is_err)
						::new (&storage) parse_error{ std::move(rhs).error() };
					else
						::new (&storage) toml::table{ std::move(rhs).table() };
				}
				else
				{
					if (is_err)
						error() = std::move(rhs).error();
					else
						table() = std::move(rhs).table();
				}
				return *this;
			}

			~parse_result() noexcept
			{
				destroy();
			}

			[[nodiscard]]
			node_view<node> operator[] (string_view key) noexcept
			{
				return is_err ? node_view<node>{} : table()[key];
			}

			[[nodiscard]]
			node_view<const node> operator[] (string_view key) const noexcept
			{
				return is_err ? node_view<const node>{} : table()[key];
			}

			#if TOML_WINDOWS_COMPAT

			[[nodiscard]]
			node_view<node> operator[] (std::wstring_view key) noexcept
			{
				return is_err ? node_view<node>{} : table()[key];
			}

			[[nodiscard]]
			node_view<const node> operator[] (std::wstring_view key) const noexcept
			{
				return is_err ? node_view<const node>{} : table()[key];
			}

			#endif // TOML_WINDOWS_COMPAT

			[[nodiscard]]
			table_iterator begin() noexcept
			{
				return is_err ? table_iterator{} : table().begin();
			}

			[[nodiscard]]
			const_table_iterator begin() const noexcept
			{
				return is_err ? const_table_iterator{} : table().begin();
			}

			[[nodiscard]]
			const_table_iterator cbegin() const noexcept
			{
				return is_err ? const_table_iterator{} : table().cbegin();
			}

			[[nodiscard]]
			table_iterator end() noexcept
			{
				return is_err ? table_iterator{} : table().end();
			}

			[[nodiscard]]
			const_table_iterator end() const noexcept
			{
				return is_err ? const_table_iterator{} : table().end();
			}

			[[nodiscard]]
			const_table_iterator cend() const noexcept
			{
				return is_err ? const_table_iterator{} : table().cend();
			}
	};

	#else

	using parse_result = table;

	#endif

	TOML_ABI_NAMESPACE_END // TOML_EXCEPTIONS
}
TOML_NAMESPACE_END

TOML_IMPL_NAMESPACE_START
{
	TOML_ABI_NAMESPACE_BOOL(TOML_EXCEPTIONS, ex, noex)

	[[nodiscard]] TOML_API parse_result do_parse(utf8_reader_interface&&) TOML_MAY_THROW;

	TOML_ABI_NAMESPACE_END // TOML_EXCEPTIONS
}
TOML_IMPL_NAMESPACE_END

#if TOML_EXCEPTIONS
	#define TOML_THROW_PARSE_ERROR(msg, path)												\
		throw parse_error{																	\
			msg, source_position{}, std::make_shared<const std::string>(std::move(path))	\
		}
#else
	#define TOML_THROW_PARSE_ERROR(msg, path)												\
		return parse_result{ parse_error{													\
			msg, source_position{}, std::make_shared<const std::string>(std::move(path))	\
		}}
#endif

TOML_NAMESPACE_START
{
	TOML_ABI_NAMESPACE_BOOL(TOML_EXCEPTIONS, ex, noex)

	[[nodiscard]]
	TOML_API
	parse_result parse(std::string_view doc, std::string_view source_path = {}) TOML_MAY_THROW;

	[[nodiscard]]
	TOML_API
	parse_result parse(std::string_view doc, std::string&& source_path) TOML_MAY_THROW;

	#if TOML_WINDOWS_COMPAT

	[[nodiscard]]
	TOML_API
	parse_result parse(std::string_view doc, std::wstring_view source_path) TOML_MAY_THROW;

	#endif // TOML_WINDOWS_COMPAT

	#ifdef __cpp_lib_char8_t

	[[nodiscard]]
	TOML_API
	parse_result parse(std::u8string_view doc, std::string_view source_path = {}) TOML_MAY_THROW;

	[[nodiscard]]
	TOML_API
	parse_result parse(std::u8string_view doc, std::string&& source_path) TOML_MAY_THROW;

	#if TOML_WINDOWS_COMPAT

	[[nodiscard]]
	TOML_API
	parse_result parse(std::u8string_view doc, std::wstring_view source_path) TOML_MAY_THROW;

	#endif // TOML_WINDOWS_COMPAT

	#endif // __cpp_lib_char8_t

	template <typename Char>
	[[nodiscard]]
	inline parse_result parse(std::basic_istream<Char>& doc, std::string_view source_path = {}) TOML_MAY_THROW
	{
		static_assert(
			sizeof(Char) == 1,
			"The stream's underlying character type must be 1 byte in size."
		);

		return impl::do_parse(impl::utf8_reader{ doc, source_path });
	}

	template <typename Char>
	[[nodiscard]]
	inline parse_result parse(std::basic_istream<Char>& doc, std::string&& source_path) TOML_MAY_THROW
	{
		static_assert(
			sizeof(Char) == 1,
			"The stream's underlying character type must be 1 byte in size."
		);

		return impl::do_parse(impl::utf8_reader{ doc, std::move(source_path) });
	}

	#if TOML_WINDOWS_COMPAT

	template <typename Char>
	[[nodiscard]]
	inline parse_result parse(std::basic_istream<Char>& doc, std::wstring_view source_path) TOML_MAY_THROW
	{
		return parse(doc, impl::narrow(source_path));
	}

	#endif // TOML_WINDOWS_COMPAT

	// Q: "why are the parse_file functions templated??"
	// A: I don't want to force users to drag in <fstream> if they're not going to do
	//    any parsing directly from files. Keeping them templated delays their instantiation
	//    until they're actually required, so only those users wanting to use parse_file()
	//    are burdened by the <fstream> overhead.

	template <typename Char, typename StreamChar = char>
	[[nodiscard]]
	inline parse_result parse_file(std::basic_string_view<Char> file_path) TOML_MAY_THROW
	{
		static_assert(
			!std::is_same_v<Char, wchar_t> || TOML_WINDOWS_COMPAT,
			"Wide-character file paths are only supported on Windows with TOML_WINDOWS_COMPAT enabled."
		);
		#if TOML_WINDOWS_COMPAT
			static_assert(
				sizeof(Char) == 1 || std::is_same_v<Char, wchar_t>,
				"The file path's underlying character type must be wchar_t or be 1 byte in size."
			);
		#else
			static_assert(
				sizeof(Char) == 1,
				"The file path's underlying character type must be 1 byte in size."
			);
		#endif
		static_assert(
			std::is_same_v<StreamChar, char>,
			"StreamChar must be 'char' (it is as an instantiation-delaying hack and is not user-configurable)."
		);

		std::string file_path_str;
		#if TOML_WINDOWS_COMPAT
		if constexpr (std::is_same_v<Char, wchar_t>)
			file_path_str = impl::narrow(file_path);
		else
		#endif
			file_path_str = std::string_view{ reinterpret_cast<const char*>(file_path.data()), file_path.length() };

		// open file with a custom-sized stack buffer
		using ifstream = std::basic_ifstream<StreamChar>;
		ifstream file;
		StreamChar file_buffer[sizeof(void*) * 4096_sz];
		file.rdbuf()->pubsetbuf(file_buffer, sizeof(file_buffer));
		file.open(file_path_str, ifstream::in | ifstream::binary | ifstream::ate);
		if (!file.is_open())
			TOML_THROW_PARSE_ERROR("File could not be opened for reading", file_path_str);

		// get size
		const auto file_size = file.tellg();
		if (file_size == -1)
			TOML_THROW_PARSE_ERROR("Could not determine file size", file_path_str);
		file.seekg(0, ifstream::beg);

		// read the whole file into memory first if the file isn't too large
		constexpr auto large_file_threshold = 1024 * 1024 * static_cast<int>(sizeof(void*)) * 4; // 32 megabytes on 64-bit
		if (file_size <= large_file_threshold)
		{
			std::vector<StreamChar> file_data;
			file_data.resize(static_cast<size_t>(file_size));
			file.read(file_data.data(), static_cast<std::streamsize>(file_size));
			return parse(std::basic_string_view<StreamChar>{ file_data.data(), file_data.size() }, std::move(file_path_str));
		}

		// otherwise parse it using the streams
		else
			return parse(file, std::move(file_path_str));
	}

	#if !defined(DOXYGEN) && !TOML_HEADER_ONLY
		extern template TOML_API parse_result parse(std::istream&, std::string_view) TOML_MAY_THROW;
		extern template TOML_API parse_result parse(std::istream&, std::string&&) TOML_MAY_THROW;
		extern template TOML_API parse_result parse_file(std::string_view) TOML_MAY_THROW;
		#ifdef __cpp_lib_char8_t
			extern template TOML_API parse_result parse_file(std::u8string_view) TOML_MAY_THROW;
		#endif
		#if TOML_WINDOWS_COMPAT
			extern template TOML_API parse_result parse_file(std::wstring_view) TOML_MAY_THROW;
		#endif
	#endif

	template <typename Char>
	[[nodiscard]]
	inline parse_result parse_file(const std::basic_string<Char>& file_path) TOML_MAY_THROW
	{
		return parse_file(std::basic_string_view<Char>{ file_path });
	}

	template <typename Char>
	[[nodiscard]]
	inline parse_result parse_file(const Char* file_path) TOML_MAY_THROW
	{
		return parse_file(std::basic_string_view<Char>{ file_path });
	}

	inline namespace literals
	{
		[[nodiscard]]
		TOML_API
		parse_result operator"" _toml(const char* str, size_t len) TOML_MAY_THROW;

		#ifdef __cpp_lib_char8_t

		[[nodiscard]]
		TOML_API
		parse_result operator"" _toml(const char8_t* str, size_t len) TOML_MAY_THROW;

		#endif // __cpp_lib_char8_t

	}

	TOML_ABI_NAMESPACE_END // TOML_EXCEPTIONS
}
TOML_NAMESPACE_END

#undef TOML_THROW_PARSE_ERROR

#endif //------------------------------------------------------------------------------------  ↑ toml_parser.h  --------

#endif // TOML_PARSER

#if TOML_IMPLEMENTATION

#if 1  //---------  ↓ toml_node.hpp  -----------------------------------------------------------------------------------

TOML_NAMESPACE_START
{
	TOML_EXTERNAL_LINKAGE
	node::node(const node& /*other*/) noexcept
	{
		// does not copy source information
		// this is not an error
	}

	TOML_EXTERNAL_LINKAGE
	node::node(node && other) noexcept
		: source_{ std::move(other.source_) }
	{
		other.source_.begin = {};
		other.source_.end = {};
	}

	TOML_EXTERNAL_LINKAGE
	node& node::operator= (const node& /*rhs*/) noexcept
	{
		// does not copy source information
		// this is not an error

		source_ = {};
		return *this;
	}

	TOML_EXTERNAL_LINKAGE
	node& node::operator= (node && rhs) noexcept
	{
		source_ = std::move(rhs.source_);
		rhs.source_.begin = {};
		rhs.source_.end = {};
		return *this;
	}

	#define TOML_MEMBER_ATTR(attr) TOML_EXTERNAL_LINKAGE TOML_ATTR(attr)

	TOML_MEMBER_ATTR(const) bool node::is_string()			const noexcept { return false; }
	TOML_MEMBER_ATTR(const) bool node::is_integer()			const noexcept { return false; }
	TOML_MEMBER_ATTR(const) bool node::is_floating_point()	const noexcept { return false; }
	TOML_MEMBER_ATTR(const) bool node::is_number()			const noexcept { return false; }
	TOML_MEMBER_ATTR(const) bool node::is_boolean()			const noexcept { return false; }
	TOML_MEMBER_ATTR(const) bool node::is_date()			const noexcept { return false; }
	TOML_MEMBER_ATTR(const) bool node::is_time()			const noexcept { return false; }
	TOML_MEMBER_ATTR(const) bool node::is_date_time()		const noexcept { return false; }
	TOML_MEMBER_ATTR(const) bool node::is_array_of_tables()	const noexcept { return false; }

	TOML_MEMBER_ATTR(const) table* node::as_table()						noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) array* node::as_array()						noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) value<std::string>* node::as_string()		noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) value<int64_t>* node::as_integer()			noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) value<double>* node::as_floating_point()	noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) value<bool>* node::as_boolean()				noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) value<date>* node::as_date()				noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) value<time>* node::as_time()				noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) value<date_time>* node::as_date_time()		noexcept { return nullptr; }

	TOML_MEMBER_ATTR(const) const table* node::as_table()					const noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) const array* node::as_array()					const noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) const value<std::string>* node::as_string()		const noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) const value<int64_t>* node::as_integer()		const noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) const value<double>* node::as_floating_point()	const noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) const value<bool>* node::as_boolean()			const noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) const value<date>* node::as_date()				const noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) const value<time>* node::as_time()				const noexcept { return nullptr; }
	TOML_MEMBER_ATTR(const) const value<date_time>* node::as_date_time()	const noexcept { return nullptr; }

	TOML_MEMBER_ATTR(const) const source_region& node::source()				const noexcept { return source_; }

	#undef TOML_MEMBER_ATTR

	TOML_EXTERNAL_LINKAGE
	node::operator node_view<node>() noexcept
	{
		return node_view<node>(this);
	}

	TOML_EXTERNAL_LINKAGE
	node::operator node_view<const node>() const noexcept
	{
		return node_view<const node>(this);
	}
}
TOML_NAMESPACE_END

#endif //---------  ↑ toml_node.hpp  -----------------------------------------------------------------------------------

#if 1  //---------------------------------  ↓ toml_array.hpp  ----------------------------------------------------------

TOML_NAMESPACE_START
{
	TOML_EXTERNAL_LINKAGE
	array::array() noexcept = default;

	TOML_EXTERNAL_LINKAGE
	array::array(const array& other) noexcept
		: node{ other }
	{
		elements.reserve(other.elements.size());
		for (const auto& elem : other)
			elements.emplace_back(impl::make_node(elem));
	}

	TOML_EXTERNAL_LINKAGE
	array::array(array&& other) noexcept
		: node{ std::move(other) },
		elements{ std::move(other.elements) }
	{}

	TOML_EXTERNAL_LINKAGE
	array& array::operator= (const array& rhs) noexcept
	{
		if (&rhs != this)
		{
			node::operator=(rhs);
			elements.clear();
			elements.reserve(rhs.elements.size());
			for (const auto& elem : rhs)
				elements.emplace_back(impl::make_node(elem));
		}
		return *this;
	}

	TOML_EXTERNAL_LINKAGE
	array& array::operator= (array&& rhs) noexcept
	{
		if (&rhs != this)
		{
			node::operator=(std::move(rhs));
			elements = std::move(rhs.elements);
		}
		return *this;
	}

	TOML_EXTERNAL_LINKAGE
	void array::preinsertion_resize(size_t idx, size_t count) noexcept
	{
		TOML_ASSERT(idx <= elements.size());
		TOML_ASSERT(count >= 1_sz);
		const auto old_size = elements.size();
		const auto new_size = old_size + count;
		const auto inserting_at_end = idx == old_size;
		elements.resize(new_size);
		if (!inserting_at_end)
		{
			for(size_t left = old_size, right = new_size - 1_sz; left --> idx; right--)
				elements[right] = std::move(elements[left]);
		}
	}

	#define TOML_MEMBER_ATTR(attr) TOML_EXTERNAL_LINKAGE TOML_ATTR(attr)

	TOML_MEMBER_ATTR(const) node_type array::type()			const noexcept	{ return node_type::array; }
	TOML_MEMBER_ATTR(const) bool array::is_table()			const noexcept	{ return false; }
	TOML_MEMBER_ATTR(const) bool array::is_array()			const noexcept	{ return true; }
	TOML_MEMBER_ATTR(const) bool array::is_value()			const noexcept	{ return false; }
	TOML_MEMBER_ATTR(const) const array* array::as_array()	const noexcept	{ return this; }
	TOML_MEMBER_ATTR(const) array* array::as_array()		noexcept		{ return this; }

	TOML_MEMBER_ATTR(pure) const node& array::operator[] (size_t index)	const noexcept	{ return *elements[index]; }
	TOML_MEMBER_ATTR(pure) node& array::operator[] (size_t index)		noexcept		{ return *elements[index]; }

	TOML_MEMBER_ATTR(pure) const node& array::front()				const noexcept	{ return *elements.front(); }
	TOML_MEMBER_ATTR(pure) const node& array::back()				const noexcept	{ return *elements.back(); }
	TOML_MEMBER_ATTR(pure) node& array::front()						noexcept		{ return *elements.front(); }
	TOML_MEMBER_ATTR(pure) node& array::back()						noexcept		{ return *elements.back(); }

	TOML_MEMBER_ATTR(pure) array::const_iterator array::begin()		const noexcept	{ return { elements.begin() }; }
	TOML_MEMBER_ATTR(pure) array::const_iterator array::end()		const noexcept	{ return { elements.end() }; }
	TOML_MEMBER_ATTR(pure) array::const_iterator array::cbegin()	const noexcept	{ return { elements.cbegin() }; }
	TOML_MEMBER_ATTR(pure) array::const_iterator array::cend()		const noexcept	{ return { elements.cend() }; }
	TOML_MEMBER_ATTR(pure) array::iterator array::begin()			noexcept		{ return { elements.begin() }; }
	TOML_MEMBER_ATTR(pure) array::iterator array::end()				noexcept		{ return { elements.end() }; }

	TOML_MEMBER_ATTR(pure) size_t array::size()						const noexcept	{ return elements.size(); }
	TOML_MEMBER_ATTR(pure) size_t array::capacity()					const noexcept	{ return elements.capacity(); }
	TOML_MEMBER_ATTR(pure) bool array::empty()						const noexcept	{ return elements.empty(); }
	TOML_MEMBER_ATTR(const) size_t array::max_size()				const noexcept	{ return elements.max_size(); }

	TOML_EXTERNAL_LINKAGE void array::reserve(size_t new_capacity)	{ elements.reserve(new_capacity); }
	TOML_EXTERNAL_LINKAGE void array::clear() noexcept				{ elements.clear(); }
	TOML_EXTERNAL_LINKAGE void array::shrink_to_fit()				{ elements.shrink_to_fit(); }

	#undef TOML_MEMBER_ATTR

	TOML_EXTERNAL_LINKAGE
	bool array::is_homogeneous(node_type ntype) const noexcept
	{
		if (elements.empty())
			return false;

		if (ntype == node_type::none)
			ntype = elements[0]->type();

		for (const auto& val : elements)
			if (val->type() != ntype)
				return false;
		return true;
	}

	namespace impl
	{
		template <typename T, typename U>
		TOML_INTERNAL_LINKAGE
		bool array_is_homogeneous(T& elements, node_type ntype, U& first_nonmatch) noexcept
		{
			if (elements.empty())
			{
				first_nonmatch = {};
				return false;
			}
			if (ntype == node_type::none)
				ntype = elements[0]->type();
			for (const auto& val : elements)
			{
				if (val->type() != ntype)
				{
					first_nonmatch = val.get();
					return false;
				}
			}
			return true;
		}
	}

	TOML_EXTERNAL_LINKAGE
	bool array::is_homogeneous(node_type ntype, toml::node*& first_nonmatch) noexcept
	{
		return impl::array_is_homogeneous(elements, ntype, first_nonmatch);
	}

	TOML_EXTERNAL_LINKAGE
	bool array::is_homogeneous(node_type ntype, const toml::node*& first_nonmatch) const noexcept
	{
		return impl::array_is_homogeneous(elements, ntype, first_nonmatch);
	}

	TOML_EXTERNAL_LINKAGE
	void array::truncate(size_t new_size)
	{
		if (new_size < elements.size())
			elements.resize(new_size);
	}

	TOML_EXTERNAL_LINKAGE
	array::iterator array::erase(const_iterator pos) noexcept
	{
		return { elements.erase(pos.raw_) };
	}

	TOML_EXTERNAL_LINKAGE
	array::iterator array::erase(const_iterator first, const_iterator last) noexcept
	{
		return { elements.erase(first.raw_, last.raw_) };
	}

	TOML_EXTERNAL_LINKAGE
	void array::pop_back() noexcept
	{
		elements.pop_back();
	}

	TOML_EXTERNAL_LINKAGE
	TOML_ATTR(pure)
	node* array::get(size_t index) noexcept
	{
		return index < elements.size() ? elements[index].get() : nullptr;
	}

	TOML_EXTERNAL_LINKAGE
	TOML_ATTR(pure)
	const node* array::get(size_t index) const noexcept
	{
		return index < elements.size() ? elements[index].get() : nullptr;
	}

	TOML_API
	TOML_EXTERNAL_LINKAGE
	bool operator == (const array& lhs, const array& rhs) noexcept
	{
		if (&lhs == &rhs)
			return true;
		if (lhs.elements.size() != rhs.elements.size())
			return false;
		for (size_t i = 0, e = lhs.elements.size(); i < e; i++)
		{
			const auto lhs_type = lhs.elements[i]->type();
			const node& rhs_ = *rhs.elements[i];
			const auto rhs_type = rhs_.type();
			if (lhs_type != rhs_type)
				return false;

			const bool equal = lhs.elements[i]->visit([&](const auto& lhs_) noexcept
			{
				return lhs_ == *reinterpret_cast<std::remove_reference_t<decltype(lhs_)>*>(&rhs_);
			});
			if (!equal)
				return false;
		}
		return true;
	}

	TOML_API
	TOML_EXTERNAL_LINKAGE
	bool operator != (const array& lhs, const array& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	TOML_EXTERNAL_LINKAGE
	size_t array::total_leaf_count() const noexcept
	{
		size_t leaves{};
		for (size_t i = 0, e = elements.size(); i < e; i++)
		{
			auto arr = elements[i]->as_array();
			leaves += arr ? arr->total_leaf_count() : 1_sz;
		}
		return leaves;
	}

	TOML_EXTERNAL_LINKAGE
	void array::flatten_child(array&& child, size_t& dest_index) noexcept
	{
		for (size_t i = 0, e = child.size(); i < e; i++)
		{
			auto type = child.elements[i]->type();
			if (type == node_type::array)
			{
				array& arr = *reinterpret_cast<array*>(child.elements[i].get());
				if (!arr.empty())
					flatten_child(std::move(arr), dest_index);
			}
			else
				elements[dest_index++] = std::move(child.elements[i]);
		}
	}

	TOML_EXTERNAL_LINKAGE
	array& array::flatten() &
	{
		if (elements.empty())
			return *this;

		bool requires_flattening = false;
		size_t size_after_flattening = elements.size();
		for (size_t i = elements.size(); i --> 0_sz;)
		{
			auto arr = elements[i]->as_array();
			if (!arr)
				continue;
			size_after_flattening--; //discount the array itself
			const auto leaf_count = arr->total_leaf_count();
			if (leaf_count > 0_sz)
			{
				requires_flattening = true;
				size_after_flattening += leaf_count;
			}
			else
				elements.erase(elements.cbegin() + static_cast<ptrdiff_t>(i));
		}

		if (!requires_flattening)
			return *this;

		elements.reserve(size_after_flattening);

		size_t i = 0;
		while (i < elements.size())
		{
			auto arr = elements[i]->as_array();
			if (!arr)
			{
				i++;
				continue;
			}

			std::unique_ptr<node> arr_storage = std::move(elements[i]);
			const auto leaf_count = arr->total_leaf_count();
			if (leaf_count > 1_sz)
				preinsertion_resize(i + 1_sz, leaf_count - 1_sz);
			flatten_child(std::move(*arr), i); //increments i
		}

		return *this;
	}

	TOML_EXTERNAL_LINKAGE
	bool array::is_array_of_tables() const noexcept
	{
		return is_homogeneous(node_type::table);
	}
}
TOML_NAMESPACE_END

#endif //---------------------------------  ↑ toml_array.hpp  ----------------------------------------------------------

#if 1  //----------------------------------------------------------  ↓ toml_table.hpp  ---------------------------------

TOML_NAMESPACE_START
{
	TOML_EXTERNAL_LINKAGE
	table::table() noexcept {}

	TOML_EXTERNAL_LINKAGE
	table::table(const table& other) noexcept
		: node{ std::move(other) },
		inline_{ other.inline_ }
	{
		for (auto&& [k, v] : other)
			map.emplace_hint(map.end(), k, impl::make_node(v));
	}

	TOML_EXTERNAL_LINKAGE
	table::table(table&& other) noexcept
		: node{ std::move(other) },
		map{ std::move(other.map) },
		inline_{ other.inline_ }
	{}

	TOML_EXTERNAL_LINKAGE
	table& table::operator= (const table& rhs) noexcept
	{
		if (&rhs != this)
		{
			node::operator=(rhs);
			map.clear();
			for (auto&& [k, v] : rhs)
				map.emplace_hint(map.end(), k, impl::make_node(v));
			inline_ = rhs.inline_;
		}
		return *this;
	}

	TOML_EXTERNAL_LINKAGE
	table& table::operator= (table&& rhs) noexcept
	{
		if (&rhs != this)
		{
			node::operator=(std::move(rhs));
			map = std::move(rhs.map);
			inline_ = rhs.inline_;
		}
		return *this;
	}

	TOML_EXTERNAL_LINKAGE
	table::table(impl::table_init_pair* pairs, size_t count) noexcept
	{
		for (size_t i = 0; i < count; i++)
		{
			if (!pairs[i].value) // empty node_views
				continue;
			map.insert_or_assign(
				std::move(pairs[i].key),
				std::move(pairs[i].value)
			);
		}
	}

	#define TOML_MEMBER_ATTR(attr) TOML_EXTERNAL_LINKAGE TOML_ATTR(attr)

	TOML_MEMBER_ATTR(const) node_type table::type()				const noexcept	{ return node_type::table; }
	TOML_MEMBER_ATTR(const) bool table::is_table()				const noexcept	{ return true; }
	TOML_MEMBER_ATTR(const) bool table::is_array()				const noexcept	{ return false; }
	TOML_MEMBER_ATTR(const) bool table::is_value()				const noexcept	{ return false; }
	TOML_MEMBER_ATTR(const) const table* table::as_table()		const noexcept	{ return this; }
	TOML_MEMBER_ATTR(const) table* table::as_table()			noexcept		{ return this; }

	TOML_MEMBER_ATTR(pure) bool table::is_inline()				const noexcept	{ return inline_; }
	TOML_EXTERNAL_LINKAGE void table::is_inline(bool val)		noexcept		{ inline_ = val; }

	TOML_EXTERNAL_LINKAGE table::const_iterator table::begin()	const noexcept	{ return { map.begin() }; }
	TOML_EXTERNAL_LINKAGE table::const_iterator table::end()	const noexcept	{ return { map.end() }; }
	TOML_EXTERNAL_LINKAGE table::const_iterator table::cbegin()	const noexcept	{ return { map.cbegin() }; }
	TOML_EXTERNAL_LINKAGE table::const_iterator table::cend()	const noexcept	{ return { map.cend() }; }
	TOML_EXTERNAL_LINKAGE table::iterator table::begin()		noexcept		{ return { map.begin() }; }
	TOML_EXTERNAL_LINKAGE table::iterator table::end()			noexcept		{ return { map.end() }; }

	TOML_MEMBER_ATTR(pure) bool table::empty()					const noexcept	{ return map.empty(); }
	TOML_MEMBER_ATTR(pure) size_t table::size()					const noexcept	{ return map.size(); }
	TOML_EXTERNAL_LINKAGE void table::clear()					noexcept		{ map.clear(); }

	#undef TOML_MEMBER_ATTR

	TOML_EXTERNAL_LINKAGE
	bool table::is_homogeneous(node_type ntype) const noexcept
	{
		if (map.empty())
			return false;

		if (ntype == node_type::none)
			ntype = map.cbegin()->second->type();

		for (const auto& [k, v] : map)
		{
			(void)k;
			if (v->type() != ntype)
				return false;
		}

		return true;
	}

	namespace impl
	{
		template <typename T, typename U>
		TOML_INTERNAL_LINKAGE
		bool table_is_homogeneous(T& map, node_type ntype, U& first_nonmatch) noexcept
		{
			if (map.empty())
			{
				first_nonmatch = {};
				return false;
			}
			if (ntype == node_type::none)
				ntype = map.cbegin()->second->type();
			for (const auto& [k, v] : map)
			{
				(void)k;
				if (v->type() != ntype)
				{
					first_nonmatch = v.get();
					return false;
				}
			}
			return true;
		}
	}

	TOML_EXTERNAL_LINKAGE
	bool table::is_homogeneous(node_type ntype, toml::node*& first_nonmatch) noexcept
	{
		return impl::table_is_homogeneous(map, ntype, first_nonmatch);
	}

	TOML_EXTERNAL_LINKAGE
	bool table::is_homogeneous(node_type ntype, const toml::node*& first_nonmatch) const noexcept
	{
		return impl::table_is_homogeneous(map, ntype, first_nonmatch);
	}

	TOML_EXTERNAL_LINKAGE
	node_view<node> table::operator[] (std::string_view key) noexcept
	{
		return node_view<node>{ this->get(key) };
	}
	TOML_EXTERNAL_LINKAGE
	node_view<const node> table::operator[] (std::string_view key) const noexcept
	{
		return node_view<const node>{ this->get(key) };
	}

	TOML_EXTERNAL_LINKAGE
	table::iterator table::erase(iterator pos) noexcept
	{
		return { map.erase(pos.raw_) };
	}

	TOML_EXTERNAL_LINKAGE
	table::iterator table::erase(const_iterator pos) noexcept
	{
		return { map.erase(pos.raw_) };
	}

	TOML_EXTERNAL_LINKAGE
	table::iterator table::erase(const_iterator first, const_iterator last) noexcept
	{
		return { map.erase(first.raw_, last.raw_) };
	}

	TOML_EXTERNAL_LINKAGE
	bool table::erase(std::string_view key) noexcept
	{
		if (auto it = map.find(key); it != map.end())
		{
			map.erase(it);
			return true;
		}
		return false;
	}

	TOML_EXTERNAL_LINKAGE
	node* table::get(std::string_view key) noexcept
	{
		return do_get(map, key);
	}

	TOML_EXTERNAL_LINKAGE
	const node* table::get(std::string_view key) const noexcept
	{
		return do_get(map, key);
	}

	TOML_EXTERNAL_LINKAGE
	table::iterator table::find(std::string_view key) noexcept
	{
		return { map.find(key) };
	}

	TOML_EXTERNAL_LINKAGE
	table::const_iterator table::find(std::string_view key) const noexcept
	{
		return { map.find(key) };
	}

	TOML_EXTERNAL_LINKAGE
	bool table::contains(std::string_view key) const noexcept
	{
		return do_contains(map, key);
	}

	#if TOML_WINDOWS_COMPAT

	TOML_EXTERNAL_LINKAGE
	node_view<node> table::operator[] (std::wstring_view key) noexcept
	{
		return node_view<node>{ this->get(key) };
	}
	TOML_EXTERNAL_LINKAGE
	node_view<const node> table::operator[] (std::wstring_view key) const noexcept
	{
		return node_view<const node>{ this->get(key) };
	}

	TOML_EXTERNAL_LINKAGE
	bool table::erase(std::wstring_view key) noexcept
	{
		return erase(impl::narrow(key));
	}

	TOML_EXTERNAL_LINKAGE
	node* table::get(std::wstring_view key) noexcept
	{
		return get(impl::narrow(key));
	}

	TOML_EXTERNAL_LINKAGE
	const node* table::get(std::wstring_view key) const noexcept
	{
		return get(impl::narrow(key));
	}

	TOML_EXTERNAL_LINKAGE
	table::iterator table::find(std::wstring_view key) noexcept
	{
		return find(impl::narrow(key));
	}

	TOML_EXTERNAL_LINKAGE
	table::const_iterator table::find(std::wstring_view key) const noexcept
	{
		return find(impl::narrow(key));
	}

	TOML_EXTERNAL_LINKAGE
	bool table::contains(std::wstring_view key) const noexcept
	{
		return contains(impl::narrow(key));
	}

	#endif // TOML_WINDOWS_COMPAT

	TOML_API
	TOML_EXTERNAL_LINKAGE
	bool operator == (const table& lhs, const table& rhs) noexcept
	{
		if (&lhs == &rhs)
			return true;
		if (lhs.map.size() != rhs.map.size())
			return false;

		for (auto l = lhs.map.begin(), r = rhs.map.begin(), e = lhs.map.end(); l != e; l++, r++)
		{
			if (l->first != r->first)
				return false;

			const auto lhs_type = l->second->type();
			const node& rhs_ = *r->second;
			const auto rhs_type = rhs_.type();
			if (lhs_type != rhs_type)
				return false;

			const bool equal = l->second->visit([&](const auto& lhs_) noexcept
			{
				return lhs_ == *reinterpret_cast<std::remove_reference_t<decltype(lhs_)>*>(&rhs_);
			});
			if (!equal)
				return false;
		}
		return true;
	}

	TOML_API
	TOML_EXTERNAL_LINKAGE
	bool operator != (const table& lhs, const table& rhs) noexcept
	{
		return !(lhs == rhs);
	}
}
TOML_NAMESPACE_END

#endif //----------------------------------------------------------  ↑ toml_table.hpp  ---------------------------------

#if 1  //-----------------------------------------------------------------------------  ↓ toml_default_formatter.hpp  --

TOML_DISABLE_WARNINGS
#include <cmath>
TOML_ENABLE_WARNINGS

TOML_PUSH_WARNINGS
TOML_DISABLE_SWITCH_WARNINGS
TOML_DISABLE_ARITHMETIC_WARNINGS

TOML_IMPL_NAMESPACE_START
{
	inline constexpr size_t default_formatter_line_wrap = 120_sz;

	TOML_API
	TOML_EXTERNAL_LINKAGE
	std::string default_formatter_make_key_segment(const std::string& str) noexcept
	{
		if (str.empty())
			return "''"s;
		else
		{
			bool requiresQuotes = false;
			{
				utf8_decoder decoder;
				for (size_t i = 0; i < str.length() && !requiresQuotes; i++)
				{
					decoder(static_cast<uint8_t>(str[i]));
					if (decoder.error())
						requiresQuotes = true;
					else if (decoder.has_code_point())
						requiresQuotes = !is_bare_key_character(decoder.codepoint);
				}
			}

			if (requiresQuotes)
			{
				std::string s;
				s.reserve(str.length() + 2_sz);
				s += '"';
				for (auto c : str)
				{
					if TOML_UNLIKELY(c >= '\x00' && c <= '\x1F')
					{
						const auto& sv = low_character_escape_table[c];
						s.append(reinterpret_cast<const char*>(sv.data()), sv.length());
					}
					else if TOML_UNLIKELY(c == '\x7F')
						s.append("\\u007F"sv);
					else if TOML_UNLIKELY(c == '"')
						s.append("\\\""sv);
					else
						s += c;
				}
				s += '"';
				return s;
			}
			else
				return str;
		}
	}

	TOML_API
	TOML_EXTERNAL_LINKAGE
	size_t default_formatter_inline_columns(const node& node) noexcept
	{
		switch (node.type())
		{
			case node_type::table:
			{
				auto& n = *reinterpret_cast<const table*>(&node);
				if (n.empty())
					return 2_sz; // "{}"
				size_t weight = 3_sz; // "{ }"
				for (auto&& [k, v] : n)
				{
					weight += k.length() + default_formatter_inline_columns(v) + 2_sz; // +  ", "
					if (weight >= default_formatter_line_wrap)
						break;
				}
				return weight;
			}

			case node_type::array:
			{
				auto& n = *reinterpret_cast<const array*>(&node);
				if (n.empty())
					return 2_sz; // "[]"
				size_t weight = 3_sz; // "[ ]"
				for (auto& elem : n)
				{
					weight += default_formatter_inline_columns(elem) + 2_sz; // +  ", "
					if (weight >= default_formatter_line_wrap)
						break;
				}
				return weight;
			}

			case node_type::string:
			{
				auto& n = *reinterpret_cast<const value<std::string>*>(&node);
				return n.get().length() + 2_sz; // + ""
			}

			case node_type::integer:
			{
				auto& n = *reinterpret_cast<const value<int64_t>*>(&node);
				auto v = n.get();
				if (!v)
					return 1_sz;
				size_t weight = {};
				if (v < 0)
				{
					weight += 1;
					v *= -1;
				}
				return weight + static_cast<size_t>(log10(static_cast<double>(v))) + 1_sz;
			}

			case node_type::floating_point:
			{
				auto& n = *reinterpret_cast<const value<double>*>(&node);
				auto v = n.get();
				if (v == 0.0)
					return 3_sz;  // "0.0"
				size_t weight = 2_sz; // ".0"
				if (v < 0.0)
				{
					weight += 1;
					v *= -1.0;
				}
				return weight + static_cast<size_t>(log10(static_cast<double>(v))) + 1_sz;
				break;
			}

			case node_type::boolean: return 5_sz;
			case node_type::date: [[fallthrough]];
			case node_type::time: return 10_sz;
			case node_type::date_time: return 30_sz;
			case node_type::none: TOML_UNREACHABLE;
			TOML_NO_DEFAULT_CASE;
		}

		TOML_UNREACHABLE;
	}

	TOML_API
	TOML_EXTERNAL_LINKAGE
	bool default_formatter_forces_multiline(const node& node, size_t starting_column_bias) noexcept
	{
		return (default_formatter_inline_columns(node) + starting_column_bias) > default_formatter_line_wrap;
	}
}
TOML_IMPL_NAMESPACE_END

TOML_NAMESPACE_START
{
	template <typename Char>
	inline void default_formatter<Char>::print_inline(const toml::table& tbl)
	{
		if (tbl.empty())
			impl::print_to_stream("{}"sv, base::stream());
		else
		{
			impl::print_to_stream("{ "sv, base::stream());

			bool first = false;
			for (auto&& [k, v] : tbl)
			{
				if (first)
					impl::print_to_stream(", "sv, base::stream());
				first = true;

				print_key_segment(k);
				impl::print_to_stream(" = "sv, base::stream());

				const auto type = v.type();
				TOML_ASSUME(type != node_type::none);
				switch (type)
				{
					case node_type::table: print_inline(*reinterpret_cast<const table*>(&v)); break;
					case node_type::array: print(*reinterpret_cast<const array*>(&v)); break;
					default:
						base::print_value(v, type);
				}
			}

			impl::print_to_stream(" }"sv, base::stream());
		}
		base::clear_naked_newline();
	}
}
TOML_NAMESPACE_END

// implementations of windows wide string nonsense
#if TOML_WINDOWS_COMPAT

TOML_DISABLE_WARNINGS
#include <Windows.h> // fuckkkk :(
TOML_ENABLE_WARNINGS

TOML_IMPL_NAMESPACE_START
{
	TOML_API
	TOML_EXTERNAL_LINKAGE
	std::string narrow(std::wstring_view str) noexcept
	{
		if (str.empty())
			return {};

		std::string s;
		const auto len = WideCharToMultiByte(
			65001, 0, str.data(), static_cast<int>(str.length()), nullptr, 0, nullptr, nullptr
		);
		if (len)
		{
			s.resize(static_cast<size_t>(len));
			WideCharToMultiByte(65001, 0, str.data(), static_cast<int>(str.length()), s.data(), len, nullptr, nullptr);
		}
		return s;
	}

	TOML_API
	TOML_EXTERNAL_LINKAGE
	std::wstring widen(std::string_view str) noexcept
	{
		if (str.empty())
			return {};

		std::wstring s;
		const auto len = MultiByteToWideChar(65001, 0, str.data(), static_cast<int>(str.length()), nullptr, 0);
		if (len)
		{
			s.resize(static_cast<size_t>(len));
			MultiByteToWideChar(65001, 0, str.data(), static_cast<int>(str.length()), s.data(), len);
		}
		return s;
	}

	#ifdef __cpp_lib_char8_t

	TOML_API
	TOML_EXTERNAL_LINKAGE
	std::wstring widen(std::u8string_view str) noexcept
	{
		if (str.empty())
			return {};

		return widen(std::string_view{ reinterpret_cast<const char*>(str.data()), str.length() });
	}

	#endif // __cpp_lib_char8_t
}
TOML_IMPL_NAMESPACE_END

#endif // TOML_WINDOWS_COMPAT

TOML_POP_WARNINGS // TOML_DISABLE_SWITCH_WARNINGS, TOML_DISABLE_ARITHMETIC_WARNINGS

#endif //-----------------------------------------------------------------------------  ↑ toml_default_formatter.hpp  --

#if 1  //----  ↓ toml_json_formatter.hpp  ------------------------------------------------------------------------------

TOML_PUSH_WARNINGS
TOML_DISABLE_SWITCH_WARNINGS

TOML_NAMESPACE_START
{
	template <typename Char>
	inline void json_formatter<Char>::print(const toml::table& tbl)
	{
		if (tbl.empty())
			impl::print_to_stream("{}"sv, base::stream());
		else
		{
			impl::print_to_stream('{', base::stream());
			base::increase_indent();
			bool first = false;
			for (auto&& [k, v] : tbl)
			{
				if (first)
					impl::print_to_stream(", "sv, base::stream());
				first = true;
				base::print_newline(true);
				base::print_indent();

				base::print_quoted_string(k, false);
				impl::print_to_stream(" : "sv, base::stream());

				const auto type = v.type();
				TOML_ASSUME(type != node_type::none);
				switch (type)
				{
					case node_type::table: print(*reinterpret_cast<const table*>(&v)); break;
					case node_type::array: print(*reinterpret_cast<const array*>(&v)); break;
					default:
						base::print_value(v, type);
				}

			}
			base::decrease_indent();
			base::print_newline(true);
			base::print_indent();
			impl::print_to_stream('}', base::stream());
		}
		base::clear_naked_newline();
	}
}
TOML_NAMESPACE_END

TOML_POP_WARNINGS // TOML_DISABLE_SWITCH_WARNINGS

#endif //----  ↑ toml_json_formatter.hpp  ------------------------------------------------------------------------------

#if TOML_PARSER

#if 1  //------------------------------  ↓ toml_utf8_streams.hpp  ------------------------------------------------------

#if !TOML_EXCEPTIONS
	#undef TOML_ERROR_CHECK
	#define TOML_ERROR_CHECK	if (reader.error()) return nullptr
#endif

TOML_IMPL_NAMESPACE_START
{
	TOML_ABI_NAMESPACE_BOOL(TOML_EXCEPTIONS, ex, noex)

	TOML_EXTERNAL_LINKAGE
	utf8_buffered_reader::utf8_buffered_reader(utf8_reader_interface& reader_) noexcept
		: reader{ reader_ }
	{}

	TOML_EXTERNAL_LINKAGE
	const source_path_ptr& utf8_buffered_reader::source_path() const noexcept
	{
		return reader.source_path();
	}

	TOML_EXTERNAL_LINKAGE
	const utf8_codepoint* utf8_buffered_reader::read_next()
	{
		TOML_ERROR_CHECK;

		if (negative_offset)
		{
			negative_offset--;

			// an entry negative offset of 1 just means "replay the current head"
			if (!negative_offset)
				return head;

			// otherwise step back into the history buffer
			else
				return history.buffer + ((history.first + history.count - negative_offset) % history_buffer_size);
		}
		else
		{
			// first character read from stream
			if TOML_UNLIKELY(!history.count && !head)
				head = reader.read_next();

			// subsequent characters and not eof
			else if (head)
			{
				if TOML_UNLIKELY(history.count < history_buffer_size)
					history.buffer[history.count++] = *head;
				else
					history.buffer[(history.first++ + history_buffer_size) % history_buffer_size] = *head;

				head = reader.read_next();
			}

			return head;
		}
	}

	TOML_EXTERNAL_LINKAGE
	const utf8_codepoint* utf8_buffered_reader::step_back(size_t count) noexcept
	{
		TOML_ERROR_CHECK;
		TOML_ASSERT(history.count);
		TOML_ASSERT(negative_offset + count <= history.count);

		negative_offset += count;

		return negative_offset
			? history.buffer + ((history.first + history.count - negative_offset) % history_buffer_size)
			: head;
	}

	TOML_EXTERNAL_LINKAGE
	bool utf8_buffered_reader::peek_eof() const
	{
		return reader.peek_eof();
	}

	#if !TOML_EXCEPTIONS
	TOML_EXTERNAL_LINKAGE
	optional<parse_error>&& utf8_buffered_reader::error() noexcept
	{
		return reader.error();
	}
	#endif

	TOML_ABI_NAMESPACE_END // TOML_EXCEPTIONS
}
TOML_IMPL_NAMESPACE_END

#undef TOML_ERROR_CHECK
#undef TOML_ERROR

#endif //------------------------------  ↑ toml_utf8_streams.hpp  ------------------------------------------------------

#if 1  //----------------------------------------------------------  ↓ toml_parser.hpp  --------------------------------

TOML_DISABLE_WARNINGS
#include <cmath>
#if TOML_INT_CHARCONV || TOML_FLOAT_CHARCONV
	#include <charconv>
#endif
#if !TOML_INT_CHARCONV || !TOML_FLOAT_CHARCONV
	#include <sstream>
#endif
#if !TOML_HEADER_ONLY
	using namespace std::string_view_literals;
#endif
TOML_ENABLE_WARNINGS

TOML_PUSH_WARNINGS
TOML_DISABLE_SWITCH_WARNINGS

#if TOML_EXCEPTIONS && !defined(__INTELLISENSE__)
	#define TOML_RETURNS_BY_THROWING		[[noreturn]]
#else
	#define TOML_RETURNS_BY_THROWING
#endif

TOML_ANON_NAMESPACE_START
{
	template <uint64_t> struct parse_integer_traits;
	template <> struct parse_integer_traits<2> final
	{
		static constexpr auto scope_qualifier = "binary integer"sv;
		static constexpr auto is_digit = ::toml::impl::is_binary_digit;
		static constexpr auto is_signed = false;
		static constexpr auto buffer_length = 63;
		static constexpr auto prefix_codepoint = U'b';
		static constexpr auto prefix = "b"sv;
	};
	template <> struct parse_integer_traits<8> final
	{
		static constexpr auto scope_qualifier = "octal integer"sv;
		static constexpr auto is_digit = ::toml::impl::is_octal_digit;
		static constexpr auto is_signed = false;
		static constexpr auto buffer_length = 21; // strlen("777777777777777777777")
		static constexpr auto prefix_codepoint = U'o';
		static constexpr auto prefix = "o"sv;
	};
	template <> struct parse_integer_traits<10> final
	{
		static constexpr auto scope_qualifier = "decimal integer"sv;
		static constexpr auto is_digit = ::toml::impl::is_decimal_digit;
		static constexpr auto is_signed = true;
		static constexpr auto buffer_length = 19; //strlen("9223372036854775807")
	};
	template <> struct parse_integer_traits<16> final
	{
		static constexpr auto scope_qualifier = "hexadecimal integer"sv;
		static constexpr auto is_digit = ::toml::impl::is_hexadecimal_digit;
		static constexpr auto is_signed = false;
		static constexpr auto buffer_length = 16; //strlen("7FFFFFFFFFFFFFFF")
		static constexpr auto prefix_codepoint = U'x';
		static constexpr auto prefix = "x"sv;
	};

	[[nodiscard]]
	TOML_INTERNAL_LINKAGE
	std::string_view to_sv(::toml::node_type val) noexcept
	{
		using namespace ::toml::impl;

		return node_type_friendly_names[unwrap_enum(val)];
	}

	[[nodiscard]]
	TOML_INTERNAL_LINKAGE
	std::string_view to_sv(const std::string& str) noexcept
	{
		return std::string_view{ str };
	}

	[[nodiscard]]
	TOML_ATTR(const)
	TOML_INTERNAL_LINKAGE
	std::string_view to_sv(bool val) noexcept
	{
		using namespace std::string_view_literals;

		return val ? "true"sv : "false"sv;
	}

	[[nodiscard]]
	TOML_INTERNAL_LINKAGE
	std::string_view to_sv(const ::toml::impl::utf8_codepoint& cp) noexcept
	{
		using namespace ::toml;
		using namespace ::toml::impl;

		if TOML_UNLIKELY(cp.value <= U'\x1F')
			return low_character_escape_table[cp.value];
		else if TOML_UNLIKELY(cp.value == U'\x7F')
			return "\\u007F"sv;
		else
			return cp.as_view();
	}

	template <typename T>
	TOML_ATTR(nonnull)
	TOML_INTERNAL_LINKAGE
	TOML_NEVER_INLINE
	void concatenate(char*& write_pos, char *const buf_end, const T& arg) noexcept
	{
		using namespace ::toml;
		using namespace ::toml::impl;

		static_assert(
			is_one_of<remove_cvref_t<T>, std::string_view, int64_t, uint64_t, double>,
			"concatenate inputs are limited to std::string_view, int64_t, uint64_t and double to keep "
			"instantiations to a minimum as an anti-bloat measure (hint: to_sv will probably help)"
		);

		if (write_pos >= buf_end)
			return;

		using arg_t = remove_cvref_t<T>;
		if constexpr (std::is_same_v<arg_t, std::string_view>)
		{
			const auto max_chars = static_cast<size_t>(buf_end - write_pos);
			const auto len = max_chars < arg.length() ? max_chars : arg.length();
			std::memcpy(write_pos, arg.data(), len);
			write_pos += len;
		}
		else if constexpr (std::is_floating_point_v<arg_t>)
		{
			#if TOML_FLOAT_CHARCONV
			{
				const auto result = std::to_chars(write_pos, buf_end, arg);
				write_pos = result.ptr;
			}
			#else
			{
				std::ostringstream ss;
				ss.imbue(std::locale::classic());
				ss.precision(std::numeric_limits<arg_t>::digits10 + 1);
				ss << arg;
				concatenate(write_pos, buf_end, to_sv(std::move(ss).str()));
			}
			#endif
		}
		else if constexpr (std::is_integral_v<arg_t>)
		{
			#if TOML_INT_CHARCONV
			{
				const auto result = std::to_chars(write_pos, buf_end, arg);
				write_pos = result.ptr;
			}
			#else
			{
				std::ostringstream ss;
				ss.imbue(std::locale::classic());
				using cast_type = std::conditional_t<std::is_signed_v<arg_t>, int64_t, uint64_t>;
				ss << static_cast<cast_type>(arg);
				concatenate(write_pos, buf_end, to_sv(std::move(ss).str()));
			}
			#endif
		}
	}

	struct error_builder final
	{
		static constexpr std::size_t buf_size = 512;
		char buf[buf_size];
		char* write_pos = buf;
		char* const max_write_pos = buf + (buf_size - std::size_t{ 1 }); //allow for null terminator

		error_builder(std::string_view scope) noexcept
		{
			concatenate(write_pos, max_write_pos, "Error while parsing "sv);
			concatenate(write_pos, max_write_pos, scope);
			concatenate(write_pos, max_write_pos, ": "sv);
		}

		template <typename T>
		void append(const T& arg) noexcept
		{
			concatenate(write_pos, max_write_pos, arg);
		}

		TOML_RETURNS_BY_THROWING
		auto finish(const ::toml::source_position& pos, const ::toml::source_path_ptr& source_path) const TOML_MAY_THROW
		{
			using namespace ::toml;

			*write_pos = '\0';

			#if TOML_EXCEPTIONS
				throw parse_error{ buf, pos, source_path };
			#else
				return parse_error{
					std::string(buf, static_cast<size_t>(write_pos - buf)),
					pos,
					source_path
				};
			#endif
		}
	};

	struct parse_scope final
	{
		std::string_view& storage_;
		std::string_view parent_;

		TOML_NODISCARD_CTOR
		explicit parse_scope(std::string_view& current_scope, std::string_view new_scope) noexcept
			: storage_{ current_scope },
			parent_{ current_scope }
		{
			storage_ = new_scope;
		}

		~parse_scope() noexcept
		{
			storage_ = parent_;
		}
	};
	#define push_parse_scope_2(scope, line)		parse_scope ps_##line{ current_scope, scope }
	#define push_parse_scope_1(scope, line)		push_parse_scope_2(scope, line)
	#define push_parse_scope(scope)				push_parse_scope_1(scope, __LINE__)

	struct parsed_key final
	{
		std::vector<std::string> segments;
	};

	struct parsed_key_value_pair final
	{
		parsed_key key;
		toml::node* value;
	};

}
TOML_ANON_NAMESPACE_END

TOML_IMPL_NAMESPACE_START
{
	// Q: "what the fuck is this? MACROS????"
	// A: The parser needs to work in exceptionless mode (returning error objects directly)
	//    and exception mode (reporting parse failures by throwing). Two totally different control flows.
	//    These macros encapsulate the differences between the two modes so I can write code code
	//    as though I was only targeting one mode and not want yeet myself into the sun.
	//    They're all #undef'd at the bottom of the parser's implementation so they should be harmless outside
	//    of toml++.

	#if defined(NDEBUG) || !defined(_DEBUG)
		#define assert_or_assume(cond)			TOML_ASSUME(cond)
	#else
		#define assert_or_assume(cond)			TOML_ASSERT(cond)
	#endif

	#define is_eof()							!cp
	#define assert_not_eof()					assert_or_assume(cp != nullptr)
	#define return_if_eof(...)					do { if (is_eof()) return __VA_ARGS__; } while(false)
	#if TOML_EXCEPTIONS
		#define is_error()						false
		#define return_after_error(...)			TOML_UNREACHABLE
		#define assert_not_error()				(void)0
		#define return_if_error(...)			(void)0
		#define return_if_error_or_eof(...)		return_if_eof(__VA_ARGS__)
	#else
		#define is_error()						!!err
		#define return_after_error(...)			return __VA_ARGS__
		#define assert_not_error()				TOML_ASSERT(!is_error())
		#define return_if_error(...)			do { if (is_error()) return __VA_ARGS__; } while(false)
		#define return_if_error_or_eof(...)		do { if (is_eof() || is_error()) return __VA_ARGS__; } while(false)
	#endif
	#define set_error_and_return(ret, ...)		\
		do { if (!is_error()) set_error(__VA_ARGS__); return_after_error(ret); } while(false)
	#define set_error_and_return_default(...)	set_error_and_return({}, __VA_ARGS__)
	#define set_error_and_return_if_eof(...)	\
		do { if (is_eof()) set_error_and_return(__VA_ARGS__, "encountered end-of-file"sv); } while(false)
	#define advance_and_return_if_error(...)	\
		do { assert_not_eof(); advance(); return_if_error(__VA_ARGS__); } while (false)
	#define advance_and_return_if_error_or_eof(...)		\
		do {											\
			assert_not_eof();							\
			advance();									\
			return_if_error(__VA_ARGS__);				\
			set_error_and_return_if_eof(__VA_ARGS__);	\
		} while (false)

	TOML_ABI_NAMESPACE_BOOL(TOML_EXCEPTIONS, ex, noex)

	class parser final
	{
		private:
			utf8_buffered_reader reader;
			table root;
			source_position prev_pos = { 1, 1 };
			const utf8_codepoint* cp = {};
			std::vector<table*> implicit_tables;
			std::vector<table*> dotted_key_tables;
			std::vector<array*> table_arrays;
			std::string recording_buffer; //for diagnostics
			bool recording = false, recording_whitespace = true;
			std::string_view current_scope;
			#if !TOML_EXCEPTIONS
			mutable optional<toml::parse_error> err;
			#endif

			[[nodiscard]]
			source_position current_position(source_index fallback_offset = 0) const noexcept
			{
				if (!is_eof())
					return cp->position;
				return { prev_pos.line, static_cast<source_index>(prev_pos.column + fallback_offset) };
			}

			template <typename... T>
			TOML_RETURNS_BY_THROWING
			TOML_NEVER_INLINE
			void set_error_at(source_position pos, const T&... reason) const TOML_MAY_THROW
			{
				static_assert(sizeof...(T) > 0_sz);
				#if !TOML_EXCEPTIONS
				if (err)
					return;
				#endif

				error_builder builder{ current_scope };
				(builder.append(reason), ...);

				#if TOML_EXCEPTIONS
					builder.finish(pos, reader.source_path());
				#else
					err.emplace(builder.finish(pos, reader.source_path()));
				#endif
			}

			template <typename... T>
			TOML_RETURNS_BY_THROWING
			void set_error(const T&... reason) const TOML_MAY_THROW
			{
				set_error_at(current_position(1), reason...);
			}

			void go_back(size_t count = 1_sz) noexcept
			{
				return_if_error();
				assert_or_assume(count);

				cp = reader.step_back(count);
				prev_pos = cp->position;
			}

			void advance() TOML_MAY_THROW
			{
				return_if_error();
				assert_not_eof();

				prev_pos = cp->position;
				cp = reader.read_next();

				#if !TOML_EXCEPTIONS
				if (reader.error())
				{
					err = std::move(reader.error());
					return;
				}
				#endif

				if (recording && !is_eof())
				{
					if (recording_whitespace || !(is_whitespace(*cp) || is_line_break(*cp)))
						recording_buffer.append(cp->as_view());
				}
			}

			void start_recording(bool include_current = true) noexcept
			{
				return_if_error();

				recording = true;
				recording_whitespace = true;
				recording_buffer.clear();
				if (include_current && !is_eof())
					recording_buffer.append(cp->as_view());
			}

			void stop_recording(size_t pop_bytes = 0_sz) noexcept
			{
				return_if_error();

				recording = false;
				if (pop_bytes)
				{
					if (pop_bytes >= recording_buffer.length())
						recording_buffer.clear();
					else if (pop_bytes == 1_sz)
						recording_buffer.pop_back();
					else
						recording_buffer.erase(
							recording_buffer.begin() + static_cast<ptrdiff_t>(recording_buffer.length() - pop_bytes),
							recording_buffer.end()
						);
				}
			}

			bool consume_leading_whitespace() TOML_MAY_THROW
			{
				return_if_error_or_eof({});

				bool consumed = false;
				while (!is_eof() && is_whitespace(*cp))
				{
					consumed = true;
					advance_and_return_if_error({});
				}
				return consumed;
			}

			bool consume_line_break() TOML_MAY_THROW
			{
				return_if_error_or_eof({});

				if (!is_line_break(*cp))
					return false;

				if (*cp == U'\r')
				{
					advance_and_return_if_error({}); // skip \r

					if (is_eof())
						return true; //eof after \r is 'fine'
					else if (*cp != U'\n')
						set_error_and_return_default("expected \\n, saw '"sv, to_sv(*cp), "'"sv);
				}
				advance_and_return_if_error({}); // skip \n (or other single-character line ending)
				return true;
			}

			bool consume_rest_of_line() TOML_MAY_THROW
			{
				return_if_error_or_eof({});

				do
				{
					if (is_line_break(*cp))
						return consume_line_break();
					else
						advance();
					return_if_error({});
				}
				while (!is_eof());

				return true;
			}

			bool consume_comment() TOML_MAY_THROW
			{
				return_if_error_or_eof({});

				if (*cp != U'#')
					return false;

				push_parse_scope("comment"sv);

				advance_and_return_if_error({}); //skip the '#'

				while (!is_eof())
				{
					if (consume_line_break())
						return true;

					if constexpr (TOML_LANG_AT_LEAST(1, 0, 0))
					{
						// toml/issues/567 (disallow non-TAB control characters in comments)
						if (is_nontab_control_character(*cp))
							set_error_and_return_default(
								"control characters other than TAB (U+0009) are explicitly prohibited"sv
							);

						// toml/pull/720 (disallow surrogates in comments)
						else if (is_unicode_surrogate(*cp))
							set_error_and_return_default(
								"unicode surrogates (U+D800 to U+DFFF) are explicitly prohibited"sv
							);
					}
					advance_and_return_if_error({});
				}

				return true;
			}

			[[nodiscard]]
			bool consume_expected_sequence(std::u32string_view seq) TOML_MAY_THROW
			{
				return_if_error({});
				TOML_ASSERT(!seq.empty());

				for (auto c : seq)
				{
					set_error_and_return_if_eof({});
					if (*cp != c)
						return false;
					advance_and_return_if_error({});
				}
				return true;
			}

			template <typename T>
			[[nodiscard]]
			bool consume_digit_sequence(T* digits, size_t len) TOML_MAY_THROW
			{
				return_if_error({});
				assert_or_assume(digits);
				assert_or_assume(len);

				for (size_t i = 0; i < len; i++)
				{
					set_error_and_return_if_eof({});
					if (!is_decimal_digit(*cp))
						return false;

					digits[i] = static_cast<T>(*cp - U'0');
					advance_and_return_if_error({});
				}
				return true;
			}

			template <typename T>
			[[nodiscard]]
			size_t consume_variable_length_digit_sequence(T* buffer, size_t max_len) TOML_MAY_THROW
			{
				return_if_error({});
				assert_or_assume(buffer);
				assert_or_assume(max_len);

				size_t i = {};
				for (; i < max_len; i++)
				{
					if (is_eof() || !is_decimal_digit(*cp))
						break;

					buffer[i] = static_cast<T>(*cp - U'0');
					advance_and_return_if_error({});
				}
				return i;
			}

			//template <bool MultiLine>
			[[nodiscard]]
			std::string parse_basic_string(bool multi_line) TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(*cp == U'"');
				push_parse_scope("string"sv);

				// skip the '"'
				advance_and_return_if_error_or_eof({});

				// multiline strings ignore a single line ending right at the beginning
				if (multi_line)
				{
					consume_line_break();
					return_if_error({});
					set_error_and_return_if_eof({});
				}

				std::string str;
				bool escaped = false;
				[[maybe_unused]] bool skipping_whitespace = false;
				do
				{
					if (escaped)
					{
						escaped = false;

						// handle 'line ending slashes' in multi-line mode
						if (multi_line)
						{
							if (is_line_break(*cp) || is_whitespace(*cp))
							{
								consume_leading_whitespace();
								if (!consume_line_break())
									set_error_and_return_default(
										"line-ending backslashes must be the last non-whitespace character on the line"sv
									);
								skipping_whitespace = true;
								return_if_error({});
								continue;
							}
						}

						bool skipped_escaped_codepoint = false;
						assert_not_eof();
						switch (const auto escaped_codepoint = *cp)
						{
							// 'regular' escape codes
							case U'b': str += '\b'; break;
							case U'f': str += '\f'; break;
							case U'n': str += '\n'; break;
							case U'r': str += '\r'; break;
							case U't': str += '\t'; break;
							case U'"': str += '"'; break;
							case U'\\': str += '\\'; break;

							// unicode scalar sequences
							case U'x':
								#if TOML_LANG_UNRELEASED // toml/pull/709 (\xHH unicode scalar sequences)
									[[fallthrough]];
								#else
									set_error_and_return_default(
										"escape sequence '\\x' is not supported in TOML 1.0.0 and earlier"sv
									);
								#endif
							case U'u': [[fallthrough]];
							case U'U':
							{
								push_parse_scope("unicode scalar escape sequence"sv);
								advance_and_return_if_error_or_eof({});
								skipped_escaped_codepoint = true;

								uint32_t place_value = escaped_codepoint == U'U'
									? 0x10000000u
									: (escaped_codepoint == U'u' ? 0x1000u : 0x10u);
								uint32_t sequence_value{};
								while (place_value)
								{
									set_error_and_return_if_eof({});
									if (!is_hexadecimal_digit(*cp))
										set_error_and_return_default("expected hex digit, saw '"sv, to_sv(*cp), "'"sv);
									sequence_value += place_value * hex_to_dec(*cp);
									place_value /= 16u;
									advance_and_return_if_error({});
								}

								if (is_unicode_surrogate(sequence_value))
									set_error_and_return_default(
										"unicode surrogates (U+D800 - U+DFFF) are explicitly prohibited"sv
									);
								else if (sequence_value > 0x10FFFFu)
									set_error_and_return_default("values greater than U+10FFFF are invalid"sv);
								else if (sequence_value <= 0x7Fu) //ascii
									str += static_cast<char>(sequence_value & 0x7Fu);
								else if (sequence_value <= 0x7FFu)
								{
									str += static_cast<char>(0xC0u | ((sequence_value >> 6) & 0x1Fu));
									str += static_cast<char>(0x80u | (sequence_value & 0x3Fu));
								}
								else if (sequence_value <= 0xFFFFu)
								{
									str += static_cast<char>(0xE0u | ((sequence_value >> 12) & 0x0Fu));
									str += static_cast<char>(0x80u | ((sequence_value >> 6) & 0x1Fu));
									str += static_cast<char>(0x80u | (sequence_value & 0x3Fu));
								}
								else
								{
									str += static_cast<char>(0xF0u | ((sequence_value >> 18) & 0x07u));
									str += static_cast<char>(0x80u | ((sequence_value >> 12) & 0x3Fu));
									str += static_cast<char>(0x80u | ((sequence_value >> 6) & 0x3Fu));
									str += static_cast<char>(0x80u | (sequence_value & 0x3Fu));
								}
								break;
							}

							// ???
							default:
								set_error_and_return_default("unknown escape sequence '\\"sv, to_sv(*cp), "'"sv);
						}

						// skip the escaped character
						if (!skipped_escaped_codepoint)
							advance_and_return_if_error_or_eof({});
					}
					else
					{
						// handle closing delimiters
						if (*cp == U'"')
						{
							if (multi_line)
							{
								size_t lookaheads = {};
								size_t consecutive_delimiters = 1_sz;
								do
								{
									advance_and_return_if_error({});
									lookaheads++;
									if (!is_eof() && *cp == U'"')
										consecutive_delimiters++;
									else
										break;
								}
								while (lookaheads < 4_sz);

								switch (consecutive_delimiters)
								{
									// """ " (one quote somewhere in a ML string)
									case 1_sz:
										str += '"';
										skipping_whitespace = false;
										continue;

									// """ "" (two quotes somewhere in a ML string)
									case 2_sz:
										str.append("\"\""sv);
										skipping_whitespace = false;
										continue;

									// """ """ (the end of the string)
									case 3_sz:
										return str;

									// """ """" (one at the end of the string)
									case 4_sz:
										str += '"';
										return str;

									// """ """"" (two quotes at the end of the string)
									case 5_sz:
										str.append("\"\""sv);
										advance_and_return_if_error({}); // skip the last '"'
										return str;

									TOML_NO_DEFAULT_CASE;
								}
							}
							else
							{
								advance_and_return_if_error({}); // skip the closing delimiter
								return str;
							}
						}

						// handle escapes
						else if (*cp == U'\\')
						{
							advance_and_return_if_error_or_eof({}); // skip the '\'
							skipping_whitespace = false;
							escaped = true;
							continue;
						}

						// handle line endings in multi-line mode
						if (multi_line && is_line_break(*cp))
						{
							consume_line_break();
							return_if_error({});
							if (!skipping_whitespace)
								str += '\n';
							continue;
						}

						// handle control characters
						if (is_nontab_control_character(*cp))
							set_error_and_return_default(
								"unescaped control characters other than TAB (U+0009) are explicitly prohibited"sv
							);

						// handle surrogates in strings (1.0.0 and later)
						if constexpr (TOML_LANG_AT_LEAST(1, 0, 0))
						{
							if (is_unicode_surrogate(*cp))
								set_error_and_return_default(
									"unescaped unicode surrogates (U+D800 to U+DFFF) are explicitly prohibited"sv
								);
						}

						if (multi_line)
						{
							if (!skipping_whitespace || !is_whitespace(*cp))
							{
								skipping_whitespace = false;
								str.append(cp->as_view());
							}
						}
						else
							str.append(cp->as_view());

						advance_and_return_if_error({});
					}
				}
				while (!is_eof());

				set_error_and_return_default("encountered end-of-file"sv);
			}

			[[nodiscard]]
			std::string parse_literal_string(bool multi_line) TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(*cp == U'\'');
				push_parse_scope("literal string"sv);

				// skip the delimiter
				advance_and_return_if_error_or_eof({});

				// multiline strings ignore a single line ending right at the beginning
				if (multi_line)
				{
					consume_line_break();
					return_if_error({});
					set_error_and_return_if_eof({});
				}

				std::string str;
				do
				{
					assert_not_error();

					// handle closing delimiters
					if (*cp == U'\'')
					{
						if (multi_line)
						{
							size_t lookaheads = {};
							size_t consecutive_delimiters = 1_sz;
							do
							{
								advance_and_return_if_error({});
								lookaheads++;
								if (!is_eof() && *cp == U'\'')
									consecutive_delimiters++;
								else
									break;
							}
							while (lookaheads < 4_sz);

							switch (consecutive_delimiters)
							{
								// ''' ' (one quote somewhere in a ML string)
								case 1_sz:
									str += '\'';
									continue;

								// ''' '' (two quotes somewhere in a ML string)
								case 2_sz:
									str.append("''"sv);
									continue;

								// ''' ''' (the end of the string)
								case 3_sz:
									return str;

								// ''' '''' (one at the end of the string)
								case 4_sz:
									str += '\'';
									return str;

								// ''' ''''' (two quotes at the end of the string)
								case 5_sz:
									str.append("''"sv);
									advance_and_return_if_error({}); // skip the last '
									return str;

								TOML_NO_DEFAULT_CASE;
							}
						}
						else
						{
							advance_and_return_if_error({}); // skip the closing delimiter
							return str;
						}
					}

					// handle line endings in multi-line mode
					if (multi_line && is_line_break(*cp))
					{
						consume_line_break();
						str += '\n';
						continue;
					}

					// handle control characters
					if (is_nontab_control_character(*cp))
						set_error_and_return_default(
							"control characters other than TAB (U+0009) are explicitly prohibited"sv
						);

					// handle surrogates in strings (1.0.0 and later)
					if constexpr (TOML_LANG_AT_LEAST(1, 0, 0))
					{
						if (is_unicode_surrogate(*cp))
							set_error_and_return_default(
								"unicode surrogates (U+D800 - U+DFFF) are explicitly prohibited"sv
							);
					}

					str.append(cp->as_view());
					advance_and_return_if_error({});
				}
				while (!is_eof());

				set_error_and_return_default("encountered end-of-file"sv);
			}

			struct parsed_string final
			{
				std::string value;
				bool was_multi_line;
			};

			[[nodiscard]]
			TOML_NEVER_INLINE
			parsed_string parse_string() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_string_delimiter(*cp));
				push_parse_scope("string"sv);

				// get the first three characters to determine the string type
				const auto first = cp->value;
				advance_and_return_if_error_or_eof({});
				const auto second = cp->value;
				advance_and_return_if_error({});
				const auto third = cp ? cp->value : U'\0';

				// if we were eof at the third character then first and second need to be
				// the same string character (otherwise it's an unterminated string)
				if (is_eof())
				{
					if (second == first)
						return {};

					set_error_and_return_default("encountered end-of-file"sv);
				}

				// if the first three characters are all the same string delimiter then
				// it's a multi-line string.
				else if (first == second && first == third)
				{
					return
					{
						first == U'\''
							? parse_literal_string(true)
							: parse_basic_string(true),
						true
					};
				}

				// otherwise it's just a regular string.
				else
				{
					// step back two characters so that the current
					// character is the string delimiter
					go_back(2_sz);

					return
					{
						first == U'\''
							? parse_literal_string(false)
							: parse_basic_string(false),
						false
					};
				}
			}

			[[nodiscard]]
			std::string parse_bare_key_segment() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_bare_key_character(*cp));

				std::string segment;

				while (!is_eof())
				{
					if (!is_bare_key_character(*cp))
						break;

					segment.append(cp->as_view());
					advance_and_return_if_error({});
				}

				return segment;
			}

			[[nodiscard]]
			bool parse_boolean() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_match(*cp, U't', U'f', U'T', U'F'));
				push_parse_scope("boolean"sv);

				start_recording(true);
				auto result = is_match(*cp, U't', U'T');
				if (!consume_expected_sequence(result ? U"true"sv : U"false"sv))
					set_error_and_return_default(
						"expected '"sv, to_sv(result), "', saw '"sv, to_sv(recording_buffer), "'"sv
					);
				stop_recording();

				if (cp && !is_value_terminator(*cp))
					set_error_and_return_default("expected value-terminator, saw '"sv, to_sv(*cp), "'"sv);

				return result;
			}

			[[nodiscard]]
			double parse_inf_or_nan() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_match(*cp, U'i', U'n', U'I', U'N', U'+', U'-'));
				push_parse_scope("floating-point"sv);

				start_recording(true);
				const bool negative = *cp == U'-';
				if (negative || *cp == U'+')
					advance_and_return_if_error_or_eof({});

				const bool inf = is_match(*cp, U'i', U'I');
				if (!consume_expected_sequence(inf ? U"inf"sv : U"nan"sv))
					set_error_and_return_default(
						"expected '"sv, inf ? "inf"sv : "nan"sv, "', saw '"sv, to_sv(recording_buffer), "'"sv
					);
				stop_recording();

				if (cp && !is_value_terminator(*cp))
					set_error_and_return_default("expected value-terminator, saw '"sv, to_sv(*cp), "'"sv);

				return inf ? (negative ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity())
					: std::numeric_limits<double>::quiet_NaN();
			}

			TOML_PUSH_WARNINGS
			TOML_DISABLE_SWITCH_WARNINGS
			TOML_DISABLE_INIT_WARNINGS

			[[nodiscard]]
			double parse_float() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_match(*cp, U'+', U'-', U'.') || is_decimal_digit(*cp));
				push_parse_scope("floating-point"sv);

				// sign
				const int sign = *cp == U'-' ? -1 : 1;
				if (is_match(*cp, U'+', U'-'))
					advance_and_return_if_error_or_eof({});

				// consume value chars
				char chars[64];
				size_t length = {};
				const utf8_codepoint* prev = {};
				bool seen_decimal = false, seen_exponent = false;
				char first_integer_part = '\0';
				while (!is_eof() && !is_value_terminator(*cp))
				{
					if (*cp == U'_')
					{
						if (!prev || !is_decimal_digit(*prev))
							set_error_and_return_default("underscores may only follow digits"sv);

						prev = cp;
						advance_and_return_if_error_or_eof({});
						continue;
					}
					else if (prev && *prev == U'_' && !is_decimal_digit(*cp))
						set_error_and_return_default("underscores must be followed by digits"sv);
					else if (*cp == U'.')
					{
						// .1
						// -.1
						// +.1 (no integer part)
						if (!first_integer_part)
							set_error_and_return_default("expected decimal digit, saw '.'"sv);

						// 1.0e+.10 (exponent cannot have '.')
						else if (seen_exponent)
							set_error_and_return_default("expected exponent decimal digit or sign, saw '.'"sv);

						// 1.0.e+.10
						// 1..0
						// (multiple '.')
						else if (seen_decimal)
							set_error_and_return_default("expected decimal digit or exponent, saw '.'"sv);

						seen_decimal = true;
					}
					else if (is_match(*cp, U'e', U'E'))
					{
						if (prev && !is_decimal_digit(*prev))
							set_error_and_return_default("expected decimal digit, saw '"sv, to_sv(*cp), "'"sv);

						// 1.0ee+10 (multiple 'e')
						else if (seen_exponent)
							set_error_and_return_default("expected decimal digit, saw '"sv, to_sv(*cp), "'"sv);

						seen_decimal = true; // implied
						seen_exponent = true;
					}
					else if (is_match(*cp, U'+', U'-'))
					{
						// 1.-0 (sign in mantissa)
						if (!seen_exponent)
							set_error_and_return_default("expected decimal digit or '.', saw '"sv, to_sv(*cp), "'"sv);

						// 1.0e1-0 (misplaced exponent sign)
						else if (!is_match(*prev, U'e', U'E'))
							set_error_and_return_default("expected exponent digit, saw '"sv, to_sv(*cp), "'"sv);
					}
					else if (length == sizeof(chars))
						set_error_and_return_default(
							"exceeds maximum length of "sv, static_cast<uint64_t>(sizeof(chars)), " characters"sv
						);
					else if (is_decimal_digit(*cp))
					{
						if (!seen_decimal)
						{
							if (!first_integer_part)
								first_integer_part = static_cast<char>(cp->bytes[0]);
							else if (first_integer_part == '0')
								set_error_and_return_default("leading zeroes are prohibited"sv);
						}
					}
					else
						set_error_and_return_default("expected decimal digit, saw '"sv, to_sv(*cp), "'"sv);

					chars[length++] = static_cast<char>(cp->bytes[0]);
					prev = cp;
					advance_and_return_if_error({});
				}

				// sanity-check ending state
				if (prev)
				{
					if (*prev == U'_')
					{
						set_error_and_return_if_eof({});
						set_error_and_return_default("underscores must be followed by digits"sv);
					}
					else if (is_match(*prev, U'e', U'E', U'+', U'-', U'.'))
					{
						set_error_and_return_if_eof({});
						set_error_and_return_default("expected decimal digit, saw '"sv, to_sv(*cp), "'"sv);
					}
				}

				// convert to double
				double result;
				#if TOML_FLOAT_CHARCONV
				{
					auto fc_result = std::from_chars(chars, chars + length, result);
					switch (fc_result.ec)
					{
						case std::errc{}: //ok
							return result * sign;

						case std::errc::invalid_argument:
							set_error_and_return_default(
								"'"sv, std::string_view{ chars, length }, "' could not be interpreted as a value"sv
							);
							break;

						case std::errc::result_out_of_range:
							set_error_and_return_default(
								"'"sv, std::string_view{ chars, length }, "' is not representable in 64 bits"sv
							);
							break;

						default: //??
							set_error_and_return_default(
								"an unspecified error occurred while trying to interpret '"sv,
								std::string_view{ chars, length }, "' as a value"sv
							);
					}
				}
				#else
				{
					std::stringstream ss;
					ss.imbue(std::locale::classic());
					ss.write(chars, static_cast<std::streamsize>(length));
					if ((ss >> result))
						return result * sign;
					else
						set_error_and_return_default(
							"'"sv, std::string_view{ chars, length }, "' could not be interpreted as a value"sv
						);
				}
				#endif
			}

			[[nodiscard]]
			double parse_hex_float() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_match(*cp, U'0', U'+', U'-'));
				push_parse_scope("hexadecimal floating-point"sv);

				#if TOML_LANG_UNRELEASED // toml/issues/562 (hexfloats)

				// sign
				const int sign = *cp == U'-' ? -1 : 1;
				if (is_match(*cp, U'+', U'-'))
					advance_and_return_if_error_or_eof({});

				// '0'
				if (*cp != U'0')
					set_error_and_return_default(" expected '0', saw '"sv, to_sv(*cp), "'"sv);
				advance_and_return_if_error_or_eof({});

				// 'x' or 'X'
				if (!is_match(*cp, U'x', U'X'))
					set_error_and_return_default("expected 'x' or 'X', saw '"sv, to_sv(*cp), "'"sv);
				advance_and_return_if_error_or_eof({});

				// <HEX DIGITS> ([.]<HEX DIGITS>)? [pP] [+-]? <DEC DIGITS>

				// consume value fragments
				struct fragment
				{
					char chars[24];
					size_t length;
					double value;
				};
				fragment fragments[] =
				{
					{}, // mantissa, whole part
					{}, // mantissa, fractional part
					{}  // exponent
				};
				fragment* current_fragment = fragments;
				const utf8_codepoint* prev = {};
				int exponent_sign = 1;
				while (!is_eof() && !is_value_terminator(*cp))
				{
					if (*cp == U'_')
					{
						if (!prev || !is_hexadecimal_digit(*prev))
							set_error_and_return_default("underscores may only follow digits"sv);

						prev = cp;
						advance_and_return_if_error_or_eof({});
						continue;
					}
					else if (prev && *prev == U'_' && !is_hexadecimal_digit(*cp))
						set_error_and_return_default("underscores must be followed by digits"sv);
					else if (*cp == U'.')
					{
						// 0x10.0p-.0 (exponent cannot have '.')
						if (current_fragment == fragments + 2)
							set_error_and_return_default("expected exponent digit or sign, saw '.'"sv);

						// 0x10.0.p-0 (multiple '.')
						else if (current_fragment == fragments + 1)
							set_error_and_return_default("expected hexadecimal digit or exponent, saw '.'"sv);

						else
							current_fragment++;
					}
					else if (is_match(*cp, U'p', U'P'))
					{
						// 0x10.0pp-0 (multiple 'p')
						if (current_fragment == fragments + 2)
							set_error_and_return_default("expected exponent digit or sign, saw '"sv, to_sv(*cp), "'"sv);

						// 0x.p-0 (mantissa is just '.')
						else if (fragments[0].length == 0_sz && fragments[1].length == 0_sz)
							set_error_and_return_default("expected hexadecimal digit, saw '"sv, to_sv(*cp), "'"sv);

						else
							current_fragment = fragments + 2;
					}
					else if (is_match(*cp, U'+', U'-'))
					{
						// 0x-10.0p-0 (sign in mantissa)
						if (current_fragment != fragments + 2)
							set_error_and_return_default("expected hexadecimal digit or '.', saw '"sv, to_sv(*cp), "'"sv);

						// 0x10.0p0- (misplaced exponent sign)
						else if (!is_match(*prev, U'p', U'P'))
							set_error_and_return_default("expected exponent digit, saw '"sv, to_sv(*cp), "'"sv);

						else
							exponent_sign = *cp == U'-' ? -1 : 1;
					}
					else if (current_fragment < fragments + 2 && !is_hexadecimal_digit(*cp))
						set_error_and_return_default("expected hexadecimal digit or '.', saw '"sv, to_sv(*cp), "'"sv);
					else if (current_fragment == fragments + 2 && !is_decimal_digit(*cp))
						set_error_and_return_default("expected exponent digit or sign, saw '"sv, to_sv(*cp), "'"sv);
					else if (current_fragment->length == sizeof(fragment::chars))
						set_error_and_return_default(
							"fragment exceeeds maximum length of "sv,
							static_cast<uint64_t>(sizeof(fragment::chars)), " characters"sv
						);
					else
						current_fragment->chars[current_fragment->length++] = static_cast<char>(cp->bytes[0]);

					prev = cp;
					advance_and_return_if_error({});
				}

				// sanity-check ending state
				if (current_fragment != fragments + 2 || current_fragment->length == 0_sz)
				{
					set_error_and_return_if_eof({});
					set_error_and_return_default("missing exponent"sv);
				}
				else if (prev && *prev == U'_')
				{
					set_error_and_return_if_eof({});
					set_error_and_return_default("underscores must be followed by digits"sv);
				}

				// calculate values for the three fragments
				for (int fragment_idx = 0; fragment_idx < 3; fragment_idx++)
				{
					auto& f = fragments[fragment_idx];
					const uint32_t base = fragment_idx == 2 ? 10 : 16;

					// left-trim zeroes
					const char* c = f.chars;
					size_t sig = {};
					while (f.length && *c == '0')
					{
						f.length--;
						c++;
						sig++;
					}
					if (!f.length)
						continue;

					// calculate value
					auto place = 1u;
					for (size_t i = 0; i < f.length - 1_sz; i++)
						place *= base;
					uint32_t val{};
					while (place)
					{
						if (base == 16)
							val += place * hex_to_dec(*c);
						else
							val += place * static_cast<uint32_t>(*c - '0');
						if (fragment_idx == 1)
							sig++;
						c++;
						place /= base;
					}
					f.value = static_cast<double>(val);

					// shift the fractional part
					if (fragment_idx == 1)
					{
						while (sig--)
							f.value /= base;
					}
				}

				return (fragments[0].value + fragments[1].value)
					* pow(2.0, fragments[2].value * exponent_sign)
					* sign;

				#else // !TOML_LANG_UNRELEASED

				set_error_and_return_default(
					"hexadecimal floating-point values are not supported "
					"in TOML 1.0.0 and earlier"sv
				);

				#endif // !TOML_LANG_UNRELEASED
			}

			template <uint64_t base>
			[[nodiscard]]
			int64_t parse_integer() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				using traits = parse_integer_traits<base>;
				push_parse_scope(traits::scope_qualifier);

				[[maybe_unused]] int64_t sign = 1;
				if constexpr (traits::is_signed)
				{
					sign = *cp == U'-' ? -1 : 1;
					if (is_match(*cp, U'+', U'-'))
						advance_and_return_if_error_or_eof({});
				}

				if constexpr (base == 10)
				{
					if (!traits::is_digit(*cp))
						set_error_and_return_default("expected expected digit or sign, saw '"sv, to_sv(*cp), "'"sv);
				}
				else
				{
					// '0'
					if (*cp != U'0')
						set_error_and_return_default("expected '0', saw '"sv, to_sv(*cp), "'"sv);
					advance_and_return_if_error_or_eof({});

					// 'b', 'o', 'x'
					if (*cp != traits::prefix_codepoint)
						set_error_and_return_default("expected '"sv, traits::prefix, "', saw '"sv, to_sv(*cp), "'"sv);
					advance_and_return_if_error_or_eof({});
				}

				// consume value chars
				char chars[traits::buffer_length];
				size_t length = {};
				const utf8_codepoint* prev = {};
				while (!is_eof() && !is_value_terminator(*cp))
				{
					if (*cp == U'_')
					{
						if (!prev || !traits::is_digit(*prev))
							set_error_and_return_default("underscores may only follow digits"sv);

						prev = cp;
						advance_and_return_if_error_or_eof({});
						continue;
					}
					else if (prev && *prev == U'_' && !traits::is_digit(*cp))
						set_error_and_return_default("underscores must be followed by digits"sv);
					else if (!traits::is_digit(*cp))
						set_error_and_return_default("expected digit, saw '"sv, to_sv(*cp), "'"sv);
					else if (length == sizeof(chars))
						set_error_and_return_default(
							"exceeds maximum length of "sv,
							static_cast<uint64_t>(sizeof(chars)), " characters"sv
						);
					else
						chars[length++] = static_cast<char>(cp->bytes[0]);

					prev = cp;
					advance_and_return_if_error({});
				}

				// sanity check ending state
				if (prev && *prev == U'_')
				{
					set_error_and_return_if_eof({});
					set_error_and_return_default("underscores must be followed by digits"sv);
				}

				// check for leading zeroes
				if constexpr (base == 10)
				{
					if (chars[0] == '0')
						set_error_and_return_default("leading zeroes are prohibited"sv);
				}

				// single digits can be converted trivially
				if (length == 1_sz)
				{
					if constexpr (base == 16)
						return static_cast<int64_t>(hex_to_dec(chars[0]));
					else if constexpr (base <= 10)
						return static_cast<int64_t>(chars[0] - '0');
				}

				// otherwise do the thing
				uint64_t result = {};
				{
					const char* msd = chars;
					const char* end = msd + length;
					while (msd < end && *msd == '0')
						msd++;
					if (msd == end)
						return 0ll;
					uint64_t power = 1;
					while (--end >= msd)
					{
						if constexpr (base == 16)
							result += power * hex_to_dec(*end);
						else
							result += power * static_cast<uint64_t>(*end - '0');
						power *= base;
					}
				}

				// range check
				if (result > static_cast<uint64_t>((std::numeric_limits<int64_t>::max)()) + (sign < 0 ? 1ull : 0ull))
					set_error_and_return_default(
						"'"sv, std::string_view{ chars, length }, "' is not representable in 64 bits"sv
					);

				if constexpr (traits::is_signed)
					return static_cast<int64_t>(result) * sign;
				else
					return static_cast<int64_t>(result);
			}

			[[nodiscard]]
			date parse_date(bool part_of_datetime = false) TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_decimal_digit(*cp));
				push_parse_scope("date"sv);

				// "YYYY"
				uint32_t digits[4];
				if (!consume_digit_sequence(digits, 4_sz))
					set_error_and_return_default("expected 4-digit year, saw '"sv, to_sv(*cp), "'"sv);
				const auto year = digits[3]
					+ digits[2] * 10u
					+ digits[1] * 100u
					+ digits[0] * 1000u;
				const auto is_leap_year = (year % 4u == 0u) && ((year % 100u != 0u) || (year % 400u == 0u));
				set_error_and_return_if_eof({});

				// '-'
				if (*cp != U'-')
					set_error_and_return_default("expected '-', saw '"sv, to_sv(*cp), "'"sv);
				advance_and_return_if_error_or_eof({});

				// "MM"
				if (!consume_digit_sequence(digits, 2_sz))
					set_error_and_return_default("expected 2-digit month, saw '"sv, to_sv(*cp), "'"sv);
				const auto month = digits[1] + digits[0] * 10u;
				if (month == 0u || month > 12u)
					set_error_and_return_default(
						"expected month between 1 and 12 (inclusive), saw "sv, static_cast<uint64_t>(month)
					);
				const auto max_days_in_month =
					month == 2u
					? (is_leap_year ? 29u : 28u)
					: (month == 4u || month == 6u || month == 9u || month == 11u ? 30u : 31u)
				;
				set_error_and_return_if_eof({});

				// '-'
				if (*cp != U'-')
					set_error_and_return_default("expected '-', saw '"sv, to_sv(*cp), "'"sv);
				advance_and_return_if_error_or_eof({});

				// "DD"
				if (!consume_digit_sequence(digits, 2_sz))
					set_error_and_return_default("expected 2-digit day, saw '"sv, to_sv(*cp), "'"sv);
				const auto day = digits[1] + digits[0] * 10u;
				if (day == 0u || day > max_days_in_month)
					set_error_and_return_default(
						"expected day between 1 and "sv, static_cast<uint64_t>(max_days_in_month),
						" (inclusive), saw "sv, static_cast<uint64_t>(day)
					);

				if (!part_of_datetime && !is_eof() && !is_value_terminator(*cp))
					set_error_and_return_default("expected value-terminator, saw '"sv, to_sv(*cp), "'"sv);

				return
				{
					static_cast<uint16_t>(year),
					static_cast<uint8_t>(month),
					static_cast<uint8_t>(day)
				};
			}

			[[nodiscard]]
			time parse_time(bool part_of_datetime = false) TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_decimal_digit(*cp));
				push_parse_scope("time"sv);

				static constexpr auto max_digits = 9_sz;
				uint32_t digits[max_digits];

				// "HH"
				if (!consume_digit_sequence(digits, 2_sz))
					set_error_and_return_default("expected 2-digit hour, saw '"sv, to_sv(*cp), "'"sv);
				const auto hour = digits[1] + digits[0] * 10u;
				if (hour > 23u)
					set_error_and_return_default(
						"expected hour between 0 to 59 (inclusive), saw "sv, static_cast<uint64_t>(hour)
					);
				set_error_and_return_if_eof({});

				// ':'
				if (*cp != U':')
					set_error_and_return_default("expected ':', saw '"sv, to_sv(*cp), "'"sv);
				advance_and_return_if_error_or_eof({});

				// "MM"
				if (!consume_digit_sequence(digits, 2_sz))
					set_error_and_return_default("expected 2-digit minute, saw '"sv, to_sv(*cp), "'"sv);
				const auto minute = digits[1] + digits[0] * 10u;
				if (minute > 59u)
					set_error_and_return_default(
						"expected minute between 0 and 59 (inclusive), saw "sv, static_cast<uint64_t>(minute)
					);
				auto time = ::toml::time{
					static_cast<uint8_t>(hour),
					static_cast<uint8_t>(minute),
				};

				// ':'
				if constexpr (TOML_LANG_UNRELEASED) // toml/issues/671 (allow omission of seconds)
				{
					if (is_eof()
						|| is_value_terminator(*cp)
						|| (part_of_datetime && is_match(*cp, U'+', U'-', U'Z')))
						return time;
				}
				else
					set_error_and_return_if_eof({});
				if (*cp != U':')
					set_error_and_return_default("expected ':', saw '"sv, to_sv(*cp), "'"sv);
				advance_and_return_if_error_or_eof({});

				// "SS"
				if (!consume_digit_sequence(digits, 2_sz))
					set_error_and_return_default("expected 2-digit second, saw '"sv, to_sv(*cp), "'"sv);
				const auto second = digits[1] + digits[0] * 10u;
				if (second > 59u)
					set_error_and_return_default(
						"expected second between 0 and 59 (inclusive), saw "sv, static_cast<uint64_t>(second)
					);
				time.second = static_cast<uint8_t>(second);

				// '.' (early-exiting is allowed; fractional is optional)
				if (is_eof()
					|| is_value_terminator(*cp)
					|| (part_of_datetime && is_match(*cp, U'+', U'-', U'Z')))
					return time;
				if (*cp != U'.')
					set_error_and_return_default("expected '.', saw '"sv, to_sv(*cp), "'"sv);
				advance_and_return_if_error_or_eof({});

				// "FFFFFFFFF"
				auto digit_count = consume_variable_length_digit_sequence(digits, max_digits);
				if (!digit_count)
				{
					set_error_and_return_if_eof({});
					set_error_and_return_default("expected fractional digits, saw '"sv, to_sv(*cp), "'"sv);
				}
				else if (!is_eof())
				{
					if (digit_count == max_digits && is_decimal_digit(*cp))
						set_error_and_return_default(
							"fractional component exceeds maximum precision of "sv, static_cast<uint64_t>(max_digits)
						);
					else if (!part_of_datetime && !is_value_terminator(*cp))
						set_error_and_return_default("expected value-terminator, saw '"sv, to_sv(*cp), "'"sv);
				}

				uint32_t value = 0u;
				uint32_t place = 1u;
				for (auto i = digit_count; i --> 0_sz;)
				{
					value += digits[i] * place;
					place *= 10u;
				}
				for (auto i = digit_count; i < max_digits; i++) //implicit zeros
					value *= 10u;
				time.nanosecond = value;
				return time;
			}

			[[nodiscard]]
			date_time parse_date_time() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_decimal_digit(*cp));
				push_parse_scope("date-time"sv);

				// "YYYY-MM-DD"
				auto date = parse_date(true);
				set_error_and_return_if_eof({});

				// ' ' or 'T'
				if (!is_match(*cp, U' ', U'T'))
					set_error_and_return_default("expected space or 'T', saw '"sv, to_sv(*cp), "'"sv);
				advance_and_return_if_error_or_eof({});

				// "HH:MM:SS.FFFFFFFFF"
				auto time = parse_time(true);
				return_if_error({});

				// no offset
				if (is_eof() || is_value_terminator(*cp))
					return { date, time };

				// zero offset ("Z")
				time_offset offset;
				if (*cp == U'Z')
					advance_and_return_if_error({});

				// explicit offset ("+/-HH:MM")
				else if (is_match(*cp, U'+', U'-'))
				{
					push_parse_scope("date-time offset"sv);

					// sign
					int sign = *cp == U'-' ? -1 : 1;
					advance_and_return_if_error_or_eof({});

					// "HH"
					int digits[2];
					if (!consume_digit_sequence(digits, 2_sz))
						set_error_and_return_default("expected 2-digit hour, saw '"sv, to_sv(*cp), "'"sv);
					const auto hour = digits[1] + digits[0] * 10;
					if (hour > 23)
						set_error_and_return_default(
							"expected hour between 0 and 23 (inclusive), saw "sv, static_cast<int64_t>(hour)
						);
					set_error_and_return_if_eof({});

					// ':'
					if (*cp != U':')
						set_error_and_return_default("expected ':', saw '"sv, to_sv(*cp), "'"sv);
					advance_and_return_if_error_or_eof({});

					// "MM"
					if (!consume_digit_sequence(digits, 2_sz))
						set_error_and_return_default("expected 2-digit minute, saw '"sv, to_sv(*cp), "'"sv);
					const auto minute = digits[1] + digits[0] * 10;
					if (minute > 59)
						set_error_and_return_default(
							"expected minute between 0 and 59 (inclusive), saw "sv, static_cast<int64_t>(minute)
						);
					offset.minutes = static_cast<int16_t>((hour * 60 + minute) * sign);
				}

				if (!is_eof() && !is_value_terminator(*cp))
					set_error_and_return_default("expected value-terminator, saw '"sv, to_sv(*cp), "'"sv);

				return { date, time, offset };
			}

			TOML_POP_WARNINGS // TOML_DISABLE_SWITCH_WARNINGS, TOML_DISABLE_INIT_WARNINGS

			[[nodiscard]] toml::array* parse_array() TOML_MAY_THROW;
			[[nodiscard]] toml::table* parse_inline_table() TOML_MAY_THROW;
			[[nodiscard]]
			node* parse_value_known_prefixes() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(!is_control_character(*cp));
				assert_or_assume(*cp != U'_');

				switch (cp->value)
				{
					// arrays
					case U'[':
						return parse_array();

					// inline tables
					case U'{':
						return parse_inline_table();

						// floats beginning with '.'
					case U'.':
						return new value{ parse_float() };

						// strings
					case U'"': [[fallthrough]];
					case U'\'':
						return new value{ std::move(parse_string().value) };

						// bools
					case U't': [[fallthrough]];
					case U'f': [[fallthrough]];
					case U'T': [[fallthrough]];
					case U'F':
						return new value{ parse_boolean() };

						// inf/nan
					case U'i': [[fallthrough]];
					case U'I': [[fallthrough]];
					case U'n': [[fallthrough]];
					case U'N':
						return new value{ parse_inf_or_nan() };
				}
				return nullptr;
			}

			[[nodiscard]]
			node* parse_value() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(!is_value_terminator(*cp));
				push_parse_scope("value"sv);

				// check if it begins with some control character
				// (note that this will also fail for whitespace but we're assuming we've
				// called consume_leading_whitespace() before calling parse_value())
				if TOML_UNLIKELY(is_control_character(*cp))
					set_error_and_return_default("unexpected control character"sv);

				// underscores at the beginning
				else if (*cp == U'_')
					set_error_and_return_default("values may not begin with underscores"sv);

				const auto begin_pos = cp->position;
				node* val{};

				do
				{
					assert_or_assume(!is_control_character(*cp));
					assert_or_assume(*cp != U'_');

					// detect the value type and parse accordingly,
					// starting with value types that can be detected
					// unambiguously from just one character.

					val = parse_value_known_prefixes();
					return_if_error({});
					if (val)
						break;

					// value types from here down require more than one character to unambiguously identify
					// so scan ahead and collect a set of value 'traits'.
					enum value_traits : int
					{
						has_nothing		= 0,
						has_digits		= 1,
						has_b			= 1 << 1, // as second char only (0b)
						has_e			= 1 << 2, // only float exponents
						has_o			= 1 << 3, // as second char only (0o)
						has_p			= 1 << 4, // only hexfloat exponents
						has_t			= 1 << 5,
						has_x			= 1 << 6, // as second or third char only (0x, -0x, +0x)
						has_z			= 1 << 7,
						has_colon		= 1 << 8,
						has_plus		= 1 << 9,
						has_minus		= 1 << 10,
						has_dot			= 1 << 11,
						begins_sign		= 1 << 12,
						begins_digit	= 1 << 13,
						begins_zero		= 1 << 14

						// Q: "why not make these real values in the enum??"
						// A: because the visual studio debugger stops treating them as a set of flags if you add
						// non-pow2 values, making them much harder to debug.
						#define signs_msk (has_plus | has_minus)
						#define bzero_msk (begins_zero | has_digits)
						#define bdigit_msk (begins_digit | has_digits)
					};
					value_traits traits = has_nothing;
					const auto has_any = [&](auto t) noexcept { return (traits & t) != has_nothing; };
					const auto has_none = [&](auto t) noexcept { return (traits & t) == has_nothing; };
					const auto add_trait = [&](auto t) noexcept { traits = static_cast<value_traits>(traits | t); };

					// examine the first character to get the 'begins with' traits
					// (good fail-fast opportunity; all the remaining types begin with numeric digits or signs)
					if (is_decimal_digit(*cp))
						add_trait(*cp == U'0' ? begins_zero : begins_digit);
					else if (is_match(*cp, U'+', U'-'))
						add_trait(begins_sign);
					else
						break;

					// scan the rest of the value to determine the remaining traits
					char32_t chars[utf8_buffered_reader::max_history_length];
					size_t char_count = {}, advance_count = {};
					bool eof_while_scanning = false;
					const auto scan = [&]() TOML_MAY_THROW
					{
						if (is_eof())
							return;
						assert_or_assume(!is_value_terminator(*cp));

						do
						{
							if (const auto c = **cp; c != U'_')
							{
								chars[char_count++] = c;

								if (is_decimal_digit(c))
									add_trait(has_digits);
								else if (is_ascii_letter(c))
								{
									assert_or_assume((c >= U'a' && c <= U'z') || (c >= U'A' && c <= U'Z'));
									switch (static_cast<char32_t>(c | 32u))
									{
										case U'b':
											if (char_count == 2_sz && has_any(begins_zero))
												add_trait(has_b);
											break;

										case U'e':
											if (char_count > 1_sz
												&& has_none(has_b | has_o | has_p | has_t | has_x | has_z | has_colon)
												&& (has_none(has_plus | has_minus) || has_any(begins_sign)))
												add_trait(has_e);
											break;

										case U'o':
											if (char_count == 2_sz && has_any(begins_zero))
												add_trait(has_o);
											break;

										case U'p':
											if (has_any(has_x))
												add_trait(has_p);
											break;

										case U'x':
											if ((char_count == 2_sz && has_any(begins_zero))
												|| (char_count == 3_sz && has_any(begins_sign) && chars[1] == U'0'))
												add_trait(has_x);
											break;

										case U't': add_trait(has_t); break;
										case U'z': add_trait(has_z); break;
									}
								}
								else if (c <= U':')
								{
									assert_or_assume(c < U'0' || c > U'9');
									switch (c)
									{
										case U'+': add_trait(has_plus); break;
										case U'-': add_trait(has_minus); break;
										case U'.': add_trait(has_dot); break;
										case U':': add_trait(has_colon); break;
									}
								}
							}

							advance_and_return_if_error();
							advance_count++;
							eof_while_scanning = is_eof();
						}
						while (advance_count < utf8_buffered_reader::max_history_length
							&& !is_eof()
							&& !is_value_terminator(*cp)
						);
					};
					scan();
					return_if_error({});

					// force further scanning if this could have been a date-time with a space instead of a T
					if (char_count == 10_sz
						&& traits == (bdigit_msk | has_minus)
						&& chars[4] == U'-'
						&& chars[7] == U'-'
						&& !is_eof()
						&& *cp == U' ')
					{
						const auto pre_advance_count = advance_count;
						const auto pre_scan_traits = traits;
						chars[char_count++] = *cp;
						add_trait(has_t);

						const auto backpedal = [&]() noexcept
						{
							go_back(advance_count - pre_advance_count);
							advance_count = pre_advance_count;
							traits = pre_scan_traits;
							char_count = 10_sz;
						};

						advance_and_return_if_error({});
						advance_count++;

						if (is_eof() || !is_decimal_digit(*cp))
							backpedal();
						else
						{
							chars[char_count++] = *cp;

							advance_and_return_if_error({});
							advance_count++;

							scan();
							return_if_error({});

							if (char_count == 12_sz)
								backpedal();
						}
					}

					// set the reader back to where we started
					go_back(advance_count);
					if (char_count < utf8_buffered_reader::max_history_length - 1_sz)
						chars[char_count] = U'\0';

					// if after scanning ahead we still only have one value character,
					// the only valid value type is an integer.
					if (char_count == 1_sz)
					{
						if (has_any(begins_zero | begins_digit))
						{
							val = new value{ static_cast<int64_t>(chars[0] - U'0') };
							advance(); //skip the digit
							break;
						}

						//anything else would be ambiguous.
						else
							set_error_and_return_default(
								eof_while_scanning
									? "encountered end-of-file"sv
									: "could not determine value type"sv
							);
					}

					// now things that can be identified from two or more characters
					return_if_error({});
					assert_or_assume(char_count >= 2_sz);

					// do some 'fuzzy matching' where there's no ambiguity, since that allows the specific
					// typed parse functions to take over and show better diagnostics if there's an issue
					// (as opposed to the fallback "could not determine type" message)
					if (has_any(has_p))
						val = new value{ parse_hex_float() };
					else if (has_any(has_x))
					{
						val = new value{ parse_integer<16>() };
						reinterpret_cast<value<int64_t>*>(val)->flags(value_flags::format_as_hexadecimal);
					}
					else if (has_any(has_o))
					{
						val = new value{ parse_integer<8>() };
						reinterpret_cast<value<int64_t>*>(val)->flags(value_flags::format_as_octal);
					}
					else if (has_any(has_b))
					{
						val = new value{ parse_integer<2>() };
						reinterpret_cast<value<int64_t>*>(val)->flags(value_flags::format_as_binary);
					}
					else if (has_any(has_e) || (has_any(begins_zero | begins_digit) && chars[1] == U'.'))
						val = new value{ parse_float() };
					else if (has_any(begins_sign))
					{
						// single-digit signed integers
						if (char_count == 2_sz && has_any(has_digits))
						{
							val = new value{
								static_cast<int64_t>(chars[1] - U'0')
								* (chars[0] == U'-' ? -1LL : 1LL)
							};
							advance(); //skip the sign
							advance(); //skip the digit
							break;
						}

						// simple signed floats (e.g. +1.0)
						if (is_decimal_digit(chars[1]) && chars[2] == U'.')
							val = new value{ parse_float() };

						// signed infinity or nan
						else if (is_match(chars[1], U'i', U'n', U'I', U'N'))
							val = new value{ parse_inf_or_nan() };
					}

					return_if_error({});
					if (val)
						break;

					// match trait masks against what they can match exclusively.
					// all correct value parses will come out of this list, so doing this as a switch is likely to
					// be a better friend to the optimizer on the success path (failure path can be slow but that
					// doesn't matter much).
					switch (unwrap_enum(traits))
					{
						//=================== binary integers
						// 0b10
						case bzero_msk | has_b:
							val = new value{ parse_integer<2>() };
							reinterpret_cast<value<int64_t>*>(val)->flags(value_flags::format_as_binary);
							break;

						//=================== octal integers
						// 0o10
						case bzero_msk | has_o:
							val = new value{ parse_integer<8>() };
							reinterpret_cast<value<int64_t>*>(val)->flags(value_flags::format_as_octal);
							break;

						//=================== decimal integers
						// 00
						// 10
						// +10
						// -10
						case bzero_msk:															[[fallthrough]];
						case bdigit_msk:														[[fallthrough]];
						case begins_sign | has_digits | has_minus:								[[fallthrough]];
						case begins_sign | has_digits | has_plus:
							val = new value{ parse_integer<10>() };
							break;

						//=================== hexadecimal integers
						// 0x10
						case bzero_msk | has_x:
							val = new value{ parse_integer<16>() };
							reinterpret_cast<value<int64_t>*>(val)->flags(value_flags::format_as_hexadecimal);
							break;

						//=================== decimal floats
						// 0e1
						// 0e-1
						// 0e+1
						// 0.0
						// 0.0e1
						// 0.0e-1
						// 0.0e+1
						case bzero_msk | has_e:													[[fallthrough]];
						case bzero_msk | has_e | has_minus:										[[fallthrough]];
						case bzero_msk | has_e | has_plus:										[[fallthrough]];
						case bzero_msk | has_dot:												[[fallthrough]];
						case bzero_msk | has_dot | has_e:										[[fallthrough]];
						case bzero_msk | has_dot | has_e | has_minus:							[[fallthrough]];
						case bzero_msk | has_dot | has_e | has_plus:							[[fallthrough]];
						// 1e1
						// 1e-1
						// 1e+1
						// 1.0
						// 1.0e1
						// 1.0e-1
						// 1.0e+1
						case bdigit_msk | has_e:												[[fallthrough]];
						case bdigit_msk | has_e | has_minus:									[[fallthrough]];
						case bdigit_msk | has_e | has_plus:										[[fallthrough]];
						case bdigit_msk | has_dot:												[[fallthrough]];
						case bdigit_msk | has_dot | has_e:										[[fallthrough]];
						case bdigit_msk | has_dot | has_e | has_minus:							[[fallthrough]];
						case bdigit_msk | has_dot | has_e | has_plus:							[[fallthrough]];
						// +1e1
						// +1.0
						// +1.0e1
						// +1.0e+1
						// +1.0e-1
						// -1.0e+1
						case begins_sign | has_digits | has_e | has_plus:						[[fallthrough]];
						case begins_sign | has_digits | has_dot | has_plus:						[[fallthrough]];
						case begins_sign | has_digits | has_dot | has_e | has_plus:				[[fallthrough]];
						case begins_sign | has_digits | has_dot | has_e | signs_msk:			[[fallthrough]];
						// -1e1
						// -1e+1
						// +1e-1
						// -1.0
						// -1.0e1
						// -1.0e-1
						case begins_sign | has_digits | has_e | has_minus:						[[fallthrough]];
						case begins_sign | has_digits | has_e | signs_msk:						[[fallthrough]];
						case begins_sign | has_digits | has_dot | has_minus:					[[fallthrough]];
						case begins_sign | has_digits | has_dot | has_e | has_minus:
							val = new value{ parse_float() };
							break;

						//=================== hexadecimal floats
						// 0x10p0
						// 0x10p-0
						// 0x10p+0
						case bzero_msk | has_x | has_p:											[[fallthrough]];
						case bzero_msk | has_x | has_p | has_minus:								[[fallthrough]];
						case bzero_msk | has_x | has_p | has_plus:								[[fallthrough]];
						// -0x10p0
						// -0x10p-0
						// +0x10p0
						// +0x10p+0
						// -0x10p+0
						// +0x10p-0
						case begins_sign | has_digits | has_x | has_p | has_minus:				[[fallthrough]];
						case begins_sign | has_digits | has_x | has_p | has_plus:				[[fallthrough]];
						case begins_sign | has_digits | has_x | has_p | signs_msk:				[[fallthrough]];
						// 0x10.1p0
						// 0x10.1p-0
						// 0x10.1p+0
						case bzero_msk | has_x | has_dot | has_p:								[[fallthrough]];
						case bzero_msk | has_x | has_dot | has_p | has_minus:					[[fallthrough]];
						case bzero_msk | has_x | has_dot | has_p | has_plus:					[[fallthrough]];
						// -0x10.1p0
						// -0x10.1p-0
						// +0x10.1p0
						// +0x10.1p+0
						// -0x10.1p+0
						// +0x10.1p-0
						case begins_sign | has_digits | has_x | has_dot | has_p | has_minus:	[[fallthrough]];
						case begins_sign | has_digits | has_x | has_dot | has_p | has_plus:		[[fallthrough]];
						case begins_sign | has_digits | has_x | has_dot | has_p | signs_msk:
							val = new value{ parse_hex_float() };
							break;

						//=================== times
						// HH:MM
						// HH:MM:SS
						// HH:MM:SS.FFFFFF
						case bzero_msk | has_colon:												[[fallthrough]];
						case bzero_msk | has_colon | has_dot:									[[fallthrough]];
						case bdigit_msk | has_colon:											[[fallthrough]];
						case bdigit_msk | has_colon | has_dot:
							val = new value{ parse_time() };
							break;

						//=================== local dates
						// YYYY-MM-DD
						case bzero_msk | has_minus:												[[fallthrough]];
						case bdigit_msk | has_minus:
							val = new value{ parse_date() };
							break;

						//=================== date-times
						// YYYY-MM-DDTHH:MM
						// YYYY-MM-DDTHH:MM-HH:MM
						// YYYY-MM-DDTHH:MM+HH:MM
						// YYYY-MM-DD HH:MM
						// YYYY-MM-DD HH:MM-HH:MM
						// YYYY-MM-DD HH:MM+HH:MM
						// YYYY-MM-DDTHH:MM:SS
						// YYYY-MM-DDTHH:MM:SS-HH:MM
						// YYYY-MM-DDTHH:MM:SS+HH:MM
						// YYYY-MM-DD HH:MM:SS
						// YYYY-MM-DD HH:MM:SS-HH:MM
						// YYYY-MM-DD HH:MM:SS+HH:MM
						case bzero_msk | has_minus | has_colon | has_t:							[[fallthrough]];
						case bzero_msk | signs_msk | has_colon | has_t:							[[fallthrough]];
						case bdigit_msk | has_minus | has_colon | has_t:						[[fallthrough]];
						case bdigit_msk | signs_msk | has_colon | has_t:						[[fallthrough]];
						// YYYY-MM-DDTHH:MM:SS.FFFFFF
						// YYYY-MM-DDTHH:MM:SS.FFFFFF-HH:MM
						// YYYY-MM-DDTHH:MM:SS.FFFFFF+HH:MM
						// YYYY-MM-DD HH:MM:SS.FFFFFF
						// YYYY-MM-DD HH:MM:SS.FFFFFF-HH:MM
						// YYYY-MM-DD HH:MM:SS.FFFFFF+HH:MM
						case bzero_msk | has_minus | has_colon | has_dot | has_t:				[[fallthrough]];
						case bzero_msk | signs_msk | has_colon | has_dot | has_t:				[[fallthrough]];
						case bdigit_msk | has_minus | has_colon | has_dot | has_t:				[[fallthrough]];
						case bdigit_msk | signs_msk | has_colon | has_dot | has_t:				[[fallthrough]];
						// YYYY-MM-DDTHH:MMZ
						// YYYY-MM-DD HH:MMZ
						// YYYY-MM-DDTHH:MM:SSZ
						// YYYY-MM-DD HH:MM:SSZ
						// YYYY-MM-DDTHH:MM:SS.FFFFFFZ
						// YYYY-MM-DD HH:MM:SS.FFFFFFZ
						case bzero_msk | has_minus | has_colon | has_z | has_t:					[[fallthrough]];
						case bzero_msk | has_minus | has_colon | has_dot | has_z | has_t:		[[fallthrough]];
						case bdigit_msk | has_minus | has_colon | has_z | has_t:				[[fallthrough]];
						case bdigit_msk | has_minus | has_colon | has_dot | has_z | has_t:
							val = new value{ parse_date_time() };
							break;
					}

					#undef signs_msk
					#undef bzero_msk
					#undef bdigit_msk
				}
				while (false);

				if (!val)
				{
					set_error_at(begin_pos, "could not determine value type"sv);
					return_after_error({});
				}

				#if !TOML_LANG_AT_LEAST(1, 0, 0) // toml/issues/665 (heterogeneous arrays)
				{
					if (auto arr = val->as_array(); arr && !arr->is_homogeneous())
					{
						delete arr;
						set_error_at(
							begin_pos,
							"arrays cannot contain values of different types before TOML 1.0.0"sv
						);
						return_after_error({});
					}
				}
				#endif

				val->source_ = { begin_pos, current_position(1), reader.source_path() };
				return val;
			}

			[[nodiscard]]
			parsed_key parse_key() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_bare_key_character(*cp) || is_string_delimiter(*cp));
				push_parse_scope("key"sv);

				parsed_key key;
				recording_whitespace = false;

				while (!is_error())
				{
					#if TOML_LANG_UNRELEASED // toml/issues/687 (unicode bare keys)
					if (is_unicode_combining_mark(*cp))
						set_error_and_return_default("bare keys may not begin with unicode combining marks"sv);
					else
					#endif

					// bare_key_segment
					if (is_bare_key_character(*cp))
						key.segments.emplace_back(parse_bare_key_segment());

					// "quoted key segment"
					else if (is_string_delimiter(*cp))
					{
						const auto begin_pos = cp->position;

						recording_whitespace = true;
						auto str = parse_string();
						recording_whitespace = false;
						return_if_error({});

						if (str.was_multi_line)
						{
							set_error_at(
								begin_pos,
								"multi-line strings are prohibited in "sv,
								key.segments.empty() ? ""sv : "dotted "sv,
								"keys"sv
							);
							return_after_error({});
						}
						else
							key.segments.emplace_back(std::move(str.value));
					}

					// ???
					else
						set_error_and_return_default(
							"expected bare key starting character or string delimiter, saw '"sv, to_sv(*cp), "'"sv
						);

					// whitespace following the key segment
					consume_leading_whitespace();

					// eof or no more key to come
					if (is_eof() || *cp != U'.')
						break;

					// was a dotted key, so go around again to consume the next segment
					advance_and_return_if_error_or_eof({});
					consume_leading_whitespace();
					set_error_and_return_if_eof({});
				}
				return_if_error({});
				return key;
			}

			[[nodiscard]]
			parsed_key_value_pair parse_key_value_pair() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(is_string_delimiter(*cp) || is_bare_key_character(*cp));
				push_parse_scope("key-value pair"sv);

				// get the key
				start_recording();
				auto key = parse_key();
				stop_recording(1_sz);

				// skip past any whitespace that followed the key
				consume_leading_whitespace();
				set_error_and_return_if_eof({});

				// '='
				if (*cp != U'=')
					set_error_and_return_default("expected '=', saw '"sv, to_sv(*cp), "'"sv);
				advance_and_return_if_error_or_eof({});

				// skip past any whitespace that followed the '='
				consume_leading_whitespace();
				return_if_error({});
				set_error_and_return_if_eof({});

				// get the value
				if (is_value_terminator(*cp))
					set_error_and_return_default("expected value, saw '"sv, to_sv(*cp), "'"sv);
				return { std::move(key), parse_value() };
			}

			[[nodiscard]]
			toml::table* parse_table_header() TOML_MAY_THROW
			{
				return_if_error({});
				assert_not_eof();
				assert_or_assume(*cp == U'[');
				push_parse_scope("table header"sv);

				const auto header_begin_pos = cp->position;
				source_position header_end_pos;
				parsed_key key;
				bool is_arr = false;

				//parse header
				{
					// skip first '['
					advance_and_return_if_error_or_eof({});

					// skip past any whitespace that followed the '['
					const bool had_leading_whitespace = consume_leading_whitespace();
					set_error_and_return_if_eof({});

					// skip second '[' (if present)
					if (*cp == U'[')
					{
						if (had_leading_whitespace)
							set_error_and_return_default(
								"[[array-of-table]] brackets must be contiguous (i.e. [ [ this ] ] is prohibited)"sv
							);

						is_arr = true;
						advance_and_return_if_error_or_eof({});

						// skip past any whitespace that followed the '['
						consume_leading_whitespace();
						set_error_and_return_if_eof({});
					}

					// check for a premature closing ']'
					if (*cp == U']')
						set_error_and_return_default("tables with blank bare keys are explicitly prohibited"sv);

					// get the actual key
					start_recording();
					key = parse_key();
					stop_recording(1_sz);
					return_if_error({});

					// skip past any whitespace that followed the key
					consume_leading_whitespace();
					return_if_error({});
					set_error_and_return_if_eof({});

					// consume the closing ']'
					if (*cp != U']')
						set_error_and_return_default("expected ']', saw '"sv, to_sv(*cp), "'"sv);
					if (is_arr)
					{
						advance_and_return_if_error_or_eof({});
						if (*cp != U']')
							set_error_and_return_default("expected ']', saw '"sv, to_sv(*cp), "'"sv);
					}
					advance_and_return_if_error({});
					header_end_pos = current_position(1);

					// handle the rest of the line after the header
					consume_leading_whitespace();
					if (!is_eof() && !consume_comment() && !consume_line_break())
						set_error_and_return_default("expected a comment or whitespace, saw '"sv, to_sv(*cp), "'"sv);
				}
				TOML_ASSERT(!key.segments.empty());

				// check if each parent is a table/table array, or can be created implicitly as a table.
				auto parent = &root;
				for (size_t i = 0; i < key.segments.size() - 1_sz; i++)
				{
					auto child = parent->get(key.segments[i]);
					if (!child)
					{
						child = parent->map.emplace(
							key.segments[i],
							new toml::table{}
						).first->second.get();
						implicit_tables.push_back(&child->ref_cast<table>());
						child->source_ = { header_begin_pos, header_end_pos, reader.source_path() };
						parent = &child->ref_cast<table>();
					}
					else if (child->is_table())
					{
						parent = &child->ref_cast<table>();
					}
					else if (child->is_array() && find(table_arrays, &child->ref_cast<array>()))
					{
						// table arrays are a special case;
						// the spec dictates we select the most recently declared element in the array.
						TOML_ASSERT(!child->ref_cast<array>().elements.empty());
						TOML_ASSERT(child->ref_cast<array>().elements.back()->is_table());
						parent = &child->ref_cast<array>().elements.back()->ref_cast<table>();
					}
					else
					{
						if (!is_arr && child->type() == node_type::table)
							set_error_and_return_default(
								"cannot redefine existing table '"sv, to_sv(recording_buffer), "'"sv
							);
						else
							set_error_and_return_default(
								"cannot redefine existing "sv, to_sv(child->type()),
								" '"sv, to_sv(recording_buffer),
								"' as "sv, is_arr ? "array-of-tables"sv : "table"sv
							);
					}
				}

				// check the last parent table for a node matching the last key.
				// if there was no matching node, then sweet;
				// we can freely instantiate a new table/table array.
				auto matching_node = parent->get(key.segments.back());
				if (!matching_node)
				{
					// if it's an array we need to make the array and it's first table element,
					// set the starting regions, and return the table element
					if (is_arr)
					{
						auto tab_arr = &parent->map.emplace(
								key.segments.back(),
								new toml::array{}
							).first->second->ref_cast<array>();
						table_arrays.push_back(tab_arr);
						tab_arr->source_ = { header_begin_pos, header_end_pos, reader.source_path() };

						tab_arr->elements.emplace_back(new toml::table{});
						tab_arr->elements.back()->source_ = { header_begin_pos, header_end_pos, reader.source_path() };
						return &tab_arr->elements.back()->ref_cast<table>();
					}

					//otherwise we're just making a table
					else
					{
						auto tab = &parent->map.emplace(
								key.segments.back(),
								new toml::table{})
							.first->second->ref_cast<table>();
						tab->source_ = { header_begin_pos, header_end_pos, reader.source_path() };
						return tab;
					}
				}

				// if there was already a matching node some sanity checking is necessary;
				// this is ok if we're making an array and the existing element is already an array (new element)
				// or if we're making a table and the existing element is an implicitly-created table (promote it),
				// otherwise this is a redefinition error.
				else
				{
					if (is_arr && matching_node->is_array() && find(table_arrays, &matching_node->ref_cast<array>()))
					{
						auto tab_arr = &matching_node->ref_cast<array>();
						tab_arr->elements.emplace_back(new toml::table{});
						tab_arr->elements.back()->source_ = { header_begin_pos, header_end_pos, reader.source_path() };
						return &tab_arr->elements.back()->ref_cast<table>();
					}

					else if (!is_arr
						&& matching_node->is_table()
						&& !implicit_tables.empty())
					{
						auto tbl = &matching_node->ref_cast<table>();
						if (auto found = find(implicit_tables, tbl))
						{
							implicit_tables.erase(implicit_tables.cbegin() + (found - implicit_tables.data()));
							tbl->source_.begin = header_begin_pos;
							tbl->source_.end = header_end_pos;
							return tbl;
						}
					}

					//if we get here it's a redefinition error.
					if (!is_arr && matching_node->type() == node_type::table)
						set_error_and_return_default("cannot redefine existing table '"sv, to_sv(recording_buffer), "'"sv);
					else
						set_error_and_return_default(
							"cannot redefine existing "sv, to_sv(matching_node->type()),
							" '"sv, to_sv(recording_buffer),
							"' as "sv, is_arr ? "array-of-tables"sv : "table"sv
						);
				}
			}

			void parse_key_value_pair_and_insert(toml::table* tab) TOML_MAY_THROW
			{
				return_if_error();
				assert_not_eof();
				push_parse_scope("key-value pair"sv);

				auto kvp = parse_key_value_pair();
				return_if_error();

				TOML_ASSERT(kvp.key.segments.size() >= 1_sz);
				if (kvp.key.segments.size() > 1_sz)
				{
					for (size_t i = 0; i < kvp.key.segments.size() - 1_sz; i++)
					{
						auto child = tab->get(kvp.key.segments[i]);
						if (!child)
						{
							child = tab->map.emplace(
								std::move(kvp.key.segments[i]),
								new toml::table{}
							).first->second.get();
							dotted_key_tables.push_back(&child->ref_cast<table>());
							dotted_key_tables.back()->inline_ = true;
							child->source_ = kvp.value->source_;
						}
						else if (!child->is_table() || !find(dotted_key_tables, &child->ref_cast<table>()))
							set_error("cannot redefine existing "sv, to_sv(child->type()), " as dotted key-value pair"sv);
						else
							child->source_.end = kvp.value->source_.end;
						return_if_error();
						tab = &child->ref_cast<table>();
					}
				}

				if (auto conflicting_node = tab->get(kvp.key.segments.back()))
				{
					if (conflicting_node->type() == kvp.value->type())
						set_error(
							"cannot redefine existing "sv, to_sv(conflicting_node->type()),
							" '"sv, to_sv(recording_buffer), "'"sv
						);
					else
						set_error(
							"cannot redefine existing "sv, to_sv(conflicting_node->type()),
							" '"sv, to_sv(recording_buffer),
							"' as "sv, to_sv(kvp.value->type())
						);
				}

				return_if_error();
				tab->map.emplace(
					std::move(kvp.key.segments.back()),
					std::unique_ptr<node>{ kvp.value }
				);
			}

			void parse_document() TOML_MAY_THROW
			{
				assert_not_error();
				assert_not_eof();
				push_parse_scope("root table"sv);
				table* current_table = &root;

				do
				{
					return_if_error();

					// leading whitespace, line endings, comments
					if (consume_leading_whitespace()
						|| consume_line_break()
						|| consume_comment())
						continue;

					// [tables]
					// [[table array]]
					else if (*cp == U'[')
						current_table = parse_table_header();

					// bare_keys
					// dotted.keys
					// "quoted keys"
					else if (is_bare_key_character(*cp) || is_string_delimiter(*cp))
					{
						push_parse_scope("key-value pair"sv);

						parse_key_value_pair_and_insert(current_table);

						// handle the rest of the line after the kvp
						// (this is not done in parse_key_value_pair() because that is also used for inline tables)
						consume_leading_whitespace();
						return_if_error();
						if (!is_eof() && !consume_comment() && !consume_line_break())
							set_error("expected a comment or whitespace, saw '"sv, to_sv(*cp), "'"sv);
					}

					else // ??
						set_error("expected keys, tables, whitespace or comments, saw '"sv, to_sv(*cp), "'"sv);

				}
				while (!is_eof());

				auto eof_pos = current_position(1);
				root.source_.end = eof_pos;
				if (current_table
					&& current_table != &root
					&& current_table->source_.end <= current_table->source_.begin)
					current_table->source_.end = eof_pos;
			}

			static void update_region_ends(node& nde) noexcept
			{
				const auto type = nde.type();
				if (type > node_type::array)
					return;

				if (type == node_type::table)
				{
					auto& tbl = nde.ref_cast<table>();
					if (tbl.inline_) //inline tables (and all their inline descendants) are already correctly terminated
						return;

					auto end = nde.source_.end;
					for (auto& [k, v] : tbl.map)
					{
						(void)k;
						update_region_ends(*v);
						if (end < v->source_.end)
							end = v->source_.end;
					}
				}
				else //arrays
				{
					auto& arr = nde.ref_cast<array>();
					auto end = nde.source_.end;
					for (auto& v : arr.elements)
					{
						update_region_ends(*v);
						if (end < v->source_.end)
							end = v->source_.end;
					}
					nde.source_.end = end;
				}
			}

		public:

			parser(utf8_reader_interface&& reader_) TOML_MAY_THROW
				: reader{ reader_ }
			{
				root.source_ = { prev_pos, prev_pos, reader.source_path() };

				if (!reader.peek_eof())
				{
					cp = reader.read_next();

					#if !TOML_EXCEPTIONS
					if (reader.error())
					{
						err = std::move(reader.error());
						return;
					}
					#endif

					if (cp)
						parse_document();
				}

				update_region_ends(root);
			}

			TOML_PUSH_WARNINGS
			TOML_DISABLE_INIT_WARNINGS

			[[nodiscard]]
			operator parse_result() && noexcept
			{
				#if TOML_EXCEPTIONS

				return { std::move(root) };

				#else

				if (err)
					return parse_result{ *std::move(err) };
				else
					return parse_result{ std::move(root) };

				#endif

			}

			TOML_POP_WARNINGS
	};

	TOML_EXTERNAL_LINKAGE
	toml::array* parser::parse_array() TOML_MAY_THROW
	{
		return_if_error({});
		assert_not_eof();
		assert_or_assume(*cp == U'[');
		push_parse_scope("array"sv);

		// skip opening '['
		advance_and_return_if_error_or_eof({});

		auto arr = new array{};
		auto& vals = arr->elements;
		enum parse_elem : int
		{
			none,
			comma,
			val
		};
		parse_elem prev = none;

		while (!is_error())
		{
			while (consume_leading_whitespace()
				|| consume_line_break()
				|| consume_comment())
				continue;
			set_error_and_return_if_eof({});

			// commas - only legal after a value
			if (*cp == U',')
			{
				if (prev == val)
				{
					prev = comma;
					advance_and_return_if_error_or_eof({});
					continue;
				}
				set_error_and_return_default("expected value or closing ']', saw comma"sv);
			}

			// closing ']'
			else if (*cp == U']')
			{
				advance_and_return_if_error({});
				break;
			}

			// must be a value
			else
			{
				if (prev == val)
				{
					set_error_and_return_default("expected comma or closing ']', saw '"sv, to_sv(*cp), "'"sv);
					continue;
				}
				prev = val;
				vals.emplace_back(parse_value());
			}
		}

		return_if_error({});
		return arr;
	}

	TOML_EXTERNAL_LINKAGE
	toml::table* parser::parse_inline_table() TOML_MAY_THROW
	{
		return_if_error({});
		assert_not_eof();
		assert_or_assume(*cp == U'{');
		push_parse_scope("inline table"sv);

		// skip opening '{'
		advance_and_return_if_error_or_eof({});

		auto tab = new table{};
		tab->inline_ = true;
		enum parse_elem : int
		{
			none,
			comma,
			kvp
		};
		parse_elem prev = none;

		while (!is_error())
		{
			if constexpr (TOML_LANG_UNRELEASED) // toml/issues/516 (newlines/trailing commas in inline tables)
			{
				while (consume_leading_whitespace()
					|| consume_line_break()
					|| consume_comment())
					continue;
			}
			else
			{
				while (consume_leading_whitespace())
					continue;
			}
			set_error_and_return_if_eof({});

			// commas - only legal after a key-value pair
			if (*cp == U',')
			{
				if (prev == kvp)
				{
					prev = comma;
					advance_and_return_if_error_or_eof({});
				}
				else
					set_error_and_return_default("expected key-value pair or closing '}', saw comma"sv);
			}

			// closing '}'
			else if (*cp == U'}')
			{
				if constexpr (!TOML_LANG_UNRELEASED) // toml/issues/516 (newlines/trailing commas in inline tables)
				{
					if (prev == comma)
					{
						set_error_and_return_default("expected key-value pair, saw closing '}' (dangling comma)"sv);
						continue;
					}
				}
				advance_and_return_if_error({});
				break;
			}

			// key-value pair
			else if (is_string_delimiter(*cp) || is_bare_key_character(*cp))
			{
				if (prev == kvp)
					set_error_and_return_default("expected comma or closing '}', saw '"sv, to_sv(*cp), "'"sv);
				else
				{
					prev = kvp;
					parse_key_value_pair_and_insert(tab);
				}
			}

			else
				set_error_and_return_default("expected key or closing '}', saw '"sv, to_sv(*cp), "'"sv);
		}

		return_if_error({});
		return tab;
	}

	TOML_API
	TOML_EXTERNAL_LINKAGE
	parse_result do_parse(utf8_reader_interface&& reader) TOML_MAY_THROW
	{
		return impl::parser{ std::move(reader) };
	}

	TOML_ABI_NAMESPACE_END // TOML_EXCEPTIONS

	#undef push_parse_scope_2
	#undef push_parse_scope_1
	#undef push_parse_scope
	#undef TOML_RETURNS_BY_THROWING
	#undef is_eof
	#undef assert_not_eof
	#undef return_if_eof
	#undef is_error
	#undef return_after_error
	#undef assert_not_error
	#undef return_if_error
	#undef return_if_error_or_eof
	#undef set_error_and_return
	#undef set_error_and_return_default
	#undef set_error_and_return_if_eof
	#undef advance_and_return_if_error
	#undef advance_and_return_if_error_or_eof
	#undef assert_or_assume
}
TOML_IMPL_NAMESPACE_END

TOML_NAMESPACE_START
{
	TOML_ABI_NAMESPACE_BOOL(TOML_EXCEPTIONS, ex, noex)

	TOML_API
	TOML_EXTERNAL_LINKAGE
	parse_result parse(std::string_view doc, std::string_view source_path) TOML_MAY_THROW
	{
		return impl::do_parse(impl::utf8_reader{ doc, source_path });
	}

	TOML_API
	TOML_EXTERNAL_LINKAGE
	parse_result parse(std::string_view doc, std::string&& source_path) TOML_MAY_THROW
	{
		return impl::do_parse(impl::utf8_reader{ doc, std::move(source_path) });
	}

	#if TOML_WINDOWS_COMPAT

	TOML_API
	TOML_EXTERNAL_LINKAGE
	parse_result parse(std::string_view doc, std::wstring_view source_path) TOML_MAY_THROW
	{
		return impl::do_parse(impl::utf8_reader{ doc, impl::narrow(source_path) });
	}

	#endif // TOML_WINDOWS_COMPAT

	#ifdef __cpp_lib_char8_t

	TOML_API
	TOML_EXTERNAL_LINKAGE
	parse_result parse(std::u8string_view doc, std::string_view source_path) TOML_MAY_THROW
	{
		return impl::do_parse(impl::utf8_reader{ doc, source_path });
	}

	TOML_API
	TOML_EXTERNAL_LINKAGE
	parse_result parse(std::u8string_view doc, std::string&& source_path) TOML_MAY_THROW
	{
		return impl::do_parse(impl::utf8_reader{ doc, std::move(source_path) });
	}

	#if TOML_WINDOWS_COMPAT

	TOML_API
	TOML_EXTERNAL_LINKAGE
	parse_result parse(std::u8string_view doc, std::wstring_view source_path) TOML_MAY_THROW
	{
		return impl::do_parse(impl::utf8_reader{ doc, impl::narrow(source_path) });
	}

	#endif // TOML_WINDOWS_COMPAT

	#endif // __cpp_lib_char8_t

	inline namespace literals
	{
		TOML_API
		TOML_EXTERNAL_LINKAGE
		parse_result operator"" _toml(const char* str, size_t len) TOML_MAY_THROW
		{
			return parse(std::string_view{ str, len });
		}

		#ifdef __cpp_lib_char8_t

		TOML_API
		TOML_EXTERNAL_LINKAGE
		parse_result operator"" _toml(const char8_t* str, size_t len) TOML_MAY_THROW
		{
			return parse(std::u8string_view{ str, len });
		}

		#endif // __cpp_lib_char8_t
	}

	TOML_ABI_NAMESPACE_END // TOML_EXCEPTIONS
}
TOML_NAMESPACE_END

TOML_POP_WARNINGS // TOML_DISABLE_SWITCH_WARNINGS

#endif //----------------------------------------------------------  ↑ toml_parser.hpp  --------------------------------

#endif // TOML_PARSER

#if !TOML_HEADER_ONLY

#if 1  //-------------------------------------------------------------------------------  ↓ toml_instantiations.hpp  ---

TOML_DISABLE_WARNINGS
#include <ostream>
#include <istream>
#include <fstream>
TOML_ENABLE_WARNINGS

#if TOML_PARSER

#endif

// internal implementation namespace
TOML_IMPL_NAMESPACE_START
{
	// formatters
	template class TOML_API formatter<char>;

	// print to stream machinery
	template TOML_API void print_floating_point_to_stream(double, std::ostream&, bool);
}
TOML_IMPL_NAMESPACE_END

// public namespace
TOML_NAMESPACE_START
{
	// value<>
	template class TOML_API value<std::string>;
	template class TOML_API value<int64_t>;
	template class TOML_API value<double>;
	template class TOML_API value<bool>;
	template class TOML_API value<date>;
	template class TOML_API value<time>;
	template class TOML_API value<date_time>;

	// node_view
	template class TOML_API node_view<node>;
	template class TOML_API node_view<const node>;

	// formatters
	template class TOML_API default_formatter<char>;
	template class TOML_API json_formatter<char>;

	// various ostream operators
	template TOML_API std::ostream& operator << (std::ostream&, const source_position&);
	template TOML_API std::ostream& operator << (std::ostream&, const source_region&);
	template TOML_API std::ostream& operator << (std::ostream&, const date&);
	template TOML_API std::ostream& operator << (std::ostream&, const time&);
	template TOML_API std::ostream& operator << (std::ostream&, const time_offset&);
	template TOML_API std::ostream& operator << (std::ostream&, const date_time&);
	template TOML_API std::ostream& operator << (std::ostream&, const value<std::string>&);
	template TOML_API std::ostream& operator << (std::ostream&, const value<int64_t>&);
	template TOML_API std::ostream& operator << (std::ostream&, const value<double>&);
	template TOML_API std::ostream& operator << (std::ostream&, const value<bool>&);
	template TOML_API std::ostream& operator << (std::ostream&, const value<toml::date>&);
	template TOML_API std::ostream& operator << (std::ostream&, const value<toml::time>&);
	template TOML_API std::ostream& operator << (std::ostream&, const value<toml::date_time>&);
	template TOML_API std::ostream& operator << (std::ostream&, default_formatter<char>&);
	template TOML_API std::ostream& operator << (std::ostream&, default_formatter<char>&&);
	template TOML_API std::ostream& operator << (std::ostream&, json_formatter<char>&);
	template TOML_API std::ostream& operator << (std::ostream&, json_formatter<char>&&);
	template TOML_API std::ostream& operator << (std::ostream&, const table&);
	template TOML_API std::ostream& operator << (std::ostream&, const array&);
	template TOML_API std::ostream& operator << (std::ostream&, const node_view<node>&);
	template TOML_API std::ostream& operator << (std::ostream&, const node_view<const node>&);
	template TOML_API std::ostream& operator << (std::ostream&, node_type);

	// node::value, node_view:::value etc
	#define TOML_INSTANTIATE(name, T)														\
		template TOML_API optional<T>		node::name<T>() const noexcept;					\
		template TOML_API optional<T>		node_view<node>::name<T>() const noexcept;		\
		template TOML_API optional<T>		node_view<const node>::name<T>() const noexcept
	TOML_INSTANTIATE(value_exact, std::string_view);
	TOML_INSTANTIATE(value_exact, std::string);
	TOML_INSTANTIATE(value_exact, const char*);
	TOML_INSTANTIATE(value_exact, int64_t);
	TOML_INSTANTIATE(value_exact, double);
	TOML_INSTANTIATE(value_exact, date);
	TOML_INSTANTIATE(value_exact, time);
	TOML_INSTANTIATE(value_exact, date_time);
	TOML_INSTANTIATE(value_exact, bool);
	TOML_INSTANTIATE(value, std::string_view);
	TOML_INSTANTIATE(value, std::string);
	TOML_INSTANTIATE(value, const char*);
	TOML_INSTANTIATE(value, signed char);
	TOML_INSTANTIATE(value, signed short);
	TOML_INSTANTIATE(value, signed int);
	TOML_INSTANTIATE(value, signed long);
	TOML_INSTANTIATE(value, signed long long);
	TOML_INSTANTIATE(value, unsigned char);
	TOML_INSTANTIATE(value, unsigned short);
	TOML_INSTANTIATE(value, unsigned int);
	TOML_INSTANTIATE(value, unsigned long);
	TOML_INSTANTIATE(value, unsigned long long);
	TOML_INSTANTIATE(value, double);
	TOML_INSTANTIATE(value, float);
	TOML_INSTANTIATE(value, date);
	TOML_INSTANTIATE(value, time);
	TOML_INSTANTIATE(value, date_time);
	TOML_INSTANTIATE(value, bool);
	#ifdef __cpp_lib_char8_t
	TOML_INSTANTIATE(value_exact, std::u8string_view);
	TOML_INSTANTIATE(value_exact, std::u8string);
	TOML_INSTANTIATE(value_exact, const char8_t*);
	TOML_INSTANTIATE(value, std::u8string_view);
	TOML_INSTANTIATE(value, std::u8string);
	TOML_INSTANTIATE(value, const char8_t*);
	#endif
	#if TOML_WINDOWS_COMPAT
	TOML_INSTANTIATE(value_exact, std::wstring);
	TOML_INSTANTIATE(value, std::wstring);
	#endif
	#undef TOML_INSTANTIATE

	// parser instantiations
	#if TOML_PARSER

		// parse error ostream
		template TOML_API std::ostream& operator << (std::ostream&, const parse_error&);

		// parse() and parse_file()
		TOML_ABI_NAMESPACE_BOOL(TOML_EXCEPTIONS, ex, noex)

		template TOML_API parse_result parse(std::istream&, std::string_view) TOML_MAY_THROW;
		template TOML_API parse_result parse(std::istream&, std::string&&) TOML_MAY_THROW;
		template TOML_API parse_result parse_file(std::string_view) TOML_MAY_THROW;
		#ifdef __cpp_lib_char8_t
			template TOML_API parse_result parse_file(std::u8string_view) TOML_MAY_THROW;
		#endif
		#if TOML_WINDOWS_COMPAT
			template TOML_API parse_result parse_file(std::wstring_view) TOML_MAY_THROW;
		#endif

		TOML_ABI_NAMESPACE_END // TOML_EXCEPTIONS

	#endif // TOML_PARSER
}
TOML_NAMESPACE_END

#endif //-------------------------------------------------------------------------------  ↑ toml_instantiations.hpp  ---

#endif // !TOML_HEADER_ONLY

#endif // TOML_IMPLEMENTATION

TOML_POP_WARNINGS // TOML_DISABLE_SPAM_WARNINGS

// macro hygiene
#if TOML_UNDEF_MACROS
	#undef TOML_ABI_NAMESPACES
	#undef TOML_ABI_NAMESPACE_BOOL
	#undef TOML_ABI_NAMESPACE_END
	#undef TOML_ABI_NAMESPACE_START
	#undef TOML_ALWAYS_INLINE
	#undef TOML_ANON_NAMESPACE
	#undef TOML_ANON_NAMESPACE_END
	#undef TOML_ANON_NAMESPACE_START
	#undef TOML_ARM
	#undef TOML_ASSERT
	#undef TOML_ASSUME
	#undef TOML_ASYMMETRICAL_EQUALITY_OPS
	#undef TOML_ATTR
	#undef TOML_CLANG
	#undef TOML_COMPILER_EXCEPTIONS
	#undef TOML_CONCAT
	#undef TOML_CONCAT_1
	#undef TOML_CONSTEVAL
	#undef TOML_CPP
	#undef TOML_DISABLE_ARITHMETIC_WARNINGS
	#undef TOML_DISABLE_INIT_WARNINGS
	#undef TOML_DISABLE_SPAM_WARNINGS
	#undef TOML_DISABLE_SHADOW_WARNINGS
	#undef TOML_DISABLE_SUGGEST_WARNINGS
	#undef TOML_DISABLE_SWITCH_WARNINGS
	#undef TOML_DISABLE_WARNINGS
	#undef TOML_ENABLE_WARNINGS
	#undef TOML_EMPTY_BASES
	#undef TOML_EVAL_BOOL_0
	#undef TOML_EVAL_BOOL_1
	#undef TOML_EXTERNAL_LINKAGE
	#undef TOML_FLOAT128
	#undef TOML_FLOAT16
	#undef TOML_FLOAT_CHARCONV
	#undef TOML_FP16
	#undef TOML_GCC
	#undef TOML_HAS_ATTR
	#undef TOML_HAS_CUSTOM_OPTIONAL_TYPE
	#undef TOML_HAS_INCLUDE
	#undef TOML_ICC
	#undef TOML_ICC_CL
	#undef TOML_IMPLEMENTATION
	#undef TOML_IMPL_NAMESPACE_END
	#undef TOML_IMPL_NAMESPACE_START
	#undef TOML_INT128
	#undef TOML_INTELLISENSE
	#undef TOML_INTERFACE
	#undef TOML_INTERNAL_LINKAGE
	#undef TOML_INT_CHARCONV
	#undef TOML_LANG_AT_LEAST
	#undef TOML_LANG_EFFECTIVE_VERSION
	#undef TOML_LANG_HIGHER_THAN
	#undef TOML_LANG_UNRELEASED
	#undef TOML_LAUNDER
	#undef TOML_LIKELY
	#undef TOML_MAKE_BITOPS
	#undef TOML_MAKE_VERSION
	#undef TOML_MAY_THROW
	#undef TOML_MSVC
	#undef TOML_NAMESPACE
	#undef TOML_NAMESPACE_END
	#undef TOML_NAMESPACE_START
	#undef TOML_NEVER_INLINE
	#undef TOML_NODISCARD_CTOR
	#undef TOML_NO_DEFAULT_CASE
	#undef TOML_PARSER_TYPENAME
	#undef TOML_POP_WARNINGS
	#undef TOML_PUSH_WARNINGS
	#undef TOML_RELOPS_REORDERING
	#undef TOML_SA_LIST_BEG
	#undef TOML_SA_LIST_END
	#undef TOML_SA_LIST_NEW
	#undef TOML_SA_LIST_NXT
	#undef TOML_SA_LIST_SEP
	#undef TOML_SA_NATIVE_VALUE_TYPE_LIST
	#undef TOML_SA_NEWLINE
	#undef TOML_SA_NODE_TYPE_LIST
	#undef TOML_SA_UNWRAPPED_NODE_TYPE_LIST
	#undef TOML_SA_VALUE_EXACT_FUNC_MESSAGE
	#undef TOML_SA_VALUE_FUNC_MESSAGE
	#undef TOML_SA_VALUE_MESSAGE_CONST_CHAR8
	#undef TOML_SA_VALUE_MESSAGE_U8STRING_VIEW
	#undef TOML_SA_VALUE_MESSAGE_WSTRING
	#undef TOML_SIMPLE_STATIC_ASSERT_MESSAGES
	#undef TOML_TRIVIAL_ABI
	#undef TOML_UINT128
	#undef TOML_UNLIKELY
	#undef TOML_UNREACHABLE
	#undef TOML_USING_ANON_NAMESPACE
#endif

#endif // INCLUDE_TOMLPLUSPLUS_H
