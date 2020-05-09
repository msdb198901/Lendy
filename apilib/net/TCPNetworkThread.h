#ifndef TCP_NETWORK_THREAD_H
#define TCP_NETWORK_THREAD_H

#include "Define.h"
#include <asio/steady_timer.hpp>
#include <asio/ip/tcp.hpp>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <algorithm>

namespace Net
{
	template<class SocketType>
	class CTCPNetworkThread
	{
		typedef std::vector<std::shared_ptr<SocketType> > SocketContainer;

	public:
		CTCPNetworkThread() :
			m_connections(0),
			m_stopped(false),
			m_pThread(nullptr),
			m_ioContext(1),
			m_acceptSocket(m_ioContext),
			m_updateTimer(m_ioContext)
		{}

		virtual ~CTCPNetworkThread()
		{
			Stop();
			if (m_pThread)
			{
				Wait();
				PDELETE(m_pThread);
			}
		}

		void Stop()
		{
			m_stopped = true;
			m_ioContext.stop();
		}

		bool Start()
		{
			if (m_pThread)
			{
				return false;
			}
			m_pThread = new std::thread(&CTCPNetworkThread::Run, this);
			return true;
		}

		void Wait()
		{
			assert(m_pThread);
			m_pThread->join();
			PDELETE(m_pThread);
		}

		int GetConnectionCount() const
		{
			return m_connections;
		}

		virtual void AddSocket(std::shared_ptr<SocketType> _socket)
		{
			std::lock_guard<std::mutex> _lock(m_newSocketLock);
			++m_connections;
			m_vNewSocket.push_back(_socket);
			SocketAdded(_socket);
		}

		asio::ip::tcp::socket *GetAcceptSocket()
		{
			return &m_acceptSocket;
		}

	protected:
		virtual void SocketAdded(std::shared_ptr<SocketType> _socket)
		{
			_socket->SocketBindCallBack();
		}

		virtual void SocketRemoved(std::shared_ptr<SocketType> _socket)
		{
			_socket->SocketShutCallBack();
		}

		void AddNewSockets()
		{
			std::lock_guard<std::mutex> _lock(m_newSocketLock);

			if (m_vNewSocket.empty())
			{
				return;
			}

			for (std::shared_ptr<SocketType> _socket : m_vNewSocket)
			{
				if (!_socket->IsOpen())
				{
					SocketRemoved(_socket);
					--m_connections;
				}
				else
				{
					m_vSocket.push_back(_socket);
				}
			}

			m_vNewSocket.clear();
		}

		void Run()
		{
			m_updateTimer.expires_from_now(std::chrono::milliseconds(10));
			m_updateTimer.async_wait(std::bind(&CTCPNetworkThread<SocketType>::Update, this));
			m_ioContext.run();

			m_vNewSocket.clear();
			m_vSocket.clear();
		}

		void Update()
		{
			if (m_stopped)
			{
				return;
			}

			m_updateTimer.expires_from_now(std::chrono::milliseconds(10));
			m_updateTimer.async_wait(std::bind(&CTCPNetworkThread<SocketType>::Update, this));

			AddNewSockets();

			m_vSocket.erase(std::remove_if(m_vSocket.begin(), m_vSocket.end(), [this](std::shared_ptr<SocketType> _socket) {
				if (!_socket->Update())
				{
					if (_socket->IsOpen())
					{
						_socket->CloseSocket();
					}
					
					this->SocketRemoved(_socket);
					--this->m_connections;
					return true;
				}
				return false;
			}), m_vSocket.end());
		}

	private:
		SocketContainer			m_vSocket;
		SocketContainer			m_vNewSocket;

		std::atomic<int>		m_connections;
		std::atomic<bool>		m_stopped;
		std::thread*			m_pThread;
		std::mutex				m_newSocketLock;

		Net::IOContext			m_ioContext;
		asio::ip::tcp::socket	m_acceptSocket;
		asio::steady_timer		m_updateTimer;
		
	};
}

#endif