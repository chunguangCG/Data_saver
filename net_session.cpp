#include "stdafx.h"
#include "net_session.h"

NetSession::NetSession(socket_type socket)
    :_socket(socket)
{
    // empty
}

#ifdef CPP11
NetSession::NetSession(NetSession &&other)
{
    _socket = other._socket;
    other._socket = INVALID_SOCKET;
}
#endif

NetSession::~NetSession()
{
    closesocket(_socket);
    _socket = INVALID_SOCKET;
}

#ifdef CPP11
NetSession& NetSession::operator=(NetSession &&other)
{
    _socket = other._socket;
    other._socket = INVALID_SOCKET;
    return *this;
}
#endif

bool NetSession::waitForRecv(double max_seconds)
{
	long seconds = static_cast<long>(max_seconds);
	long u_seconds = static_cast<long>((max_seconds - seconds)*1000000);
	timeval t = {seconds, u_seconds};
	fd_set s;
	FD_ZERO(&s);
	FD_SET(_socket, &s);
	int ret = select(0, &s, 0, 0, &t);
	return ret > 0;
}

bool NetSession::waitForSend(double max_seconds)
{
	long seconds = static_cast<long>(max_seconds);
	long u_seconds = static_cast<long>((max_seconds - seconds)*1000000);
	timeval t = {seconds, u_seconds};
	fd_set s;
	FD_ZERO(&s);
	FD_SET(_socket, &s);
	return select(0, 0, &s, 0, &t) > 0;
}

bool NetSession::recv(buffer_type &buf)
{
	const unsigned int offset = buf.size();
	buf.resize(buf.capacity());
	const int num = ::recv(_socket, &buf.front()+offset, buf.size()-offset, 0);
	if (num <= 0)
	{
		closesocket(_socket);
		_socket = INVALID_SOCKET;
		return false;
	}
    buf.resize(offset+num);
    return true;
}

bool NetSession::send(const char* ptr, const unsigned int len)
{
	unsigned int sz = 0;
	bool res = true;
	while (sz < len)
	{
		const int ret = ::send(_socket, ptr+sz, len-sz, 0);
		if (ret == SOCKET_ERROR)
		{
			res = false;
			break;
		}
		sz += ret;
	}
	return res;
}
bool NetSession::send(const buffer_type &buf)
{
    unsigned int sz = 0;
    bool res = true;
    while (sz < buf.size())
    {
        const int ret = ::send(_socket, &buf.front()+sz, buf.size()-sz, 0);
        if (ret == SOCKET_ERROR)
        {
            res = false;
            break;
        }
        sz += ret;
    }
    return res;
}

const std::string NetSession::error_message()
{
    const int error_code = WSAGetLastError();
    char* pmsg = 0;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  error_code,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  pmsg, 0, NULL);
    std::string error_message = std::string(pmsg);
    LocalFree(pmsg);
    return error_message;
}

const std::string NetSession::peerAddress()
{
	address_type addr;
	int name_length = sizeof(address_type);
	if (0 == ::getpeername(_socket, (sockaddr*)&addr, &name_length)) 
		return std::string(inet_ntoa(addr.sin_addr));
	else
		return std::string();
}

const unsigned short int NetSession::peerPort()
{
	address_type addr;
	int name_length = sizeof(address_type);
	if (0 == ::getpeername(_socket, (sockaddr*)&addr, &name_length)) 
		return addr.sin_port;
	else
		return 0;
}

const std::string NetSession::localAddress()
{
	address_type addr;
	int name_length = sizeof(address_type);
	if (0 == ::getsockname(_socket, (sockaddr*)&addr, &name_length))
		return std::string(inet_ntoa(addr.sin_addr));
	else
		return std::string();
}

const unsigned short int NetSession::localPort()
{
	address_type addr;
	int name_length = sizeof(address_type);
	if (0 == ::getsockname(_socket, (sockaddr*)&addr, &name_length)) 
		return addr.sin_port;
	else
		return 0;
}