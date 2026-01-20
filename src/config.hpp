/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Nihilai Collective Corp
 * https://github.com/nihilai-collective/cafberiht
 * src/config.hpp
 */
#pragma once

#include <iostream>
#include <cstdint>
#include <limits>
#include <array>

#if defined(_MSC_VER)
	#define CAFBERIHT_INLINE [[msvc::forceinline]]
#elif defined(__GNUC__) || defined(__clang__)
	#define CAFBERIHT_INLINE inline __attribute__((always_inline))
#else
	#define CAFBERIHT_INLINE inline
#endif

static constexpr uint64_t cafberiht_width{ CAFBERIHT_WIDTH };