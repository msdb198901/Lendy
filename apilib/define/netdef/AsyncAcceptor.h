#ifndef ASYNC_ACCEPTOR_H
#define AYSNC_ACCEPTOR_H

#include "Define.h"
#include "IOContext.h"
#include "IPAddress.h"
#include <asio/ip/tcp.hpp>
#include <functional>
#include <atomic>

#define MAX_LISTEN_CONNECTIONS asio::socket_base::max_listen_connections

namespace Net
{
	using asio::ip::tcp;

	typedef std::function<void(tcp::socket&& newSocket, uint32 threadIndex)> AcceptCallBack;

	class AsyncAcceptor
	{
	public:
		AsyncAcceptor(Net::IOContext& ioContext, std::string const& strIP, uint16 port) :
			m_acceptor(ioContext),
			m_endpoint(asio::ip::make_address(strIP), port),
			m_socket(ioContext),
			m_closed(false),
			m_socketFactory(std::bind(&AsyncAcceptor::DefeaultSocketFactory, this))
		{
		}

		template<class T>
		void AsyncAccept()
		{
			m_acceptor.async_accept(m_socket, [this](asio::error_code ec) {
				if (!ec)
				{
					try
					{
						std::make_shared<T>(std::move(this->m_socket))->Start();
					}
					catch (const std::exception&)
					{
						assert(nullptr);
					}
				}
			});

			if (!m_closed)
			{
				this->AsyncAccept<T>();
			}
		}

		void AsyncAcceptWithCallBack(AcceptCallBack acceptCallBack)
		{
			assert(m_acceptor.is_open());

			tcp::socket* socket;
			uint32 threadIndex;
			std::tie(socket, threadIndex) = m_socketFactory();
			m_acceptor.async_accept(*socket, [this, socket, threadIndex, acceptCallBack](asio::error_code ec) {
				if (!ec)
				{
					try
					{
						socket->non_blocking(true);
						acceptCallBack(std::move(*socket), threadIndex);
					}
					catch (asio::error_code const& err)
					{
						(void)err;
					}

					if (!m_closed)
					{
						this->AsyncAcceptWithCallBack(acceptCallBack);
					}
				}
			});
		}

		bool Bind()
		{
			asio::error_code ec;
			m_acceptor.open(m_endpoint.protocol(), ec);
			
			if (ec)
			{
				return false;
			}

#if LENDY_PLATFORM != LENDY_PLATFORM_WINDOWS
			m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true), ec);
			if (ec)
			{
				return false;
			}
#endif
			m_acceptor.bind(m_endpoint, ec);
			if (ec)
			{
				return false;
			}

			m_acceptor.listen(MAX_LISTEN_CONNECTIONS, ec);

			if (ec)
			{
				return false;
			}

			return true;
		}

		void Close()
		{
			if (m_closed.exchange(true))
			{
				return;
			}

			asio::error_code ec;
			m_acceptor.close(ec);
		}
	private:
		std::pair<tcp::socket*, uint32> DefeaultSocketFactory() { return std::make_pair(&m_socket, 0); }

		tcp::acceptor		m_acceptor;
		tcp::endpoint		m_endpoint;
		tcp::socket			m_socket;
		std::atomic<bool>	m_closed;
		std::function<std::pair<tcp::socket*, uint32>()> m_socketFactory;
	};
}

#endif