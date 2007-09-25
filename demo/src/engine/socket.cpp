////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   socket.cpp
*** \author Alistair Lynn, arplynn@gmail.com
*** \brief  Source file for socket access abstraction
*** ***************************************************************************/

#include "socket.h"

static uint32 num_sockets = 0;

namespace hoa_socket {

Socket::Socket ()
{
	if (num_sockets == 0)
		SDLNet_Init();
	num_sockets++;
	sock = NULL;
	set = NULL;
	connected = false;
}

Socket::~Socket ()
{
	if (IsConnected())
		Disconnect();
	num_sockets--;
	if (num_sockets == 0)
		SDLNet_Quit();
}

bool Socket::IsConnected ()
{
	return connected;
}

void Socket::Connect ( const std::string& host, uint16 port )
{
	IPaddress ipa;
	SDLNet_ResolveHost(&ipa, (char*)host.c_str(), port);
	sock = SDLNet_TCP_Open ( &ipa );
	if (sock != NULL)
	{
		set = SDLNet_AllocSocketSet(1);
		if (!set)
			SDLNet_TCP_Close(sock);
		else
		{
			SDLNet_TCP_AddSocket(set, sock);
			connected = true;
		}
	}
}

void Socket::Disconnect ()
{
	if (!IsConnected())
		return;
	SDLNet_TCP_Close(sock);
	SDLNet_FreeSocketSet(set);
	connected = false;
}

bool Socket::IsQueued(uint32 wait_time) {
	if (!IsConnected())
		return false;
	return SDLNet_CheckSockets ( set, wait_time );
}

uint32 Socket::SendBinary(const void* data, uint32 len) {
	if (!IsConnected())
		return 0;
	return SDLNet_TCP_Send(sock, const_cast<void*> (data), len);
}

uint32 Socket::RecvBinary(void* location, uint32 len) {
	if (!IsConnected())
		return 0;
	return SDLNet_TCP_Recv ( sock, location, len );
}

std::string Socket::ReadLine() {
	std::string ret;
	char* res = (char*)malloc(1);

	while (RecvBinary((void*)res, 1)) {
		if (*res == '\n')
			break;
		ret.append ( 1, *res );
	}

	std::string::size_type pos;
	if ((pos = ret.find_last_of('\r')) != std::string::npos) {
		ret.erase(pos, 1);
	}
	free((void*)res);
	return ret;
}

void Socket::ScanLine ( const char* format, ... ) {
	va_list va;
	va_start(va, format);
	
	std::string line = ReadLine ();
	
	//vsscanf ( line.c_str(), format, va );
	
	va_end(va);
}

} // namespace hoa_socket
