////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   socket.h
*** \author Alistair Lynn, arplynn@gmail.com
*** \brief  Header file for socket access abstraction
*** 
*** Socket abstraction for TCP sockets.
*** ***************************************************************************/

#ifndef __SOCKET_HEADER__
#define __SOCKET_HEADER__

#include "utils.h"
#include "defs.h"
#include <SDL/SDL.h>
#ifdef __MACH__
#include <SDL_net/SDL_net.h>
#else
#include <SDL/SDL_net.h>
#endif
#include <stdarg.h>

namespace hoa_socket
{

/** ****************************************************************************
*** \brief TCP socket
***
*** TCP client socket with basic connectivity functionality.
***
*** \note 1) Server functionality will be added on soon
*** ***************************************************************************/
class Socket
{
private:
	TCPsocket sock;
	SDLNet_SocketSet set;
	bool connected;
public:
	/** \brief Standard constructor
	*** 
	*** Initialises SDL_net if it is not already started, and creates a TCP socket
	**/
	Socket ();
	/** \brief Standard destructor
	*** 
	*** Quits SDL_net if it is not needed, disconnects if the TCP socket is still connected
	**/
	~Socket ();
	
	/** \brief Checks whether the socket is connected
	**/
	bool IsConnected ();
	
	/** \brief Connects to a server
	*** \param host The hostname to which to connect
	*** \param port The TCP port number with which to connect
	**/
	void Connect ( const std::string& host, uint16 port );
	/** \brief Disconnects from the server
	**/
	void Disconnect ();
	
	/** \brief Checks if there is any incoming data in the queue
	*** \param wait_time Time to wait for incoming data, in milliseconds
	**/
	bool IsQueued ( uint32 wait_time = 0 );
	
	/** \brief Sends binary data
	*** \param d A pointer to a block of data
	*** \param len The number of bytes of data
	*** \return The amount of data sent
	**/
	uint32 SendBinary ( void* data, uint32 len );
	
	/** \brief Reads binary data
	*** \param d A pointer to the location in which to store the data
	*** \param len The length of the data up to which to read
	*** \return The amount of data actually read
	**/
	uint32 RecvBinary ( void* location, uint32 maxlen );
	
	/** \brief Writes textual data
	*** \param fmt The format of the data, in printf style
	*** \param ... Other arguments, following from the format given (the format is a const char*, don't forget)
	**/
	void Write ( const char* format, ... );
	
	/** \brief Reads one line
	*** \return One line of text. Note that this is a std::string
	**/
	std::string ReadLine ();
	
	/** \brief Scans one line, in scanf style
	*** \param fmt The format to scan
	*** \param ... Other arguments, following from the scanf format given
	**/
	void ScanLine ( const char* format, ... );
};

}

#endif
