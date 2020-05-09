#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#include <asio/io_context.hpp>
#include <asio/post.hpp>

namespace Net
{
	class IOContext
	{
	public:
		IOContext() : m_io()
		{
		}
	
		explicit IOContext(int concurrency) : m_io(concurrency) 
		{
		}

		operator asio::io_context&() 
		{ 
			return m_io; 
		}

		operator asio::io_context const&() const
		{ 
			return m_io; 
		}

		std::size_t run() 
		{ 
			return m_io.run(); 
		}

		void stop() 
		{ 
			m_io.stop();
		}

		asio::io_context::executor_type get_executor() noexcept 
		{ 
			return m_io.get_executor();
		}

	private:
		asio::io_context m_io;
	};

	template <typename T>
#ifdef LENDY_COMPILER_14
	inline decltype(auto) post(asio::io_context &io, T&& t)
	{
		return asio::post(io, std::forward<T>(t));
	}
#else
	inline void post(asio::io_context &io, T&& t)
	{
		asio::post(io, std::forward<T>(t));
		return;
	}
#endif

#ifdef LENDY_COMPILER_14
	template<typename T>
	inline decltype(auto) get_io_context(T&& io)
	{
		return io.get_executor().context();
	}
#endif
}

#endif