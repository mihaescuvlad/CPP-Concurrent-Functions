#ifndef _THREAD_SAFE_QUEUE_
#define _THREAD_SAFE_QUEUE_

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace parallelism {
	template<typename T>
	class threadsafe_queue
	{
	private:
		mutable std::mutex m_mutex;
		std::queue<T> m_data_queue;
		std::condition_variable m_data_cond;
	public:
		~threadsafe_queue() = default;
		threadsafe_queue() = default;

		threadsafe_queue(threadsafe_queue const& other)
		{
			std::lock_guard<std::mutex> otherlk(other.m_mutex);

			m_data_queue = other.m_data_queue;
		}

		threadsafe_queue(threadsafe_queue&& other) noexcept
		{
			std::lock_guard<std::mutex> otherlk(other.m_mutex);

			m_data_queue = std::move(other.m_data_queue);
		}

		threadsafe_queue& operator=(const threadsafe_queue& other)
		{
			threadsafe_queue copy = other;
			copy.swap(*this);
			return *this;
		}

		threadsafe_queue& operator=(threadsafe_queue&& other) noexcept
		{
			std::lock_guard<std::mutex> otherlk(other.m_mutex);

			m_data_queue = std::move(other.m_data_queue);

			return *this;
		}

		void push(T new_value)
		{
			std::lock_guard<std::mutex> lk(m_mutex);

			m_data_queue.push(new_value);
			m_data_cond.notify_one();
		}

		void wait_and_pop(T& value)
		{
			std::unique_lock<std::mutex> lk(m_mutex);

			m_data_cond.wait(lk, [this]() -> void
			{
					return m_data_queue.empty() == false;
			});

			value = m_data_queue.front();
			m_data_queue.pop();
		}

		std::shared_ptr<T> wait_and_pop()
		{
			std::unique_lock<std::mutex> lk(m_mutex);

			m_data_cond.wait(lk, [this]() -> void
			{
				return m_data_queue.empty() == false;
			});

			std::shared_ptr<T> value{ std::make_shared<T>(m_data_queue.front()) };
			m_data_queue.pop();

			return value;
		}

		bool try_pop(T& value)
		{
			std::lock_guard<std::mutex> lk(m_mutex);

			if (m_data_queue.empty())
			{
				return false;
			}

			value = m_data_queue.front();
			m_data_queue.pop();

			return true;
		}

		std::shared_ptr<T> try_pop()
		{
			std::lock_guard<std::mutex> lk(m_mutex);

			if(m_data_queue.empty())
			{
				return std::make_shared<T>();
			}

			std::shared_ptr<T> value{ std::make_shared<T>(m_data_queue.front()) };
			m_data_queue.pop();

			return value;
		}

		bool empty() const
		{
			std::lock_guard<std::mutex> lk(m_mutex);

			return m_data_queue.empty();
		}

		void swap(threadsafe_queue& other) noexcept
		{
			std::lock_guard<std::mutex> lk(m_mutex);
			std::lock_guard<std::mutex> otherlk(other.m_mutex);

			std::swap(m_data_queue, other.m_data_queue);
		}

		friend void swap(threadsafe_queue& a, threadsafe_queue& b) noexcept
		{
			a.swap(b);
		}
	};
}


#endif