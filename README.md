# CAFBERIHT: Constexpr Aggregated and Filtered Bases for Efficient Runtime Iteration of Heterogeneous Types

**Production patterns from Nihilus**

> ⚠️ **Important**: These libraries are used internally at Nihilai Collective,
> but are **not representative** of the totality of our inference architecture,
> execution model, or deployment techniques.

## What This Is
* General-purpose C++20/23 patterns for heterogeneous type collections
* Zero-overhead abstractions used in our production systems
* Examples of modern template metaprogramming techniques

## What This Is NOT
* Our LLM inference engine (that's proprietary)
* Our hardware optimization layers (patent pending)
* Our deployment infrastructure (commercial product)

## Why We're Open Sourcing This
We believe in advancing C++ systems programming. These patterns have served us well, and we want to share them with the community while focusing on our core product: **the Nihilus LLM inference ecosystem**.

## Build-Time Efficiency & Scalability

A core advantage of the CAFBERIHT pattern is its ability to scale to thousands of heterogeneous types without introducing runtime dispatch overhead. To observe how the pattern shifts the computational burden from **Runtime Execution** to **Compile-Time Orchestration**, we encourage users to stress-test the architectural limits of their toolchain.

### Stress-Testing with `CAFBERIHT_WIDTH`

You can verify the stability and build-time benefits of this pattern by adjusting the inheritance depth during configuration. Increasing the width demonstrates the compiler's ability to "fuse" thousands of unique base classes into a linear instruction stream.

**Test the build times by setting the width via CMake:**

```bash
# Example: Stress-testing with 10,000 unique base classes
cmake -B build -DCAFBERIHT_WIDTH=10000
time cmake --build build --config Release

```

**What to look for:**

* **The "Compile-Time Tax"**: At 10,000 width, modern compilers (MSVC/Clang) may take several minutes to resolve the symbol tables and perform COMDAT folding. This is a one-time cost paid during the build.
* **The "Runtime Dividend"**: Despite the massive compile-time complexity, notice the final executable remains remarkably small (typically under **300 KB**). This proves that the abstraction has been entirely erased, leaving only the raw, linear logic required for your hot loop.

## Getting Started

[Installation](Installation.md)

[Compilation Times](Compilation_Times.md)

[Whitepaper](Whitepaper.md)

## Learn More About Nihilus

* [Website]()
* [Try our inference demo](coming soon)
* [Enterprise licensing inquiries](contact us)
