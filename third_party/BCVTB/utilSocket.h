// Methods for interfacing clients using BSD sockets.

/*
********************************************************************
Copyright Notice
----------------

Building Controls Virtual Test Bed (BCVTB) Copyright (c) 2008-2009, The
Regents of the University of California, through Lawrence Berkeley
National Laboratory (subject to receipt of any required approvals from
the U.S. Dept. of Energy). All rights reserved.

If you have questions about your rights to use or distribute this
software, please contact Berkeley Lab's Technology Transfer Department
at TTD@lbl.gov

NOTICE.  This software was developed under partial funding from the U.S.
Department of Energy.  As such, the U.S. Government has been granted for
itself and others acting on its behalf a paid-up, nonexclusive,
irrevocable, worldwide license in the Software to reproduce, prepare
derivative works, and perform publicly and display publicly.  Beginning
five (5) years after the date permission to assert copyright is obtained
from the U.S. Department of Energy, and subject to any subsequent five
(5) year renewals, the U.S. Government is granted for itself and others
acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide
license in the Software to reproduce, prepare derivative works,
distribute copies to the public, perform publicly and display publicly,
and to permit others to do so.


Modified BSD License agreement
------------------------------

Building Controls Virtual Test Bed (BCVTB) Copyright (c) 2008-2009, The
Regents of the University of California, through Lawrence Berkeley
National Laboratory (subject to receipt of any required approvals from
the U.S. Dept. of Energy).  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. Neither the name of the University of California, Lawrence
      Berkeley National Laboratory, U.S. Dept. of Energy nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

You are under no obligation whatsoever to provide any bug fixes,
patches, or upgrades to the features, functionality or performance of
the source code ("Enhancements") to anyone; however, if you choose to
make your Enhancements available either publicly, or directly to
Lawrence Berkeley National Laboratory, without imposing a separate
written license agreement for such Enhancements, then you hereby grant
the following license: a non-exclusive, royalty-free perpetual license
to install, use, modify, prepare derivative works, incorporate into
other computer software, distribute, and sublicense such enhancements or
derivative works thereof, in binary and source code form.

********************************************************************
*/

///////////////////////////////////////////////////////
/// \file   utilSocket.h
///
/// \brief  Methods for interfacing clients
///         using BSD sockets.
///
/// \author Michael Wetter,
///         Simulation Research Group,
///         LBNL,
///         MWetter@lbl.gov
///
/// \date   2007-12-01
///
/// \version $Id: utilSocket.h 55724 2009-09-16 17:51:58Z mwetter $
///
/// This file provides methods that allow clients to
/// establish a socket connection. Clients typically call
/// the method \c establishclientsocket()
/// once, and then call the method
/// \c exchangedoubleswithsocket() in each time step.
/// At the end of the simulation, a client should call
/// \c closeipc() to close the socket connection.
/// These three functions are the only functions that are
/// needed to interface a client to the BCVTB.
///
/// \sa establishclientsocket
/// \sa exchangedoubleswithsocket
/// \sa closeipc
///
///////////////////////////////////////////////////////
#ifndef _UTILSOCKET_H_
#define _UTILSOCKET_H_
#ifdef _WIN32  // Windows compiler
#  include <windows.h>
#  include <winsock.h>

//#include <winsock2.h>
//#include <ws2tcpip.h> // this gives compile error due to bug in .h file
#else
// Include arpa/inet.h so that inet_ntoa is defined for 64bit linux
// and on Mac OS X 10.6.1 Snow Leopard
#  include <arpa/inet.h>
#  include <sys/socket.h>
//#include <netinet/in.h>
#  include <netdb.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <errno.h>

#include "defines.h"

extern FILE* f1;
#define HEADER_LENGTH 21600
extern int REQUIRED_READ_LENGTH;
extern int REQUIRED_WRITE_LENGTH;

/// This will be overwritten to contain the
/// version number of the server
extern int SERVER_VERSION;

////////////////////////////////////////////////////////////////
/// Appends a character array to another character array.
///
/// The array size of \c buffer may be extended by this function
/// to prevent a buffer overflow. If \c realloc fails to allocate
/// new memory, then this function calls \c perror(...) and
/// returns \c EXIT_FAILURE.
///
///\param buffer The buffer to which the character array will be added.
///\param toAdd The character array that will be appended to \c buffer
///\param bufLen The length of the character array \c buffer. This parameter will
///              be set to the new size of \c buffer if memory was reallocated.
///\return 0 if no error occurred.
int save_append(char** buffer, const char* toAdd, size_t* bufLen);

////////////////////////////////////////////////////////////////
/// Assembles the buffer that will be exchanged through the IPC.
///
///\param flag The communication flag.
///\param nDbl The number of double values.
///\param nInt The number of integer values.
///\param nBoo The number of boolean values.
///\param dblVal The array that stores the double values.
///\param intVal The array that stores the integer values.
///\param booVal The array that stores the boolean values.
///\param buffer The buffer into which the values will be written.
///\param bufLen The buffer length prior and after the call.
///\return 0 if no error occurred.
int assembleBuffer(int flag, int nDbl, int nInt, int nBoo, double curSimTim, double dblVal[], int intVal[], int booVal[], char** buffer,
                   size_t* bufLen);

/////////////////////////////////////////////////////////////////
/// Gets an integer and does the required error checking.
///
///\param nptr Pointer to character buffer that contains the number.
///\param endptr After return, this variable contains a pointer to the
///            character after the last character of the number.
///\param base Base for the integer.
///\param The value contained in the character buffer.
///\return 0 if no error occurred.
int getIntCheckError(const char* nptr, char** endptr, const int base, int* val);

/////////////////////////////////////////////////////////////////
/// Gets a double and does the required error checking.
///
///\param nptr Pointer to character buffer that contains the number.
///\param endptr After return, this variable contains a pointer to the
///            character after the last character of the number.
///\param The value contained in the character buffer.
///\return 0 if no error occurred.
int getDoubleCheckError(const char* nptr, char** endptr, double* val);

/////////////////////////////////////////////////////////////////
/// Disassembles the header of the buffer that has been received through the IPC.
///
/// This method is separated from disassemblebuffer since in the
/// first call, we only need to peek at the header to assign
/// a long enough buffer for the read operation.
///
///\param buffer The buffer that contains the values to be parsed.
///\param flag The communication flag.
///\param nDbl The number of double values received.
///\param nInt The number of integer values received.
///\param nBoo The number of boolean values received.
///\return 0 if no error occurred.
int disassembleHeaderBuffer(const char* buffer, char** endptr, const int base, int* fla, int* nDbl, int* nInt, int* nBoo);

/////////////////////////////////////////////////////////////////
/// Disassembles the buffer that has been received through the IPC.
///
///\param buffer The buffer that contains the values to be parsed.
///\param flag The communication flag.
///\param nDbl The number of double values received.
///\param nInt The number of integer values received.
///\param nBoo The number of boolean values received.
///\param dblVal The array that stores the double values.
///\param intVal The array that stores the integer values.
///\param booVal The array that stores the boolean values.
///\return 0 if no error occurred.
int disassembleBuffer(const char* buffer, int* fla, int* nDbl, int* nInt, int* nBoo, double* curSimTim, double dblVal[], int intVal[], int booVal[]);

/////////////////////////////////////////////////////////////////////
/// Gets the port number for the BSD socket communication.
///
/// This method parses the xml file for the socket number.
/// \param docname Name of xml file.
/// \return the socket port number if successful, or -1 if an error occured.
int getsocketportnumber(const char* const docname);

/////////////////////////////////////////////////////////////////////
/// Gets the hostname for the BSD socket communication.
///
/// This method parses the xml file for the socket host name.
/// \param docname Name of xml file.
/// \param hostname The hostname will be written to this argument.
/// \return 0 if successful, or -1 if an error occured.
int getsockethost(const char* const docname, char* const hostname);

//////////////////////////////////////////////////////////////////
/// Returns the main version number of the client.
///
/// This method returns the version number. A negative return value
/// is used in a dummy dll to check in EnergyPlus whether the BCVTB
/// has been installed.
///
/// \return The main version number, or a negative value if an error occured.
int getmainversionnumber();

//////////////////////////////////////////////////////////////////
/// Establishes a connection to the socket.
///
/// This method establishes the client socket.
///
/// \param docname Name of xml file that contains the socket information.
/// \return The socket file descripter, or a negative value if an error occured.
int establishclientsocket(const char* const docname);

/////////////////////////////////////////////////////////////////
/// Writes data to the socket.
///
/// Clients can call this method to write data to the socket.
///\param sockfd Socket file descripter
///\param flaWri Communication flag to write to the socket stream.
///\param nDblWri Number of double values to write.
///\param nIntWri Number of integer values to write.
///\param nBooWri Number of boolean values to write.
///\param curSimTim Current simulation time in seconds.
///\param dblValWri Double values to write.
///\param intValWri Integer values to write.
///\param boolValWri Boolean values to write.
///\sa int establishclientsocket(uint16_t *portNo)
///\return The exit value of \c send, or a negative value if an error occured.
int writetosocket(const int* sockfd, const int* flaWri, const int* nDblWri, const int* nIntWri, const int* nBooWri, double* curSimTim,
                  double dblValWri[], int intValWri[], int booValWri[]);

/////////////////////////////////////////////////////////////////
/// Writes a message flag to the socket stream.
///
/// This method should be used by clients if they need to send
/// a flag to the BCVTB.
///
/// The flag flaWri is defined as follows:
/// +1: simulation reached end time.
/// -1: simulation terminates due to an (unspecified) error.
/// -10: simulation terminates due to error during initialization.
/// -20: simulation terminates due to error during time integration.
///
///\param sockfd Socket file descripter
///\param flaWri Flag to be sent to the BCVTB
int sendclientmessage(const int* sockfd, const int* flaWri);

/////////////////////////////////////////////////////////////////
/// Writes an error flag to the socket stream.
///
/// This method should be used by clients if they experience an
/// error and need to terminate the socket connection.
///
///\deprecated Use \c sendclientmessage instead
///
///\param sockfd Socket file descripter
///\param flaWri should be set to a negative value.
int sendclienterror(const int* sockfd, const int* flaWri);

/////////////////////////////////////////////////////////////////
/// Returns the required socket buffer length by reading from
/// the socket how many data it contains.
/// This method also set the global variable \c SERVER_VERSION
///
///\param sockfd Socket file descripter
///\return nCha The nunber of characters needed to store the buffer
int getRequiredReadBufferLength(const int* sockfd);

/////////////////////////////////////////////////////////////////
/// Returns the required socket buffer length.
///
///\param nDbl Number of double values to read or write.
///\param nInt Number of integer values to read or write.
///\param nBoo Number of boolean values to read or write.
int getrequiredbufferlength(const int nDbl, const int nInt, const int nBoo);

/////////////////////////////////////////////////////////////////
/// Reads data from the socket.
//
/// Clients can call this method to exchange data through the socket.
///
///\param sockfd Socket file descripter
///\param flaRea Communication flag read from the socket stream.
///\param nDblRea Number of double values to read.
///\param nIntRea Number of integer values to read.
///\param nBooRea Number of boolean values to read.
///\param curSimTim Current simulation time in seconds read from socket.
///\param dblValRea Double values read from socket.
///\param intValRea Integer values read from socket.
///\param boolValRea Boolean values read from socket.
///\sa int establishclientsocket(uint16_t *portNo)
int readfromsocket(const int* sockfd, int* flaRea, int* nDblRea, int* nIntRea, int* nBooRea, double* curSimTim, double dblValRea[], int intValRea[],
                   int booValRea[]);

/////////////////////////////////////////////////////////////////
/// Reads a character buffer from the socket.
///
/// This method is called by \c readfromsocket.
///
///\param sockfd The socket file descripter.
///\param buffer The buffer into which the values will be written.
///\param bufLen The buffer length prior to the call.
///\return The exit value of the \c read command.
int readbufferfromsocket(const int* sockfd, char* buffer, int* bufLen);

/////////////////////////////////////////////////////////////////
/// Exchanges data with the socket.
///
/// Clients can call this method to exchange data through the socket.
///\param sockfd Socket file descripter
///\param flaWri Communication flag to write to the socket stream.
///\param flaRea Communication flag read from the socket stream.
///\param nDblWri Number of double values to write.
///\param nIntWri Number of integer values to write.
///\param nBooWri Number of boolean values to write.
///\param nDblRea Number of double values to read.
///\param nIntRea Number of integer values to read.
///\param nBooRea Number of boolean values to read.
///\param simTimWri Current simulation time in seconds to write.
///\param dblValWri Double values to write.
///\param intValWri Integer values to write.
///\param boolValWri Boolean values to write.
///\param simTimRea Current simulation time in seconds read from socket.
///\param dblValRea Double values read from socket.
///\param intValRea Integer values read from socket.
///\param boolValRea Boolean values read from socket.
///\sa int establishclientsocket(uint16_t *portNo)
///\return The exit value of \c send or \c read, or a negative value if an error occured.
int exchangewithsocket(const int* sockfd, const int* flaWri, int* flaRea, const int* nDblWri, const int* nIntWri, const int* nBooWri, int* nDblRea,
                       int* nIntRea, int* nBooRea, double* simTimWri, double dblValWri[], int intValWri[], int booValWri[], double* simTimRea,
                       double dblValRea[], int intValRea[], int booValRea[]);

/////////////////////////////////////////////////////////////////
/// Exchanges data with the socket.
///
/// Clients can call this method to exchange data through the socket.
///\param sockfd Socket file descripter
///\param flaWri Communication flag to write to the socket stream.
///\param flaRea Communication flag read from the socket stream.
///\param nDblWri Number of double values to write.
///\param nDblRea Number of double values to read.
///\param simTimWri Current simulation time in seconds to write.
///\param dblValWri Double values to write.
///\param simTimRea Current simulation time in seconds read from socket.
///\param dblValRea Double values read from socket.
///\sa int establishclientsocket(uint16_t *portNo)
///\return The exit value of \c send or \c read, or a negative value if an error occured.
int exchangedoubleswithsocket(const int* sockfd, const int* flaWri, int* flaRea, const int* nDblWri, int* nDblRea, double* simTimWri,
                              double dblValWri[], double* simTimRea, double dblValRea[]);

/////////////////////////////////////////////////////////////////
/// Exchanges data with the socket.
///
/// Clients can call this method to exchange data through the socket.
/// This methods differs from the exchangedoubleswithsocket. It has
/// a flag that is used to set a global variable in readbufferfromsocket.
///\param sockfd Socket file descripter
///\param flaWri Communication flag to write to the socket stream.
///\param flaRea Communication flag read from the socket stream.
///\param nDblWri Number of double values to write.
///\param nDblRea Number of double values to read.
///\param simTimWri Current simulation time in seconds to write.
///\param dblValWri Double values to write.
///\param simTimRea Current simulation time in seconds read from socket.
///\param dblValRea Double values read from socket.
///\param flaexport Flag for FMUExport.
///\sa int establishclientsocket(uint16_t *portNo)
///\return The exit value of \c send or \c read, or a negative value if an error occured.
int exchangedoubleswithsocketFMU(const int* sockfd, const int* flaWri, int* flaRea, const int* nDblWri, int* nDblRea, double* simTimWri,
                                 double dblValWri[], double* simTimRea, double dblValRea[], const int* flaExport);

///////////////////////////////////////////////////////////
/// Closes the inter process communication socket.
///
///\param sockfd Socket file descripter.
///\return The return value of the \c close function.
int closeipc(int* sockfd);

#endif /* _UTILSOCKET_H_ */
