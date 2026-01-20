/*
 * Copyright 2026 Nihilai Collective Corp
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// cafberiht_example.cpp

#include "config.hpp"

template<typename value_type>
concept integral_or_enum_types = std::integral<std::remove_cvref_t<value_type>> || std::is_enum_v<std::remove_cvref_t<value_type>>;

// For inserting "values" as "types" to be printed by the error_printer.
template<auto... values> struct error_printer_val_inserter;

// Undefined template - triggers compiler error with enum_error and values embedded in the type name
// This creates readable compile-time error messages that show exactly what went wrong
template<integral_or_enum_types auto enum_error, typename... values> struct static_assert_printer_impl;

// Static assertion wrapper that generates compile-time errors with contextual information
// Uses immediately-invoked constexpr lambda to trigger static_assert_printer_impl_val when condition fails
template<bool value, integral_or_enum_types auto enum_error, typename... values> struct static_assert_printer {
	static constexpr bool impl{ [] {
		if constexpr (!value) {
			static_assert_printer_impl<enum_error, values...>::nonexistent_value;
			return false;
		} else {
			return true;
		}
	}() };
};

// Neural network operation types - represents the full compute graph topology
// These enum values become compile-time indices that completely vanish in the final binary
enum class core_types {
	attn_q,
	attn_k,
	attn_v,
	attn_output,
	attn_norm,
	ffn_gate,
	ffn_up,
	ffn_down,
	moe_gate,
	moe_experts_gate,
	moe_experts_up,
	moe_experts_down,
	ffn_norm,
	token_embd,
	rope_freqs,
	output_norm,
	output,
	end_of_weights,
	inp_tokens,
	inp_pos,
	inp_out_ids,
	cache_k,
	cache_v,
	kq_mask,
	benchmark_data,
	end_of_input_only,
	inp_embd_get_rows,
	end_of_global_inputs,
	norm_rms_norm,
	attn_norm_mul,
	qcur_mul_mat,
	qcur_reshape,
	qcur_rope,
	kcur_mul_mat,
	kcur_reshape,
	kcur_rope,
	vcur_mul_mat,
	k_cache_view,
	k_cache_view_copy,
	vcur_transpose,
	v_cache_view,
	v_cache_view_copy,
	v_view,
	k_view,
	q_permute,
	kq_mul_mat,
	kq_soft_max,
	kqv_mul_mat,
	kqv_merged_permute,
	kqv_merged_cont,
	kqv_out_mul_mat,
	ffn_inp_add,
	norm_pre_ffn_rms_norm,
	ffn_norm_mul,
	ffn_gate_mul_mat,
	ffn_silu,
	ffn_up_mul_mat,
	ffn_gate_par_mul,
	ffn_out_mul_mat,
	moe_inp_add,
	norm_pre_moe_rms_norm,
	moe_norm_mul,
	moe_router_mul_mat,
	moe_router_softmax,
	moe_expert_select,
	moe_expert_gate_mul_mat,
	moe_expert_silu,
	moe_expert_up_mul_mat,
	moe_expert_gate_par_mul,
	moe_expert_down_mul_mat,
	moe_expert_weighted_sum,
	layer_out_add,
	end_of_per_block,
	node_1016_get_rows,
	node_1017_get_rows,
	final_ffn_inp_add,
	final_norm_pre_rms_norm,
	final_ffn_norm_mul,
	final_ffn_gate_mul_mat,
	final_ffn_silu,
	final_ffn_up_mul_mat,
	final_ffn_gate_par_mul,
	final_ffn_out_mul_mat,
	final_moe_inp_add,
	final_norm_pre_moe_rms_norm,
	final_moe_norm_mul,
	final_moe_router_mul_mat,
	final_moe_router_softmax,
	final_moe_expert_select,
	final_moe_expert_gate_mul_mat,
	final_moe_expert_silu,
	final_moe_expert_up_mul_mat,
	final_moe_expert_gate_par_mul,
	final_moe_expert_down_mul_mat,
	final_moe_expert_weighted_sum,
	final_layer_out_add,
	final_norm_rms_norm,
	result_norm_mul,
	result_output_mul_mat,
	sample_tokens,
	count
};

// Tag-based dispatch using std::integral_constant - enables O(1) compile-time indexing
// The tag<N> type encodes the index in the type system, allowing operator[] overload resolution
template<auto index> using tag = std::integral_constant<uint64_t, static_cast<uint64_t>(index)>;

struct core_aggregator;

// CRTP base providing tag-based indexing - each derived class is indexed by its enum value
// This creates a compile-time heterogeneous array where each element is uniquely addressable
template<integral_or_enum_types auto index, typename derived_type_new> struct core_elem_base {
	using derived_type = derived_type_new;

	constexpr decltype(auto) operator[](tag<index>) & noexcept {
		return *static_cast<derived_type*>(this);
	}

	constexpr decltype(auto) operator[](tag<index>) const& noexcept {
		return *static_cast<const derived_type*>(this);
	}
};

// Concrete interface type parameterized by enum value - holds per-operation state
// Each instantiation becomes a unique base class in the final cafberiht inheritance chain
template<integral_or_enum_types auto enum_value_new> struct core_interface : public core_elem_base<enum_value_new, core_interface<enum_value_new>> {
	uint64_t kernel_iteration_count{};
	static constexpr core_types enum_value{ enum_value_new };
};

// Aggregator defining the active subset of operations for this configuration
// Changing this array recompiles the entire inheritance hierarchy with different bases
struct core_aggregator {
	static constexpr std ::array values{ [] {
		std ::array<uint64_t, cafberiht_width> return_values{};
		for (uint64_t x = 0; x < cafberiht_width; ++x) {
			return_values[x] = x;
		}
		return return_values;
	}() };
};

// Mixin demonstrating compile-time filtering - only operations passing filter() are executed
// The filter() is evaluated at compile time, causing non-matching operations to be entirely elided
// Variant 1: Volatile write — should keep ALL calls
template<typename base_type> struct addition_mixin {
	static consteval bool filter() {
		return (static_cast<uint64_t>(base_type::enum_value) % 2) == 0;
	}
	static void impl(base_type&, volatile uint64_t& output_value) {
		output_value += rand();
	}
};

enum class cafberiht_errors {
	get_core_by_index_oob,
	invalid_base_cast,
	empty_cafberiht_bases_pack,
};

// The core cafberiht type - heterogeneous compile-time array via multiple inheritance
// Each base class is uniquely indexed by its enum_value, enabling O(1) lookup via operator[] overloading
// All indexing, filtering, and dispatch happens at compile time - runtime sees only the final operations
template<typename... bases> struct cafberiht : public bases... {
	static_assert(static_assert_printer<(sizeof...(bases) > 0), cafberiht_errors::empty_cafberiht_bases_pack>::impl);
	using bases::operator[]...;
	using enum_type = core_types;
	constexpr cafberiht() {
	}

	static constexpr uint64_t size{ sizeof...(bases) };

	// Fold expression that applies mixin_type::impl() to each base passing mixin_type::filter()
	// The if constexpr in impl_internal_filtered causes non-matching bases to generate zero code
	// Result: compiler unrolls the entire operation sequence with no loops or branches
	template<template<typename> typename mixin_type, typename... arg_types> constexpr void impl(arg_types&&... args) noexcept {
		(impl_internal_filtered<mixin_type, bases>(args...), ...);
	}

	// Direct O(1) access by enum value using tag dispatch - resolves entirely at compile time
	template<enum_type enum_value> decltype(auto) get_core_by_enum() noexcept {
		return (*this)[tag<static_cast<uint64_t>(enum_value)>()];
	}

	// Integer index access with compile-time bounds checking
	template<uint64_t index_new> decltype(auto) get_core_by_index() const noexcept {
		static_assert(static_assert_printer<(index_new < size), cafberiht_errors::get_core_by_index_oob, error_printer_val_inserter<index_new>>::impl);
		static constexpr uint64_t index{ static_cast<uint64_t>(index_transform_values[static_cast<uint64_t>(index_new)]) };
		return (*this)[tag<index>()];
	}

	// Reverse lookup: enum -> array index
	template<enum_type enum_value> static consteval uint64_t get_index_by_enum() noexcept {
		for (uint64_t x = 0; x < size; ++x) {
			if (static_cast<enum_type>(index_transform_values[x]) == enum_value) {
				return x;
			}
		}
		return std::numeric_limits<uint64_t>::max();
	}

  protected:
	// Per-base filtering and dispatch - if constexpr ensures filtered operations produce zero code
	// The compiler completely eliminates this function for non-matching bases
	template<template<typename> typename mixin_type, typename base_type, typename... arg_types>
	constexpr void impl_internal_filtered([[maybe_unused]] arg_types&&... args) noexcept {
		if constexpr (mixin_type<base_type>::filter()) {
			static_assert(static_assert_printer<std::is_base_of_v<base_type, cafberiht>, cafberiht_errors::invalid_base_cast>::impl);
			mixin_type<base_type>::impl(*this, args...);
		}
	}

	// Compile-time lookup table: array position -> enum value
	static constexpr uint64_t index_transform_values[sizeof...(bases)]{ static_cast<uint64_t>(bases::enum_value)... };
};

// Helper to extract the type of a specific base from the cafberiht hierarchy
template<typename cafberiht_type, integral_or_enum_types auto index> using get_cafberiht_type_at_enum =
	std::remove_cvref_t<decltype(std::declval<cafberiht_type>().template get_core_by_enum<index>())>;

// Metaprogramming machinery to build cafberiht<base_type<enum_0>, base_type<enum_1>, ...>
// from an aggregator's value array using index_sequence expansion
template<typename enum_type, typename aggregator_type, template<enum_type, typename...> typename base_type, typename... value_type> struct get_cafberiht_array;

template<typename enum_type, typename aggregator_type, template<enum_type, typename...> typename base_type, size_t... indices>
struct get_cafberiht_array<enum_type, aggregator_type, base_type, std::index_sequence<indices...>> {
	using type = cafberiht<base_type<static_cast<enum_type>(aggregator_type::values[indices])>...>;
};

// Type alias that constructs the final cafberiht type from an aggregator
// Expands aggregator::values into a parameter pack of base_type instantiations
template<typename enum_type, typename aggregator_type, template<enum_type, typename...> typename base_type> using get_cafberiht_array_t =
	std::remove_cvref_t<typename get_cafberiht_array<enum_type, aggregator_type, base_type, std::make_index_sequence<static_cast<uint64_t>(aggregator_type::values.size())>>::type>;

// Demonstration: the entire template machinery above compiles to 10 sequential rand() calls
// No loops, no branches, no runtime overhead - pure computational essence
// Check the assembly: search for "main:" (GCC/Clang) or "main PROC" (MSVC)
int main() {
	get_cafberiht_array_t<core_types, core_aggregator, core_interface> cafberiht_val{};
	uint64_t value{};
	cafberiht_val.impl<addition_mixin>(value);
	std::cout << "Final Value: " << value << ", For Cafberiht Width of: " << cafberiht_width << std::endl;
	return 0;
}