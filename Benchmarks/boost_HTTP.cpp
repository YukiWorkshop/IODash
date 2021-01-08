// Original file header:

//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

const char http_reply[] = "HTTP/1.0 200 OK\r\n"
			  "Date: Thu, 07 May 2020 12:49:30 GMT\r\n"
			  "Connection: close\r\n"
			  "Accept-Ranges: bytes\r\n"
			  "Last-Modified: Thu, 07 May 2020 12:49:25 GMT\r\n"
			  "Content-Length: 0\r\n"
			  "\r\n";

using boost::asio::ip::tcp;

class session
{
public:
	session(boost::asio::io_service& io_service)
		: socket_(io_service)
	{
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
					boost::bind(&session::handle_read, this,
						    boost::asio::placeholders::error,
						    boost::asio::placeholders::bytes_transferred));
	}

private:
	void handle_read(const boost::system::error_code& error,
			 size_t bytes_transferred)
	{
		if (!error)
		{
			boost::asio::async_write(socket_,
						 boost::asio::buffer(http_reply, sizeof(http_reply)-1),
						 boost::bind(&session::handle_write, this,
							     boost::asio::placeholders::error));
		}
		else
		{
			delete this;
		}
	}

	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			socket_.close();

			delete this;
		}
		else
		{
			delete this;
		}
	}

	tcp::socket socket_;
	enum { max_length = 1024, a = 111 };
	char data_[max_length];
};

class server
{
public:
	server(boost::asio::io_service& io_service, short port)
		: io_service_(io_service),
		  acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
	{
		start_accept();
	}

private:
	void start_accept()
	{
		session* new_session = new session(io_service_);
		acceptor_.async_accept(new_session->socket(),
				       boost::bind(&server::handle_accept, this, new_session,
						   boost::asio::placeholders::error));
	}

	void handle_accept(session* new_session,
			   const boost::system::error_code& error)
	{
		if (!error)
		{
			new_session->start();
		}
		else
		{
			delete new_session;
		}

		start_accept();
	}

	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
	try
	{
		boost::asio::io_service io_service;

		using namespace std; // For atoi.
		server s(io_service, 8083);
		std::cerr << "listening on port 8083\n";

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}