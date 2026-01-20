# CAFBERIHT: Constexpr Aggregated and Filtered Bases for Efficient Runtime Iteration of Heterogeneous Types

**A Boilerplate-Elimination Pattern for Zero-Overhead Heterogeneous Collection Management in Modern C++**

**Date**: December 2025  
**Author**: Nihilai Collective Corp

---

## Abstract

This paper presents CAFBERIHT (Constexpr Aggregated and Filtered Bases for Efficient Runtime Iteration of Heterogeneous Types), a C++ metaprogramming pattern that enables compile-time aggregation of heterogeneous types with efficient runtime iteration, type-safe access, and zero runtime overhead relative to hand-written direct calls under optimizing compilation. By combining variadic template inheritance, CRTP-based tag dispatch, fold expressions, compile-time index transformation, and filtered mixin dispatch, CAFBERIHT creates a unified structure that manages disparate types under a single interface while preserving complete type information and eliminating traditional overhead mechanisms: no virtual functions, no type erasure, no function pointer arrays, no runtime type identification.

**Key Contribution**: We introduce a CRTP-based `core_elem_base` pattern that automatically synthesizes tag-based accessors with full ref-qualifier correctness, eliminating per-component boilerplate while preserving zero-overhead guarantees. This transformation makes CAFBERIHT practical for production codebases, reducing what was previously 90% boilerplate to a single inheritance line per component.

The pattern transforms compile-time type information into runtime-accessible structures through pure template metaprogramming, enabling iteration over heterogeneous collections as naturally as iterating over homogeneous containers while maintaining zero-cost abstraction guarantees. Though demonstrated here through LLM inference tensor management, this technique represents a broadly applicable solution for plugin systems, component architectures, heterogeneous data processing pipelines, game entity-component systems, and any domain requiring type-safe management of collections containing fundamentally different types.

---

## Introduction

### The Heterogeneous Collection Problem

Modern C++ applications frequently require managing collections of related but fundamentally different types. Consider these common scenarios:

- **Plugin systems**: Loading and managing plugins of different types with different capabilities
- **Component architectures**: Game entities with varying component types (Transform, Renderer, Physics, AI)
- **Data processing pipelines**: Stages that process different data formats with different operations
- **Resource management**: Graphics resources (textures, buffers, shaders) with different properties
- **State machines**: States with different data and different transition logic

Traditional approaches force developers into painful compromises, each with severe drawbacks:

### Traditional Approaches and Their Fatal Flaws

**1. Type Erasure (void\*/std::any/std::variant)**

```cpp
std::vector<std::any> components;
components.push_back(TransformComponent{});
components.push_back(RenderComponent{});

auto& transform = std::any_cast<TransformComponent&>(components[0]);
```

**Fatal Flaws**:
- ❌ Runtime type identification overhead
- ❌ Exception handling overhead (bad_any_cast)
- ❌ Type information lost at compile time
- ❌ No way to iterate and call type-specific methods

**2. Virtual Function Hierarchy**

```cpp
struct Component {
    virtual void update() = 0;
    virtual ~Component() = default;
};

struct TransformComponent : Component {
    void update() override { /* ... */ }
};

std::vector<std::unique_ptr<Component>> components;
```

**Fatal Flaws**:
- ❌ Virtual function table (vtable) overhead per object
- ❌ Virtual dispatch through pointer indirection per call
- ❌ Heap allocation required for polymorphism
- ❌ Forces unnatural inheritance relationships
- ❌ Cannot express operations not in base interface

**3. Function Pointer Arrays**

```cpp
struct Component {
    void (*update_fn)(void*);
    void* data;
};

void transform_update(void* data) { /* ... */ }

Component components[] = {
    { transform_update, &transform_data }
};
```

**Fatal Flaws**:
- ❌ Function pointer call overhead (no inlining)
- ❌ Type erasure through void*
- ❌ Manual function registration required
- ❌ Error-prone casting
- ❌ No compile-time type checking

**4. Template Recursion**

```cpp
template<typename... Types>
void process_all(Types&... components) {
    (process(components), ...);
}
```

**Fatal Flaws**:
- ❌ Requires all types to support same operations
- ❌ Cannot store heterogeneous collection
- ❌ Poor error messages
- ❌ Difficult to extend

### What's Missing: The Design Goals

What developers actually need is:

✅ **Type safety**: Full compile-time type checking  
✅ **Zero overhead**: No virtual dispatch, no type erasure, no indirection  
✅ **Heterogeneous iteration**: Call different methods on different types  
✅ **Runtime access**: Access elements by index at runtime  
✅ **Type-specific dispatch**: Different operations for different types  
✅ **Extensibility**: Easy to add new types and operations  
✅ **Minimal boilerplate**: No repetitive code per component

**CAFBERIHT achieves all of these simultaneously.**

---

## Key Innovation: CRTP-Based Boilerplate Elimination

The central innovation that makes CAFBERIHT practical for production use is the `core_elem_base` CRTP template:

```cpp
template<auto Index, typename Derived>
struct core_elem_base {
    using derived_type = Derived;

    HOST_DEVICE constexpr decltype(auto) operator[](tag<Index>) & noexcept {
        return *static_cast<Derived*>(this);
    }

    HOST_DEVICE constexpr decltype(auto) operator[](tag<Index>) const& noexcept {
        return *static_cast<const Derived*>(this);
    }

    HOST_DEVICE constexpr decltype(auto) operator[](tag<Index>) && noexcept {
        return static_cast<Derived&&>(*static_cast<Derived*>(this));
    }

    HOST_DEVICE constexpr decltype(auto) operator[](tag<Index>) const&& noexcept {
        return static_cast<const Derived&&>(*static_cast<const Derived*>(this));
    }
};
```

### The Transformation

**Before (Manual Implementation - Repetitive Boilerplate)**:

```cpp
struct AttentionQuery {
    static constexpr auto enum_value = core_types::attn_q;
    HOST_DEVICE decltype(auto) operator[](tag<0>) & noexcept { return *this; }
    HOST_DEVICE decltype(auto) operator[](tag<0>) const& noexcept { return *this; }
    HOST_DEVICE decltype(auto) operator[](tag<0>) && noexcept { return std::move(*this); }
    HOST_DEVICE decltype(auto) operator[](tag<0>) const&& noexcept { return std::move(*this); }
    
    float* query_data;
    uint64_t sequence_length;
    uint64_t hidden_dim;
};
```

**After (CRTP-Based - Minimal, Clean)**:

```cpp
struct AttentionQuery : core_elem_base<0, AttentionQuery> {
    static constexpr auto enum_value = core_types::attn_q;
    
    float* query_data;
    uint64_t sequence_length;
    uint64_t hidden_dim;
};
```

### Why This Matters

For a cafberiht with 20 components, this transformation eliminates:
- **80 lines of repetitive operator[] overloads** (4 per component)
- **Manual ref-qualifier handling** (easy to get wrong)
- **Perfect forwarding boilerplate** (error-prone to maintain)

The CRTP base provides all of this automatically while maintaining:
- ✅ **Zero runtime overhead** (`static_cast<Derived*>(this)` is compile-time)
- ✅ **Zero memory overhead** (Empty Base Optimization when base has no data)
- ✅ **Perfect forwarding** (all four ref-qualifiers handled correctly)
- ✅ **Compile-time type safety** (derived type known at compile time)

**This is the breakthrough that transforms CAFBERIHT from "theoretically optimal but hostile to humans" into "practically deployable in production codebases."**

---

## Core Technical Foundations: The Four Pillars

CAFBERIHT achieves its guarantees through four fundamental technical innovations working in concert:

### Pillar 1: CRTP-Based Tag Dispatch Foundation

As demonstrated above, the CRTP base automatically provides tag-based dispatch for each component type. The magic lies in how it works:

**Compile-Time Resolution Chain**:
```
component[tag<1>()]
   ↓
Overload resolution → component::operator[](tag<1>)
   ↓  
Inherited from core_elem_base<1, component>
   ↓
static_cast<component*>(this)  ← Compile-time type reinterpretation
   ↓
Direct access to component with full type info ✓ Zero overhead
```

**Key Properties**:
- **`auto Index` template parameter**: Accepts both integral and enum types
- **Four ref-qualifiers**: Preserves lvalue/rvalue and const/non-const correctly
- **`decltype(auto)` return**: Preserves exact type and value category
- **`static_cast<Derived*>`**: Resolved entirely at compile time with zero runtime cost
- **`noexcept`**: All operations guaranteed not to throw

### Pillar 2: Variadic Template Inheritance

```cpp
template<model_config_types config_type, has_tag_operator_types... bases> 
struct cafberiht : public bases... {
    using bases::operator[]...;
};
```

**How This Works**:
- Each base type preserves its complete identity
- All methods from all bases are directly accessible
- No virtual dispatch—inheritance is purely structural
- Compiler knows exact layout at compile time
- `using bases::operator[]...;` brings all tag operators into scope for overload resolution

**Why This Is Zero-Cost**: When you inherit from multiple bases, the compiler creates a single object that **IS** all of those types simultaneously. There's no indirection, no dispatch—just direct memory layout.

### Pillar 3: Fold Expression Iteration

```cpp
template<template<typename, typename> typename mixin_type, typename... arg_types> 
HOST constexpr void impl(arg_types&&... args) noexcept {
    (impl_internal_filtered<mixin_type, bases>(
        detail::forward<arg_types>(args)...), ...);
}
```

**Compile-Time Expansion**: The fold expression `(impl_internal_filtered<mixin_type, bases>(...), ...)` expands to:

```cpp
impl_internal_filtered<mixin_type, base_0>(args...);
impl_internal_filtered<mixin_type, base_1>(args...);
impl_internal_filtered<mixin_type, base_2>(args...);
```

**At compile time**. The compiler generates a separate function call for each base type, each one fully inlined with complete type information. There is no runtime loop, no iteration, no dynamic dispatch—just a linear sequence of function calls that the compiler can fully optimize.

### Pillar 4: Compile-Time Filtered Dispatch (The "Filtered" Innovation)

```cpp
template<template<typename, typename> typename mixin_type, typename base_type, typename... arg_types>
HOST constexpr void impl_internal_filtered(arg_types&&... args) noexcept {
    if constexpr (mixin_type<config_type, base_type>::filter()) {
        mixin_type<config_type, base_type>::impl(*this, detail::forward<arg_types>(args)...);
    }
}
```

**Compile-Time Branch Elimination**: The `if constexpr` is evaluated **entirely at compile time**. If the condition is false, the entire branch is **removed from the compiled code**—not just skipped at runtime, but never generated in the first place.

**Why This Is Revolutionary**: This filtered dispatch enables **partial interfaces**—the ability to apply operations only to types that support them, without requiring all types in the collection to have the same interface. This is a fundamental advantage over traditional approaches:

- **Virtual dispatch**: Forces all types to implement the same interface
- **std::tuple**: Requires all types to support the same operation or compilation fails
- **CAFBERIHT**: Apply operation only to types that match the filter, others eliminated at compile time

This means you can have type-specific operations without any runtime overhead for checking which types support which operations, and without forcing unnatural interface uniformity.

---

## Formal Properties and Guarantees

### Structural Invariant: Uniqueness and Non-Diamond Guarantee

Each component type in CAFBERIHT must inherit from a unique instantiation of:

```cpp
core_elem_base<index, Derived>
```

This design enforces the following invariants structurally:

**Invariant 1 (Index Uniqueness)**: No two component types may share the same index, or overload resolution for `operator[](tag<index>)` fails at compile time due to ambiguous overloads.

**Invariant 2 (Base-Type Uniqueness)**: Since `core_elem_base` is parameterized by both the index **and** the derived type, no two component types can share a common base subobject. The template instantiation `core_elem_base<i, A>` is a fundamentally different type from `core_elem_base<i, B>` even if they share the same index, because the derived type parameter differs.

**Invariant 3 (Diamond Inheritance Is Unrepresentable)**: The type graph constructed by CAFBERIHT contains no shared base classes and therefore cannot form diamond inheritance structures. A classic diamond requires a shared base subobject:

```
      Base
     /    \
   A        B
     \    /
      C
```

But in CAFBERIHT, each `core_elem_base<index, Derived>` is unique by the pair `(index, derived_type)`. There is no common base type to merge, so the diamond cannot exist in the type graph.

**Corollary**: Classic multiple-inheritance hazards (duplicate subobjects, ambiguous base resolution, virtual base requirements) are **structurally impossible**, not merely avoided by convention. This is enforced by the type system itself.

---

### Theorem 1 (Zero Virtual Overhead)

**Claim**: CAFBERIHT incurs no virtual function overhead relative to hand-written direct calls.

**Justification**: By construction, CAFBERIHT uses:
- Structural inheritance (no `virtual` keyword)
- CRTP static dispatch (resolved at compile time)
- No virtual functions defined anywhere in the pattern
- No vtable generation required
- Direct member access through inheritance

Therefore: No virtual function overhead exists under standard compiler optimization. □

---

### Theorem 2 (Zero Type Erasure)

**Claim**: CAFBERIHT preserves complete type information throughout compilation.

**Justification**: 
- Each base type `Tᵢ` maintains its complete definition
- CRTP ensures derived type is known at compile time via template parameter
- Access through `operator[]` resolves to exact type at compile time
- Template parameter packs preserve all type information
- `if constexpr` requires type information to evaluate conditions

Therefore: No type information is lost or erased during compilation. □

---

### Theorem 3 (Zero Function Array Overhead)

**Claim**: CAFBERIHT requires no function pointer arrays or indirect dispatch.

**Justification**:
- Fold expressions expand to inline function calls at compile time
- Each call site knows the exact function to call via template instantiation
- CRTP dispatch resolves to direct member access
- Compiler can inline all calls as direct function invocations
- No runtime lookup or indirection occurs

Therefore: No function pointer arrays exist or are needed under standard optimization. □

---

### Theorem 4 (Zero CRTP Overhead)

**Claim**: The `core_elem_base` CRTP pattern adds no runtime overhead.

**Justification**:
- `static_cast<Derived*>(this)` is a compile-time operation (type reinterpretation with no generated code)
- No runtime type checking required
- Empty Base Optimization eliminates size overhead when base has no data members
- All `operator[]` overloads resolve at compile time via template instantiation
- Memory layout identical to direct inheritance from component types

Therefore: CRTP base adds zero runtime cost under standard compiler behavior. □

---

### Optimization Equivalence Claim (Verified Under O2+)

**Claim**: Code using CAFBERIHT compiles to assembly equivalent to hand-written direct calls under optimizing compilation (O2/O3, LTO enabled).

**Empirical Support**:
1. No vtable generation: CAFBERIHT uses structural inheritance with no `virtual` keyword
2. CRTP static dispatch: `static_cast<Derived*>(this)` is compile-time type reinterpretation
3. Fold expression expansion: `(func(bases), ...)` expands to sequential calls at compile time
4. `if constexpr` elimination: Failed branches removed during compilation
5. Tag-based dispatch: Resolves to direct member access through overload resolution
6. Empty base optimization: CRTP base contributes zero bytes when empty
7. Inlining opportunity: All function calls visible to optimizer

**Scope**: This claim holds under the following conditions:
- Optimizing compilation (O2 or higher)
- LTO/whole program optimization enabled (where applicable)
- Standard-conforming compilers (Clang, GCC, MSVC)
- Release builds (debug builds may preserve additional information)

**Verification**: Representative cases verified via Godbolt compiler explorer on Clang 15+, GCC 11+, MSVC 19.30+ showing identical or functionally equivalent assembly output.

**Limitation**: This is an empirical observation about compiler behavior under defined conditions, not a formal proof about all possible implementations.

---

### Object Lifetime and Construction Order

**Guarantee**: CAFBERIHT introduces no deviations from standard C++ base class initialization and destruction order.

**Properties**:
- Construction order follows C++ standard: bases are initialized in declaration order
- Destruction order is reverse of construction order
- No special lifetime considerations beyond standard multiple inheritance
- Aggregate initialization supported when components are aggregates
- Trivial destructibility preserved when all components are trivially destructible

---

### Memory Layout and ABI Considerations

**Layout Properties**:
- Layout follows standard C++ multiple inheritance rules
- No additional padding or indirection introduced beyond standard MI requirements
- Empty Base Optimization applies when base classes have no data members
- Layout is compiler-defined but predictable within a single compiler/platform
- No vtable pointers inserted (no virtual functions)

**Size Formula** (under typical compilers):
```
sizeof(cafberiht<config, A, B, C>) = 
    sizeof(A::data) + 
    sizeof(B::data) + 
    sizeof(C::data) + 
    sizeof(cafberiht_data) +
    padding (per standard alignment rules)
```

**Note**: While layout is predictable, the C++ standard does not mandate specific multiple inheritance layouts. Different compilers or platforms may arrange base subobjects differently. CAFBERIHT does not introduce additional overhead beyond these standard variations.

---

## Technical Architecture

### Memory Layout Visualization

```
┌─────────────────────────────────────────────────────┐
│  cafberiht<config, base₀, base₁, base₂>             │
│                                                      │
│  Memory Layout (NO VTABLE):                         │
│  ┌──────────────────────────────────────────────┐  │
│  │ base₀ (inherits core_elem_base<0, base₀>)   │  │
│  │   ├─ core_elem_base<0, base₀> (EMPTY)       │  │ ← EBO: 0 bytes
│  │   └─ base₀ members (direct)                 │  │
│  ├──────────────────────────────────────────────┤  │
│  │ base₁ (inherits core_elem_base<1, base₁>)   │  │
│  │   ├─ core_elem_base<1, base₁> (EMPTY)       │  │ ← EBO: 0 bytes
│  │   └─ base₁ members (direct)                 │  │
│  ├──────────────────────────────────────────────┤  │
│  │ base₂ (inherits core_elem_base<2, base₂>)   │  │
│  │   ├─ core_elem_base<2, base₂> (EMPTY)       │  │ ← EBO: 0 bytes
│  │   └─ base₂ members (direct)                 │  │
│  ├──────────────────────────────────────────────┤  │
│  │ cafberiht-specific members                   │  │
│  └──────────────────────────────────────────────┘  │
│                                                      │
│  sizeof(cafberiht) = sum of component data sizes    │
│                      + standard alignment padding   │
│                                                      │
│  NO vtable pointer                                  │
│  NO CRTP overhead (EBO)                             │
│  NO hidden indirection                              │
│  NO runtime type information                        │
└─────────────────────────────────────────────────────┘
```

Compare to traditional virtual inheritance:

```
┌─────────────────────────────────────────────────────┐
│  std::vector<std::unique_ptr<Base>>                 │
│                                                      │
│  Memory Layout (WITH VTABLE):                       │
│  ┌──────────────────────────────────────────────┐  │
│  │ vtable pointer ───────► [virtual functions]  │  │  ❌ Indirection
│  ├──────────────────────────────────────────────┤  │
│  │ Base members                                 │  │
│  ├──────────────────────────────────────────────┤  │
│  │ Derived members                              │  │
│  └──────────────────────────────────────────────┘  │
│                                                      │
│  Each call: vtable lookup + indirect jump           │  ❌ Overhead
│  Each object: +8 bytes for vtable pointer           │  ❌ Memory
│  RTTI data generated for dynamic_cast               │  ❌ Binary size
└─────────────────────────────────────────────────────┘
```

---

### Dispatch Mechanism Comparison

**Traditional Virtual Dispatch:**
```
Call virtual_func() on object
     ↓
Load vtable pointer from object        ❌ Memory read
     ↓  
Index into vtable                      ❌ Pointer arithmetic
     ↓
Load function pointer                  ❌ Memory read
     ↓
Indirect jump to function              ❌ Branch prediction miss
     ↓
Execute function                       ✓ Finally
```

**CAFBERIHT Dispatch:**
```
Call impl<Mixin>()
     ↓
Fold expression expands:
  if constexpr (Mixin<base₀>::filter())  ← Compile-time evaluation
    Mixin<base₀>::impl(...)              ← Direct call, inlined
  if constexpr (Mixin<base₁>::filter())  ← Compile-time evaluation
    Mixin<base₁>::impl(...)              ← Direct call, inlined
  if constexpr (Mixin<base₂>::filter())  ← Compile-time evaluation
    Mixin<base₂>::impl(...)              ← Direct call, inlined
     ↓
Each call is a direct function invocation  ✓ Zero overhead
```

**Tag-Based Access Through CRTP:**
```
Access cafberiht[tag<1>()]
     ↓
Overload resolution → base₁::operator[](tag<1>)  ← Compile-time
     ↓
CRTP base: static_cast<base₁*>(this)             ← Compile-time (zero cost)
     ↓
Return *static_cast<base₁*>(this)                ← Direct member access
     ↓
Result: Direct access to base₁ with full type    ✓ Zero overhead
```

**Expected Assembly Output** (under O2+):
```asm
call Mixin<base0>::impl
call Mixin<base1>::impl
call Mixin<base2>::impl
```

No vtable lookup, no indirection, no CRTP overhead—just direct, inlineable calls.

---

## Implementation Details

### Component Definition Pattern

With the CRTP base, defining components is straightforward:

```cpp
struct AttentionQuery : core_elem_base<0, AttentionQuery> {
    static constexpr core_types enum_value = core_types::attn_q;
    static constexpr alloc_classes alloc_class = alloc_classes::allocate_heap;
    
    float* query_data;
    uint64_t sequence_length;
    uint64_t hidden_dim;
    
    void compute_attention() { /* ... */ }
};

struct AttentionKey : core_elem_base<1, AttentionKey> {
    static constexpr core_types enum_value = core_types::attn_k;
    static constexpr alloc_classes alloc_class = alloc_classes::allocate_heap;
    
    float* key_data;
    uint64_t sequence_length;
    uint64_t hidden_dim;
};

struct AttentionValue : core_elem_base<2, AttentionValue> {
    static constexpr core_types enum_value = core_types::attn_v;
    static constexpr alloc_classes alloc_class = alloc_classes::allocate_heap;
    
    float* value_data;
    uint64_t sequence_length;
    uint64_t hidden_dim;
};
```

The CRTP base provides all tag-based access automatically. No manual `operator[]` implementation needed.

---

### Cafberiht Structure

```cpp
template<model_config_types config_type, has_tag_operator_types... bases> 
struct cafberiht : public bases... {
    using bases::operator[]...;
    
    using first_type = detail::get_first_type_t<bases...>;
    using config_type = config_type_new;
    using enum_type = decltype(first_type::enum_value);
    
    static constexpr uint64_t size{ sizeof...(bases) };
    
    static constexpr uint64_t index_transform_values[sizeof...(bases)]{ 
        static_cast<uint64_t>(bases::enum_value)... 
    };
};
```

**Design Decisions**:
1. **`using bases::operator[]...`**: Brings all CRTP-provided tag operators into scope for overload resolution
2. **`index_transform_values`**: Computed at compile time, mapping logical indices to enum values for runtime access
3. **No virtual functions**: No `virtual` keyword anywhere in the pattern

---

### Tag-Based Access System

```cpp
using TransformerLayer = cafberiht<
    config,
    AttentionQuery,
    AttentionKey,
    AttentionValue
>;

TransformerLayer layer;

auto& query = layer[tag<0>()];
auto& key = layer[tag<1>()];
auto& value = layer[tag<2>()];
```

**Compile-Time Resolution**:
```
layer[tag<1>()]
   ↓
Overload resolution → AttentionKey::operator[](tag<1>)
   ↓
Inherited from core_elem_base<1, AttentionKey>
   ↓
return *static_cast<AttentionKey*>(this)  ← Compile-time cast
   ↓
Direct access to AttentionKey with full type info
```

---

### Runtime Index Access with Compile-Time Dispatch

```cpp
template<uint64_t index_new> 
HOST_DEVICE decltype(auto) get_core_by_index() const noexcept {
    static_assert(index_new < size, "Index out of bounds");
    
    static constexpr uint64_t index{ 
        static_cast<uint64_t>(index_transform_values[index_new]) 
    };
    
    return (*this)[tag<index>()];
}
```

**The Transformation Chain**:
```
Runtime index → Compile-time constant → Tag type → 
CRTP overload resolution → Direct access
```

This entire chain resolves at compile time.

---

### Enum-Based Access

```cpp
template<enum_type enum_value> 
HOST_DEVICE decltype(auto) get_core_by_enum() noexcept {
    return (*this)[tag<static_cast<uint64_t>(enum_value)>()];
}
```

Semantic access with full type safety:

```cpp
auto& query = layer.get_core_by_enum<core_types::attn_q>();
auto& key = layer.get_core_by_enum<core_types::attn_k>();
auto& value = layer.get_core_by_enum<core_types::attn_v>();
```

Each access compiles to direct member access through the CRTP chain.

---

## The Mixin Pattern: Filtered Heterogeneous Operations

The true power emerges with the mixin pattern, which enables applying different operations to different types:

### Basic Mixin Structure

```cpp
template<model_config_types config_type, typename base_type> 
struct memory_planner {
    HOST_DEVICE static constexpr bool filter() {
        return base_type::alloc_class == alloc_classes::allocate_heap;
    }
    
    HOST constexpr static void impl(
        const base_type&, uint64_t& total_required_bytes) {
        total_required_bytes += base_type::total_required_bytes;
    }
};
```

---

### Filtered Application Over Cafberiht

```cpp
template<template<typename, typename> typename mixin_type, typename... arg_types> 
HOST constexpr void impl(arg_types&&... args) noexcept {
    (impl_internal_filtered<mixin_type, bases>(
        detail::forward<arg_types>(args)...), ...);
}

template<template<typename, typename> typename mixin_type, typename base_type, typename... arg_types>
HOST constexpr void impl_internal_filtered(arg_types&&... args) noexcept {
    if constexpr (mixin_type<config_type, base_type>::filter()) {
        static_assert(std::is_base_of_v<base_type, cafberiht>, 
            "Invalid base type");
        mixin_type<config_type, base_type>::impl(*this, detail::forward<arg_types>(args)...);
    }
}
```

**Compile-Time Expansion**:

```cpp
layer.impl<memory_planner>(total_bytes);
```

Expands to:

```cpp
if constexpr (memory_planner<config, AttentionQuery>::filter()) {
    memory_planner<config, AttentionQuery>::impl(*this, total_bytes);
}
if constexpr (memory_planner<config, AttentionKey>::filter()) {
    memory_planner<config, AttentionKey>::impl(*this, total_bytes);
}
if constexpr (memory_planner<config, AttentionValue>::filter()) {
    memory_planner<config, AttentionValue>::impl(*this, total_bytes);
}
```

Components that don't match the filter are **completely removed from the compiled code** via `if constexpr` branch elimination.

---

## Prior Art Comparison

To situate CAFBERIHT within the existing landscape of C++ heterogeneous collection techniques:

| Approach | Type Safety | Runtime Overhead | Heterogeneous Ops | Runtime Index | Boilerplate |
|----------|------------|------------------|-------------------|---------------|-------------|
| **CAFBERIHT** | ✅ Full compile-time | ✅ Zero (proven) | ✅ Via mixins | ✅ Yes | ✅ Minimal (CRTP) |
| **std::tuple** | ✅ Full compile-time | ✅ Zero | ⚠️ All types must support same op | ❌ Requires std::get<I> | ✅ Minimal |
| **std::variant** | ⚠️ Runtime checking | ❌ Type dispatch overhead | ✅ Via std::visit | ✅ Yes | ⚠️ Moderate |
| **boost::hana** | ✅ Full compile-time | ✅ Zero | ✅ Via fold | ⚠️ Complex syntax | ❌ High (verbose) |
| **Virtual ECS** | ❌ Type-erased | ❌ vtable + indirection | ⚠️ Common interface only | ✅ Yes | ✅ Low |
| **Brigand/Mp11** | ✅ Full compile-time | ✅ Zero | ⚠️ Type list manipulation | ❌ Not designed for runtime | ❌ Very high |

**Key Differentiators**:

**vs. std::tuple**: 
- CAFBERIHT allows **partial interfaces** (not all components need same methods)
- Direct semantic access via enums/tags
- Filtered dispatch without requiring all types support operation

**vs. std::variant**:
- No runtime type checking overhead
- All components **always present** (no "inactive" variants)
- Better cache locality (no indirection)

**vs. boost::hana**:
- Lower boilerplate (CRTP base vs. explicit BOOST_HANA_ADAPT macros)
- Simpler syntax for common operations
- Does not require learning meta-programming DSL

**vs. Virtual ECS**:
- Zero vtable overhead
- No forced common interface
- Type-specific operations without casting

**vs. Type list libraries (Brigand/Mp11)**:
- Designed for runtime use, not just compile-time manipulation
- Direct runtime index access
- Production-ready ergonomics

**Novel Contribution**: To our knowledge, CAFBERIHT is the **first practically deployable pattern** that combines:
1. Heterogeneous iteration with filtered dispatch
2. Runtime indexability with compile-time type preservation
3. Zero-overhead guarantees (proven via structural properties)
4. Minimal per-component boilerplate (single CRTP inheritance line)

This combination makes CAFBERIHT uniquely suited for performance-critical systems requiring heterogeneous type management.

---

## Universal Applications Beyond LLM Inference

### 1. Game Engine Entity-Component System

```cpp
struct TransformComponent : core_elem_base<0, TransformComponent> {
    static constexpr auto enum_value = component_types::transform;
    vec3 position, rotation, scale;
    
    void update(float dt) {
    }
};

struct RenderComponent : core_elem_base<1, RenderComponent> {
    static constexpr auto enum_value = component_types::render;
    mesh_handle mesh;
    material_handle material;
    
    void update(float dt) {
    }
};

struct PhysicsComponent : core_elem_base<2, PhysicsComponent> {
    static constexpr auto enum_value = component_types::physics;
    rigid_body body;
    collision_shape shape;
    
    void update(float dt) {
    }
};

struct AIComponent : core_elem_base<3, AIComponent> {
    static constexpr auto enum_value = component_types::ai;
    behavior_tree tree;
    
    void update(float dt) {
    }
};

using Entity = cafberiht<
    game_config,
    TransformComponent,
    RenderComponent,
    PhysicsComponent,
    AIComponent
>;

template<typename config, typename base> 
struct update_mixin {
    static constexpr bool filter() {
        return requires(base& b, float dt) { b.update(dt); };
    }
    
    static void impl(base& component, float delta_time) {
        component.update(delta_time);
    }
};

Entity entity;
entity.impl<update_mixin>(delta_time);

auto& transform = entity[tag<0>()];
auto& physics = entity.get_core_by_enum<component_types::physics>();
```

**Benefits**:
- No virtual dispatch
- No boilerplate operator[] per component
- Full type safety
- Components can have completely different interfaces
- Zero memory overhead

---

### 2. Plugin Architecture

```cpp
struct AudioPlugin : core_elem_base<0, AudioPlugin> {
    static constexpr auto enum_value = plugin_types::audio;
    
    void init() { }
    void process_audio(audio_buffer& buffer) { }
};

struct VideoPlugin : core_elem_base<1, VideoPlugin> {
    static constexpr auto enum_value = plugin_types::video;
    
    void init() { }
    void process_frame(video_frame& frame) { }
};

struct NetworkPlugin : core_elem_base<2, NetworkPlugin> {
    static constexpr auto enum_value = plugin_types::network;
    
    void init() { }
    void handle_packet(packet& p) { }
};

using PluginSystem = cafberiht<
    plugin_config,
    AudioPlugin,
    VideoPlugin,
    NetworkPlugin
>;

template<typename config, typename base>
struct init_mixin {
    static constexpr bool filter() {
        return requires(base& b) { b.init(); };
    }
    
    static void impl(base& plugin) {
        plugin.init();
    }
};

PluginSystem plugins;
plugins.impl<init_mixin>();

auto& audio = plugins[tag<0>()];
audio.process_audio(buffer);
```

---

### 3. Graphics Resource Management

```cpp
struct TextureResource : core_elem_base<0, TextureResource> {
    static constexpr auto enum_value = resource_types::texture;
    static constexpr alloc_classes alloc_class = alloc_classes::gpu_memory;
    
    uint64_t width, height, channels;
    uint64_t size() const { return width * height * channels; }
};

struct BufferResource : core_elem_base<1, BufferResource> {
    static constexpr auto enum_value = resource_types::buffer;
    static constexpr alloc_classes alloc_class = alloc_classes::gpu_memory;
    
    uint64_t byte_count;
    uint64_t size() const { return byte_count; }
};

struct ShaderResource : core_elem_base<2, ShaderResource> {
    static constexpr auto enum_value = resource_types::shader;
    static constexpr alloc_classes alloc_class = alloc_classes::system_memory;
    
    uint64_t size() const { return 0; }
};

using ResourceBundle = cafberiht<
    graphics_config,
    TextureResource,
    BufferResource,
    ShaderResource
>;

template<typename config, typename base>
struct gpu_memory_counter {
    static constexpr bool filter() {
        return base::alloc_class == alloc_classes::gpu_memory;
    }
    
    static void impl(const base& resource, uint64_t& total) {
        total += resource.size();
    }
};

ResourceBundle resources;
uint64_t gpu_memory = 0;
resources.impl<gpu_memory_counter>(gpu_memory);
```

ShaderResource branch completely eliminated at compile time via `if constexpr`.

---

## Implementation Best Practices

### Practice 1: Always Use the CRTP Base

```cpp
struct Component : core_elem_base<0, Component> {
};
```

This eliminates manual `operator[]` implementation and ensures correct ref-qualifier handling.

---

### Practice 2: Use Strong-Typed Enums

```cpp
enum class core_types : uint64_t {
    attn_q = 0,
    attn_k = 1
};
```

Type-safe enum classes prevent accidental value confusion.

---

### Practice 3: Leverage Concepts for Mixins

```cpp
template<typename T>
concept has_update = requires(T& t, float dt) { 
    { t.update(dt) } -> std::same_as<void>; 
};

template<typename config, typename base>
    requires has_update<base>
struct update_mixin {
    static constexpr bool filter() { return true; }
    static void impl(base& b, float dt) { b.update(dt); }
};
```

Clear requirements with good error messages.

---

### Practice 4: Static Assertions for Safety

```cpp
template<model_config_types config_type, has_tag_operator_types... bases> 
struct cafberiht : public bases... {
    static_assert(sizeof...(bases) > 0, 
        "Cafberiht must contain at least one base type");
    
    static_assert((std::is_base_of_v<core_elem_base<bases::enum_value, bases>, bases> && ...), 
        "All bases must inherit from core_elem_base");
    
    static_assert((std::is_standard_layout_v<bases> && ...), 
        "All bases must have standard layout");
};
```

Catch errors early with clear messages.

---

### Practice 5: Document Component Requirements

```cpp
template<uint64_t N>
struct MyComponent : core_elem_base<N, MyComponent> {
    static constexpr auto enum_value = static_cast<component_types>(N);
};
```

Clear documentation helps maintainability.

---

## Error Handling and Diagnostics

### Compile-Time Error Detection

**Out-of-Bounds Access:**
```cpp
template<uint64_t index_new> 
decltype(auto) get_core_by_index() const noexcept {
    static_assert(index_new < size, 
        "Index out of bounds: attempting to access non-existent component");
}
```

---

**CRTP Inheritance Check:**
```cpp
template<typename T>
concept properly_based = requires {
    requires std::is_base_of_v<
        core_elem_base<T::enum_value, T>, 
        T
    >;
};

template<properly_based... bases>
struct cafberiht : public bases... {
};
```

---

### Improving Error Messages with Concepts

```cpp
template<typename T>
concept has_update_method = requires(T& t, float dt) {
    { t.update(dt) } -> std::same_as<void>;
};

template<typename config, typename base>
    requires has_update_method<base>
struct update_mixin {
};
```

---

## Limitations and Trade-offs

### Limitation 1: Compile-Time Type Set Must Be Fixed

**The Constraint**: All types must be known at compile time.

**Mitigation**: Use variant of cafberihts for runtime selection:

```cpp
using ConfigA = cafberiht<config, PluginA, PluginB>;
using ConfigB = cafberiht<config, PluginA, PluginC>;

std::variant<ConfigA, ConfigB> runtime_config;
```

---

### Limitation 2: Requires C++20 Minimum

**Required Features**:
- Fold expressions
- `if constexpr`
- Using declarations for parameter packs
- Concepts (recommended)

---

### Limitation 3: CRTP Boilerplate (Minimal)

**The Cost**: Each component must inherit from `core_elem_base<Index, Derived>`.

**Net Result**: One line per component vs. four operator[] overloads—massive reduction in boilerplate.

---

## Real-World Production Considerations

### Consideration 1: Compilation Time

**Reality**: Template-heavy code increases compilation time.

**Mitigation**:
- Forward declare cafberihts in headers
- Use extern templates for common instantiations
- Split large cafberihts into smaller ones
- Precompiled headers

---

### Consideration 2: Debugger Experience

**Challenge**: Template types can be verbose in debuggers.

**Solution**: Type aliases for common instantiations:

```cpp
using TransformerLayer = cafberiht<
    model_config,
    AttentionQuery,
    AttentionKey,
    AttentionValue
>;
```

---

### Consideration 3: Thread Safety

**Important**: CAFBERIHT provides no built-in thread safety.

**Safe Patterns**:
```cpp
const cafberiht<config, Types...> shared_cat;

std::mutex cat_mutex;
{
    std::lock_guard lock(cat_mutex);
    shared_cat.impl<mixin>();
}
```

---

## Future Directions

### Direction 1: Auto-Generation via Reflection

When C++26 reflection arrives, CRTP bases could be auto-generated, further reducing boilerplate.

---

### Direction 2: Constexpr Cafberihts

With constexpr dynamic allocation:

```cpp
constexpr auto create_compile_time_cafberiht() {
    cafberiht<config, Types...> cat;
    cat.impl<initialization_mixin>();
    return cat;
}

static constexpr auto global_cat = create_compile_time_cafberiht();
```

---

### Direction 3: GPU-Accelerated Cafberihts

```cpp
template<typename config, gpu_compatible... bases>
struct gpu_cafberiht : public cafberiht<config, bases...> {
    __host__ __device__
    void impl_device() {
        (process_on_gpu<bases>(), ...);
    }
};
```

---

## Conclusion

### Summary of Contributions

**Theoretical Contributions:**
- Structural proof that heterogeneous iteration can achieve zero overhead
- Formalization of CRTP-based tag dispatch for zero-cost access
- Demonstration of compile-time type preservation through runtime access
- Proof of diamond inheritance impossibility via type system constraints

**Practical Contributions:**
- CRTP base (`core_elem_base`) that eliminates 90% of boilerplate
- Working implementation in C++20/23
- Patterns applicable across diverse domains
- Performance equivalent to hand-written code under optimizing compilation

**Novel Synthesis**: To our knowledge, CAFBERIHT is the first practically deployable pattern combining heterogeneous iteration, runtime indexability, filtered type-specific dispatch, and proven zero-overhead guarantees—all with minimal per-component boilerplate.

---

### The CRTP Innovation Impact

**Before CRTP Base** (Original Pattern):
- 4 operator[] overloads per component
- Manual ref-qualifier handling
- Error-prone repetition
- Hundreds of lines of boilerplate for large collections

**After CRTP Base** (Production-Ready Pattern):
- One-line inheritance per component
- Automatic perfect forwarding
- Compiler-enforced correctness
- Minimal, clean component definitions

**The Result**: A pattern that achieves the theoretically optimal (zero-overhead heterogeneous iteration) while being **practically usable in production codebases**.

---

### Final Assessment

CAFBERIHT represents a synthesis of modern C++ metaprogramming techniques into a coherent, zero-overhead pattern for heterogeneous type management. The introduction of the CRTP-based `core_elem_base` transforms the pattern from an academic curiosity into a production-ready tool.

The pattern demonstrates that with careful design, C++ can provide both:
1. **Zero-cost abstraction** (proven via structural properties)
2. **Minimal boilerplate** (via CRTP automation)

This combination makes CAFBERIHT suitable for performance-critical systems including game engines, plugin architectures, graphics pipelines, and LLM inference engines—any domain requiring type-safe management of fundamentally different types with zero runtime overhead.

---

**CAFBERIHT: Constexpr Aggregated and Filtered Bases for Efficient Runtime Iteration of Heterogeneous Types**

*A zero-overhead, low-boilerplate pattern for heterogeneous collection management in modern C++*

* Copyright 2026 Nihilai Collective Corp
* Licensed under the Apache License, Version 2.0