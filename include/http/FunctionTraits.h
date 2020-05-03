#pragma once

// This file is taken from the sqlite modern cpp wrapper project
// https://github.com/SqliteModernCpp/sqlite_modern_cpp
//
// It contains minor modifications from the original, such as changing the
// namespace

// MIT License
//
// Copyright (c) 2017 aminroosta
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <tuple>
#include <type_traits>

namespace ARo::Http::Internal {

		template<typename> struct function_traits;

		template <typename Function>
		struct function_traits : public function_traits<
			decltype(&std::remove_reference<Function>::type::operator())
		> { };

		template <
			typename    ClassType,
			typename    ReturnType,
			typename... Arguments
		>
		struct function_traits<
			ReturnType(ClassType::*)(Arguments...) const
		> : function_traits<ReturnType(*)(Arguments...)> { };

    /* support the non-const operator ()
     * this will work with user defined functors */
		template <
			typename    ClassType,
			typename    ReturnType,
			typename... Arguments
		>
		struct function_traits<
			ReturnType(ClassType::*)(Arguments...)
		> : function_traits<ReturnType(*)(Arguments...)> { };

		template <
			typename    ReturnType,
			typename... Arguments
		>
		struct function_traits<
			ReturnType(*)(Arguments...)
		> {
			typedef ReturnType result_type;

			template <std::size_t Index>
			using argument = typename std::tuple_element<
				Index,
				std::tuple<Arguments...>
			>::type;

			static const std::size_t arity = sizeof...(Arguments);
		};

} // namespace ARo::Http::Utility

