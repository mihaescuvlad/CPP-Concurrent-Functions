#ifndef _THREAD_SAFE_STACK_
#define _THREAD_SAFE_STACK_

#include <exception>
#include <memory>
#include <mutex>
#include <stack>

namespace parallelism 
{
	struct empty_stack : std::exception
	{
		const char* what() throw();
	};

	template<typename T>
	class threadsafe_stack
	{
	private:
		std::stack<T> m_data;
		mutable std::mutex m_mutex;
	public:
		threadsafe_stack() {};
		threadsafe_stack(const threadsafe_stack& other)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			
			m_data = other.m_data;
		}
		threadsafe_stack& operator=(const threadsafe_stack&) = delete;

		void push(T new_value)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			
			m_data.push(new_value);
		}

		std::shared_ptr<T> pop()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_data.empty()) throw empty_stack();
			
			const std::shared_ptr<T> result(std::make_shared<T>(m_data.top()));
			m_data.pop();

			return result;
		}

		void pop(T& value)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_data.empty()) throw empty_stack();

			value = m_data.top();
			m_data.pop();
		}

		bool empty() const
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_data.empty();
		}
	};
}

#endif