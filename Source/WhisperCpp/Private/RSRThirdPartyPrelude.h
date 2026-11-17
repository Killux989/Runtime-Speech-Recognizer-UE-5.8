// Georgy Treshchev 2024.
// Shared prelude for the wrapper translation units that compile upstream ggml / whisper.cpp.
// Must stay valid C and C++. Each wrapper compiles exactly one upstream file so their static symbols never collide.

#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(push, 0)
#pragma warning(disable : 4005 4100 4189 4244 4267 4305 4456 4457 4458 4459 4464 4702 4706 4996 5054)
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Weverything"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#define RSR_ARCH_X86 1
#else
#define RSR_ARCH_X86 0
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define RSR_ARCH_ARM64 1
#else
#define RSR_ARCH_ARM64 0
#endif
