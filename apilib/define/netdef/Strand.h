#ifndef STRAND_H
#define STRAND_H

#include "IOContext.h"
#include <asio/strand.hpp>

namespace Net
{
	class Strand : public asio::io_context::strand
	{
	public:
		Strand(Net::IOContext& ioContext) : asio::io_context::strand(ioContext) { }
	};

	template<typename T>
#ifdef LENDY_COMPILER_14
	inline decltype(auto) bind_executor(Strand& strand, T&& t)
#else
	inline auto bind_executor(Strand& strand, T&& t)
#endif
	{
		return strand.wrap(std::forward<T>(t));
	}
}

#endif