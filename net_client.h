#ifndef TCP_CLIENT_H__
#define TCP_CLIENT_H__
#include "net_session.h"
class NetClient : public NetSession
{
public:
    NetClient(bool tcp_socket = true, socket_type s = INVALID_SOCKET);
#ifdef CPP11
    NetClient(NetClient &&);
    NetClient &operator=(NetClient &&);
#endif
	bool bind(std::string ip, short int port);
    bool connect(std::string ip, short int port);
    void disconnect();
	~NetClient();
private:
	SOCKET create_socket();
	bool _tcp_socket;
};

#endif // TCP_CLIENT_H

