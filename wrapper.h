#pragma once
#include <memory>

class function_wrapper {
public:
	template<typename T>
	function_wrapper(T&& t) :
		impl(new impl_type<T>(std::move(t))) { }
	void operator()() { impl->call(); }
	function_wrapper() = default;
	function_wrapper(function_wrapper&& other) : impl(std::move(other.impl)) { }
	function_wrapper& operator=(function_wrapper&& other) {
		impl = std::move(other.impl);
		return *this;
	}
	function_wrapper(const function_wrapper&) = delete;
	function_wrapper(function_wrapper&) = delete;
	function_wrapper& operator=(const function_wrapper&) = delete;
private:
	struct impl_base {
		virtual void call() = 0;
		virtual ~impl_base() {}
	};

	std::unique_ptr<impl_base> impl;

	template<typename T>
	struct impl_type : impl_base {
		T t;
		impl_type(T&& t_) : t(std::move(t_)) { }
		void call() { t(); }
	};
};