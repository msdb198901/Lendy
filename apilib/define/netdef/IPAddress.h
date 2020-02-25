#ifndef IP_ADDRESS_H
#define IP_ADDRESS_H

#include "Define.h"
#include <asio/ip/address.hpp>

namespace Net
{
	using asio::ip::make_address;
	using asio::ip::make_address_v4;

	inline uint32 address_to_uint(asio::ip::address_v4 const& address)
	{
		return address.to_uint();
	}


}

#endif