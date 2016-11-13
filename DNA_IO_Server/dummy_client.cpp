#include <iostream>
#include <deque>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "protocol.pb.h"
#include "PB_handler.h"
#include "Settings.h"

class DummyClient
{
public:
	DummyClient(boost::asio::io_service& io_service)
		: m_Io_service(io_service), m_Socket(io_service),
		m_bIslogin(false), m_nPacketBufferMark(0)
	{
		InitializeCriticalSectionAndSpinCount(&m_Lock, 4000);
	}

	~DummyClient()
	{
		EnterCriticalSection(&m_Lock);

		while (!send_queue.empty())
		{
			delete[] send_queue.front();
			send_queue.pop_front();
		}

		LeaveCriticalSection(&m_Lock);

		DeleteCriticalSection(&m_Lock);
	}

	bool Is_connect() const { return m_Socket.is_open(); }

	void Set_login() { m_bIslogin = true; }
	bool Is_login() const { return m_bIslogin; }

	void Set_Id(std::string id) { m_id = id; }
	std::string Get_Id() { return m_id; }

	void Set_Passwd(std::string passwd) { m_passwd = passwd; }
	std::string Get_Passwd() { return m_passwd; }

	void Connect(boost::asio::ip::tcp::endpoint endpoint)
	{
		m_nPacketBufferMark = 0;

		m_Socket.async_connect(endpoint,
			boost::bind(&DummyClient::handle_connect, this,
				boost::asio::placeholders::error)
		);
	}

	void Close()
	{
		if (m_Socket.is_open())
			m_Socket.close();
	}

	void Send(const bool b_Immediately, unsigned char *packet, size_t size)
	{ 
		unsigned char *SendData = nullptr;

		EnterCriticalSection(&m_Lock);

		if (b_Immediately == false)
		{
			SendData = new unsigned char[size];
			memcpy(SendData, packet, size);

			send_queue.push_back(SendData);
		}
		else
		{
			SendData = packet;
		}

		if (b_Immediately || send_queue.size() < 2)
		{
			boost::asio::async_write(m_Socket, boost::asio::buffer(SendData, size),
				boost::bind(&DummyClient::handle_write, this,
					boost::asio::placeholders::error, 
					boost::asio::placeholders::bytes_transferred)
			);
		}

		LeaveCriticalSection(&m_Lock);
	}

private:
	void Receive()
	{
	}

	void handle_connect(const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout << "[Dummy Client] 서버와 연결에 성공했습니다." << std::endl;
			std::cout << "이름과 패스워드를 입력해주세요." << std::endl;
	
			Receive();
		}
		else
		{
			std::cout << "[Dummy Client] 서버와 연결에 실패했습니다." << std::endl;
			std::cout << "[Dummy Client] Error No: " << error.value()
				<< ", Message: " << error.message() << std::endl;
		}
	}
	
	void handle_write(const boost::system::error_code& error,
		size_t bytes_transferred)
	{
		EnterCriticalSection(&m_Lock);

		delete[] send_queue.front();
		send_queue.pop_front();

		unsigned char *SendData = nullptr;

		if (!send_queue.empty())
		{
			SendData = send_queue.front();
		}

		LeaveCriticalSection(&m_Lock);

		if (SendData != nullptr)
		{
			Send(true, SendData, sizeof(SendData));
		}
	}

	void handle_receive(const boost::system::error_code& error,
		size_t bytes_transferred)
	{

	}

	std::string m_id;
	std::string m_passwd;

	boost::asio::io_service& m_Io_service;
	boost::asio::ip::tcp::socket m_Socket;

	std::array<google::protobuf::uint8, MAX_RECEIVE_BUF_LEN> ReceiveBuf;

	int m_nPacketBufferMark;
	google::protobuf::uint8 m_PacketBuf[MAX_RECEIVE_BUF_LEN * 10];

	CRITICAL_SECTION m_Lock;
	std::deque<unsigned char *> send_queue;

	bool m_bIslogin;
};

class DummyClientTest
{
public:
	void Test()
	{
		boost::asio::io_service io_service;
		auto endpoint = boost::asio::ip::tcp::endpoint(
			boost::asio::ip::address::from_string("127.0.0.1"), PORT_NUM);

		dna_info::LoginRequest req;
		req.set_id("kernelgarden");
		req.set_passwd("sunrint123");

		PacketHeader header;
		header.size = req.ByteSize();
		header.type = dna_info::LOGIN_REQ;

		int nWrite = 0;
		unsigned char buf[MAX_RECEIVE_BUF_LEN];

		memcpy(buf, &header, sizeof(header));
		nWrite += sizeof(header);
		memcpy(&buf[nWrite], &req, header.size);

		DummyClient client(io_service);
		client.Connect(endpoint);

		boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));

		char szMessage[MAX_RECEIVE_BUF_LEN] = { 0, };

		while (std::cin.getline(szMessage, MAX_RECEIVE_BUF_LEN))
		{
			if (strnlen_s(szMessage, MAX_RECEIVE_BUF_LEN) == 0)
				break;

			if (client.Is_connect() == false)
			{
				std::cout << "[Dummy Client] 서버와 연결되지 않았습니다." << std::endl;
				continue;
			}

			if (client.Is_login() == false)
			{
				int buf_size = 0;
				char id[300];
				char passwd[300];
				dna_info::LoginRequest req;

				std::cout << "ID: ";
				std::cin >> id;
				std::cout << "PASSWD: ";
				std::cin >> passwd;

				req.set_id(id);
				req.set_passwd(passwd);

				buf_size += sizeof(PacketHeader) + req.ByteSize();
				protobuf::uint8 *outputBuf = new protobuf::uint8[buf_size];

				protobuf::io::ArrayOutputStream output_array_stream(outputBuf, buf_size);
				protobuf::io::CodedOutputStream output_coded_stream(&output_array_stream);

				WriteMessageToStream(req, dna_info::LOGIN_REQ, output_coded_stream);

				client.Send(false, outputBuf, buf_size);
			}
			else
			{
				std::cout << "[Dummy Client] 로그인 되었습니다." << std::endl;
			}
		}

		io_service.stop();
		client.Close();
		thread.join();

		std::cout << "Dummy Client Test 종료" << std::endl;
	}
};