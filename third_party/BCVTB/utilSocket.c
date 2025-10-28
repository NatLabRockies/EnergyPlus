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
/// \file   utilSocket.c
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
/// \version $Id: utilSocket.c 55724 2009-09-16 17:51:58Z mwetter $
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
#include <fcntl.h>
#ifdef _WIN32
#  include <io.h>
#else
#  include <unistd.h>
#endif
#include "utilSocket.h"
#include "utilXml.h"

FILE* f1 = NULL;
int REQUIRED_READ_LENGTH = 0;
int REQUIRED_WRITE_LENGTH = 0;
int SERVER_VERSION = 0;

// Global variable to check for FMUExport case
int FMUEXPORT = 0;
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
int save_append(char** buffer, const char* toAdd, size_t* bufLen) {
  const size_t size = 1024;
  const size_t nNewCha = strlen(toAdd);
  const size_t nBufCha = strlen(*buffer);
  // reallocate memory if needed
  if (*bufLen < nNewCha + nBufCha + 1) {
    *bufLen = *bufLen + size * (((nNewCha + nBufCha) / size) + 1);
    *buffer = realloc(*buffer, *bufLen);
    if (*buffer == NULL) {
      perror("Realloc failed in save_append.");
#ifdef NDEBUG
      fprintf(f1, "Realloc failed in save_append.\n");
#endif
      return EXIT_FAILURE;
    }
  }
  // append toAdd to buffer
  strcpy(*buffer + strlen(*buffer), toAdd);
  return 0;
}

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
                   size_t* bufLen) {
  int i;
  int retVal;
  char temCha[1024];  // temporary character array
  memset((char*)*buffer, '\0', *bufLen);
  // Set up how many values will be in buffer
  // This is an internally used version number to make update
  // of the format possible later without braking old versions
  retVal = snprintf(temCha, sizeof temCha, "%d ", MAINVERSION);
  if (retVal < 0) return -1;
  retVal = save_append(buffer, temCha, bufLen);
  if (retVal != 0) return retVal;
  retVal = snprintf(temCha, sizeof temCha, "%d ", flag);
  if (retVal < 0) return -1;
  retVal = save_append(buffer, temCha, bufLen);
  if (retVal != 0) return retVal;
  if (flag == 0) {
    // Only process data if the flag is zero.
    retVal = snprintf(temCha, sizeof temCha, "%d ", nDbl);
    if (retVal < 0) return -1;
    retVal = save_append(buffer, temCha, bufLen);
    if (retVal != 0) return retVal;
    retVal = snprintf(temCha, sizeof temCha, "%d ", nInt);
    if (retVal < 0) return -1;
    retVal = save_append(buffer, temCha, bufLen);
    if (retVal != 0) return retVal;
    retVal = snprintf(temCha, sizeof temCha, "%d ", nBoo);
    if (retVal < 0) return -1;
    retVal = save_append(buffer, temCha, bufLen);
    if (retVal != 0) return retVal;
    retVal = snprintf(temCha, sizeof temCha, "%20.15e ", curSimTim);
    if (retVal < 0) return -1;
    retVal = save_append(buffer, temCha, bufLen);
    if (retVal != 0) return retVal;
    // add values to buffer
    for (i = 0; i < nDbl; i++) {
      retVal = snprintf(temCha, sizeof temCha, "%20.15e ", dblVal[i]);
      if (retVal < 0) return -1;
      retVal = save_append(buffer, temCha, bufLen);
      if (retVal != 0) return retVal;
    }
    for (i = 0; i < nInt; i++) {
      retVal = snprintf(temCha, sizeof temCha, "%d ", intVal[i]);
      if (retVal < 0) return -1;
      retVal = save_append(buffer, temCha, bufLen);
      if (retVal != 0) return retVal;
    }
    for (i = 0; i < nBoo; i++) {
      retVal = snprintf(temCha, sizeof temCha, "%d ", booVal[i]);
      if (retVal < 0) return -1;
      retVal = save_append(buffer, temCha, bufLen);
      if (retVal != 0) return retVal;
    }
  }
  // For the Java server to read the line, the line
  // needs to be terminated with '\n'
  retVal = snprintf(temCha, sizeof temCha, "\n");
  if (retVal < 0) return -1;
  retVal = save_append(buffer, temCha, bufLen);
  if (retVal != 0) return retVal;
  // No error, return 0
  return 0;
}

/////////////////////////////////////////////////////////////////
/// Gets an integer and does the required error checking.
///
///\param nptr Pointer to character buffer that contains the number.
///\param endptr After return, this variable contains a pointer to the
///            character after the last character of the number.
///\param base Base for the integer.
///\param The value contained in the character buffer.
///\return 0 if no error occurred.
int getIntCheckError(const char* nptr, char** endptr, const int base, int* val) {
  errno = 0;  // must reset errno to 0
  // Parse integer
  *val = strtol(nptr, endptr, base);
  /////////////////////////////////////////////////////////////////
  // do error checking
  if ((errno == ERANGE) || (errno != 0 && *val == 0)) {
    perror("strtol caused error.");
    if (strlen(nptr) < 1) {
      fprintf(stderr, "strtol() was called with a string of length less than 1. This can occur when no data is read.\n");
    } else {
      fprintf(stderr, "strtol was called with strtol(%s, %s, %d)\n", nptr, *endptr, base);
    }
    return EXIT_FAILURE;
  }
  if (*endptr == nptr) {
    //FMU Export - added the check to skip the error message for FMUExport
    if (FMUEXPORT != 1) {
      fprintf(stderr, "Error: No digits were found in getIntCheckError.\n");
      fprintf(stderr, "Further characters after number: %s\n", *endptr);
      fprintf(stderr, "Sending EXIT_FAILURE = : %d\n", EXIT_FAILURE);
    }
    return EXIT_FAILURE;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////
/// Gets a double and does the required error checking.
///
///\param nptr Pointer to character buffer that contains the number.
///\param endptr After return, this variable contains a pointer to the
///            character after the last character of the number.
///\param The value contained in the character buffer.
///\return 0 if no error occurred.
int getDoubleCheckError(const char* nptr, char** endptr, double* val) {
  errno = 0;  // must reset errno to 0
  *val = strtod(nptr, endptr);
  /////////////////////////////////////////////////////////////////
  // do error checking
  if ((errno == ERANGE && (*val == HUGE_VAL || *val == -HUGE_VAL)) || (errno != 0 && *val == 0)) {
    perror("strtod caused error.");
    return EXIT_FAILURE;
  }
  if (*endptr == nptr) {
    fprintf(stderr, "Error: No digits were found in getDoubleCheckError.\n");
    fprintf(stderr, "Further characters after number: %s\n", *endptr);
    fprintf(stderr, "Sending EXIT_FAILURE = : %d\n", EXIT_FAILURE);
    return EXIT_FAILURE;
  }
  return 0;
}

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
int disassembleHeaderBuffer(const char* buffer, char** endptr, const int base, int* fla, int* nDbl, int* nInt, int* nBoo) {
  int retVal;  // return value
  //////////////////////////////////////////////////////
  // Get first few integers to set up dictionaray
  // set number of received values to zero to ensure that
  // if retVal != 0, we have the values initialized
  *nDbl = 0;
  *nInt = 0;
  *nBoo = 0;
  // version number
  retVal = getIntCheckError(buffer, endptr, base, &SERVER_VERSION);
  if (retVal) return retVal;
  //////////////////////////////////////////////////////
  // communication flag
  retVal = getIntCheckError(*endptr, endptr, base, fla);
  if (retVal) return retVal;
  // number of doubles, integers and booleans
  retVal = getIntCheckError(*endptr, endptr, base, nDbl);
  if (retVal) return retVal;
  retVal = getIntCheckError(*endptr, endptr, base, nInt);
  if (retVal) return retVal;
  retVal = getIntCheckError(*endptr, endptr, base, nBoo);
  return retVal;
}

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
int disassembleBuffer(const char* buffer, int* fla, int* nDbl, int* nInt, int* nBoo, double* curSimTim, double dblVal[], int intVal[], int booVal[]) {
  int i;
  int retVal;  // return value
  const int base = 10;
  char* endptr = 0;
  retVal = disassembleHeaderBuffer(buffer, &endptr, base, fla, nDbl, nInt, nBoo);
  if (retVal) {
#ifdef NDEBUG
    fprintf(f1, "Error while disassembling the header of the buffer.\n");
#endif
    return retVal;
  }

  *curSimTim = 0;
  // current simulation time
  retVal = getDoubleCheckError(endptr, &endptr, curSimTim);
  if (retVal) {
#ifdef NDEBUG
    fprintf(f1, "Error while getting the current simulation time.\n");
#endif
    return retVal;
  }
  //////////////////////////////////////////////////////
  // Get doubles
  for (i = 0; i < *nDbl; i++) {
    retVal = getDoubleCheckError(endptr, &endptr, &dblVal[i]);
    if (retVal) {
#ifdef NDEBUG
      fprintf(f1, "Error while getting double %d of %d.\n", i, *nDbl);
#endif
      return retVal;
    }
  }
  //////////////////////////////////////////////////////
  // Get integers
  for (i = 0; i < *nInt; i++) {
    retVal = getIntCheckError(endptr, &endptr, base, &intVal[i]);
    if (retVal) {
#ifdef NDEBUG
      fprintf(f1, "Error while getting integer %d of %d.\n", i, *nInt);
#endif
      return retVal;
    }
  }
  //////////////////////////////////////////////////////
  // Get boolean
  for (i = 0; i < *nBoo; i++) {
    retVal = getIntCheckError(endptr, &endptr, base, &booVal[i]);
    if (retVal) {
#ifdef NDEBUG
      fprintf(f1, "Error while getting boolean %d of %d.\n", i, *nBoo);
#endif
      return retVal;
    }
  }
  return retVal;
}

/////////////////////////////////////////////////////////////////////
/// Gets the port number for the BSD socket communication.
///
/// This method parses the xml file for the socket number.
/// \param docname Name of xml file.
/// \return the socket port number if successful, or -1 if an error occured.
int getsocketportnumber(const char* const docname) {
  int retVal;
  char* xPat = "//ipc/socket[@port]";
  char* res;
  size_t i;
  res = malloc(BUFFER_LENGTH);
  if (res == NULL) {
    perror("malloc failed in getsocketportnumber.");
#ifdef NDEBUG
    fprintf(f1, "malloc failed in getsocketportnumber.\n");
#endif
    return -1;
  }
  if (0 == getxmlvalue(docname, xPat, res, &i, BUFFER_LENGTH))
    retVal = atoi((char*)res);
  else
    retVal = -1;
  free(res);
  return retVal;
}

//////////////////////////////////////////////////////////////////
/// Returns the version number of the client.
///
/// This method returns the version number of the client.
/// A negative return value
/// is used in a dummy dll to check in EnergyPlus whether the BCVTB
/// has been installed.
///
/// \return The main version number, or a negative value if an error occured.
int getmainversionnumber() {
  return MAINVERSION;
}

/////////////////////////////////////////////////////////////////////
/// Gets the hostname for the BSD socket communication.
///
/// This method parses the xml file for the socket host name.
/// \param docname Name of xml file.
/// \param hostname The hostname will be written to this argument.
/// \return 0 if successful, or -1 if an error occured.
int getsockethost(const char* const docname, char* const hostname) {
  char* xPat = "//ipc/socket[@hostname]";
  size_t i;
  int r = getxmlvalue(docname, xPat, hostname, &i, BUFFER_LENGTH);
  return r;
}

//////////////////////////////////////////////////////////////////
/// Establishes a connection to the socket.
///
/// This method establishes the client socket.
///
/// \param docname Name of xml file that contains the socket information.
/// \return The socket file descripter, or a negative value if an error occured.
int establishclientsocket(const char* const docname) {
  int portNo, retVal, sockfd;
  char* hostname;
  char* serverIP;
#ifdef _MSC_VER /************* Windows specific code ********/
  struct hostent* FAR server;
  WSADATA wsaData;
  WORD wVersionRequested;
#else /************* End of Windows specific code *******/
  struct hostent* server;
#endif
  struct sockaddr_in serAdd;
  const int arg = 1;  // 1: true
#ifdef NDEBUG
  if (f1 == NULL) f1 = fopen("utilSocket.log", "w");
  if (f1 == NULL) {
    fprintf(stderr, "Could not open file '%s'\n", "utilSocket.log");
    return -1;
  } else
    fprintf(f1, "utilSocket: Establishing socket based on file %s.\n", docname);
#endif
  hostname = malloc(BUFFER_LENGTH);
  if (hostname == NULL) {
    perror("malloc failed in establishclientsocket.");
#ifdef NDEBUG
    fprintf(f1, "malloc failed in establishclientsocket.\n");
#endif
    return -2;
  }

  //////////////////////////////////////////////////////
  // get the socket port number
#ifdef NDEBUG
  fprintf(f1, "Getting socket port number.\n");
#endif
  portNo = getsocketportnumber(docname);
#ifdef NDEBUG
  fprintf(f1, "Received socket port number %d.\n", portNo);
#endif
  if (portNo < 0) {
    fprintf(stderr, "Error: Could not obtain socket port number. Return value = %d.\n", portNo);
#ifdef NDEBUG
    fprintf(f1, "Error: Could not obtain socket port number. Return value = %d.\n", portNo);
#endif
    free(hostname);
    return portNo;
  }
#ifdef NDEBUG
  fprintf(f1, "Socket port number = %d.\n", portNo);
#endif
  //////////////////////////////////////////////////////
  // get the socket host name
  retVal = getsockethost(docname, hostname);
  if (retVal < 0) {
#ifdef NDEBUG
    fprintf(f1, "Error: Could not obtain socket hostname. Return value = %d.\n", retVal);
#endif
    return retVal;
  }
  // open socket
#ifdef _MSC_VER /************* Windows specific code ********/
  wVersionRequested = MAKEWORD(2, 2);
  retVal = WSAStartup(wVersionRequested, &wsaData);
  if (retVal != 0) {
    /* Tell the user that we could not find a usable */
    /* WinSock DLL.                                  */
#  ifdef NDEBUG
    fprintf(f1, "Error: Could not find a usable WinSock DLL.\n");
    fprintf(f1, "WSAGetLastError = %d\n", WSAGetLastError());
#  endif
    return -1;
  }
  /* Confirm that the WinSock DLL supports 2.2.*/
  /* Note that if the DLL supports versions greater    */
  /* than 2.2 in addition to 2.2, it will still return */
  /* 2.2 in wVersion since that is the version we      */
  /* requested.                                        */
  if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
    /* Tell the user that we could not find a usable */
    /* WinSock DLL.                                  */
#  ifdef NDEBUG
    fprintf(f1, "Error: Could not find a usable WinSock DLL for requested version.\n");
#  endif
    WSACleanup();
    return -1;
  }
#  ifdef NDEBUG
  fprintf(f1, "WinSock DLL is acceptable.\n");
#  endif
  /* The WinSock DLL is acceptable. Proceed. */
#endif /************* End of Windows specific code *******/
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
#ifdef NDEBUG
    fprintf(f1, "Error opening socket. sockfd = %d.\n", sockfd);
#endif
    return sockfd;
  }
#ifdef NDEBUG
  fprintf(f1, "Socket opened, sockfd = %d.\n", sockfd);
#endif

  // See
  // http://msdn.microsoft.com/en-us/library/ms740476%28VS.85%29.aspx
  // for increasing the buffer size.
  // Note that getsockopt must be called to see if the increased buffer
  // size was provided by the implementation.
  if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char*)&arg, sizeof(arg)) != 0) {
#ifdef NDEBUG
    fprintf(f1, "Error setting socket option keep alive.\n");
    fprintf(f1, "Error flag errno = %d.\n", errno);
#endif
    return -1;
  }

  if (sockfd < 0) {
#ifdef NDEBUG
    fprintf(f1, "Error opening socket\n");
#endif
    return sockfd;
  }
  //////////////////////////////////////////////////////
  // establish server address
  server = gethostbyname(hostname);
  free(hostname);
  if (server == NULL) {
#ifdef NDEBUG
    fprintf(f1, "Error, no such host\n");
#endif
    return -1;
  }
  serverIP = inet_ntoa(*(struct in_addr*)*server->h_addr_list);
  memset((char*)&serAdd, '\0', sizeof(serAdd));
  serAdd.sin_family = AF_INET;
  serAdd.sin_addr.s_addr = inet_addr(serverIP);
  serAdd.sin_port = htons(portNo);
  //////////////////////////////////////////////////////
  // establish connection
  //retVal = connect(sockfd, (void *)&serAdd, sizeof(serAdd));
  retVal = connect(sockfd, (const struct sockaddr*)&serAdd, sizeof(serAdd));
  if (retVal < 0) {
#ifdef _MSC_VER
    fprintf(stderr, "Error when connecting to socket: WSAGetLastError = %d\n", WSAGetLastError());
#else
    fprintf(stderr, "Error when connecting to socket: %s\n", strerror(errno));
#endif
#ifdef NDEBUG
#  ifdef _MSC_VER
    fprintf(f1, "Error when connecting to socket: WSAGetLastError = %d\n", WSAGetLastError());
#  else
    fprintf(f1, "Error when connecting to socket: %s\n", strerror(errno));
#  endif
#endif
    return retVal;
  }
  // Set flags that indicate that we have not yet written to
  // or read from the buffer
  REQUIRED_READ_LENGTH = 0;
  REQUIRED_WRITE_LENGTH = 0;
  return sockfd;
}

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
                  double dblValWri[], int intValWri[], int booValWri[]) {
  int retVal;
  // buffer used to exchange data
  char* buffer;
  size_t bufLen = REQUIRED_WRITE_LENGTH;

#ifdef NDEBUG
  if (f1 == NULL)  // open file
    f1 = fopen("utilSocket.log", "w");
  if (f1 == NULL) {
    fprintf(stderr, "Cannot open file %s\n", "utilSocket.log");
    return -1;
  }
#endif

  /////////////////////////////////////////////////////
  // make sure that the socketFD is valid
  if (*sockfd < 0) {
    fprintf(stderr, "Error: Called write to socket with negative socket number.\n");
    fprintf(stderr, "       sockfd : %d\n", *sockfd);
#ifdef NDEBUG
    fprintf(f1, "Error: Called write to socket with negative socket number.\n");
    fprintf(f1, "       sockfd : %d\n", *sockfd);
    fflush(f1);
#endif
    return -1;  // return a negative value in case of an error
  }

  /////////////////////////////////////////////////////
  // allocate storage for buffer
#ifdef NDEBUG
  fprintf(f1, "Assembling buffer.\n");
#endif
  buffer = malloc(bufLen);
  if (buffer == NULL) {
    perror("malloc failed in writetosocket.");
#ifdef NDEBUG
    fprintf(f1, "malloc failed in writetosocket.\n");
#endif
    return -1;
  }
  //////////////////////////////////////////////////////
  // copy arguments to buffer
  retVal = assembleBuffer(*flaWri, *nDblWri, *nIntWri, *nBooWri, *curSimTim, dblValWri, intValWri, booValWri, &buffer, &bufLen);

  if (retVal != 0) {
    fprintf(stderr, "Error: Failed to allocate memory for buffer before writing to socket.\n");
    fprintf(stderr, "       retVal : %d\n", retVal);
    fprintf(stderr, "       Message: %s\n", strerror(errno));
#ifdef NDEBUG
    fprintf(f1, "Error: Failed to allocate memory for buffer before writing to socket.\n");
    fprintf(f1, "       retVal : %d\n", retVal);
    fprintf(f1, "       Message: %s\n", strerror(errno));
    fflush(f1);
#endif
    free(buffer);
    return -1;  // return a negative value in case of an error
  }
  //////////////////////////////////////////////////////
  // write to socket
#ifdef NDEBUG
  fprintf(f1, "Write to socket with fd = %d\n", *sockfd);
  fprintf(f1, "Buffer        = %s\n", buffer);
#endif

#ifdef _MSC_VER
  retVal = send(*sockfd, buffer, (int)strlen(buffer), 0);
#else
  retVal = write(*sockfd, buffer, strlen(buffer));
#endif

#ifdef NDEBUG
  if (retVal >= 0)
    fprintf(f1, "Wrote %d characters to socket.\n", retVal);
  else
    fprintf(f1, "Error writing to socket: Return value = %d.\n", retVal);
#endif
  if (retVal < 0) {
#ifdef NDEBUG
#  ifdef _MSC_VER
    fprintf(f1, "Error writing to socket: WSAGetLastError = %d\n", WSAGetLastError());
#  else
    fprintf(f1, "Error writing to socket: %s\n", strerror(errno));
#  endif
    fflush(f1);
#endif
  }
  free(buffer);
  return retVal;
}

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
///\deprecated Use \c sendclientmessage instead
///
///\param sockfd Socket file descripter
///\param flaWri Flag to be sent to the BCVTB
int sendclientmessage(const int* sockfd, const int* flaWri) {
  int zI = 0;
  int retVal = 0;
  double zD = 0;
  int bufLen = HEADER_LENGTH;
  char inpBuf[HEADER_LENGTH];
  memset(inpBuf, '\0', HEADER_LENGTH);

  if (*sockfd >= 0) {
    retVal = writetosocket(sockfd, flaWri, &zI, &zI, &zI, &zD, NULL, NULL, NULL);
#ifdef NDEBUG
    fprintf(f1, "sendclientmessage wrote flag %d, return value = %d.\n", *flaWri, retVal);
#endif
    if (retVal >= 0) {
      // No error. Wait for acknowledgement. This is needed on Windows for E+.
      // Otherwise, E+ sometimes terminates and breaks the socket connection before
      // Ptolemy read the message.
      retVal = readbufferfromsocket(sockfd, inpBuf, &bufLen);
    }
  } else
    retVal = 0;
  return retVal;
}

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
int sendclienterror(const int* sockfd, const int* flaWri) {
  return sendclientmessage(sockfd, flaWri);
}

/////////////////////////////////////////////////////////////////
/// Returns the required socket buffer length by reading from
/// the socket how many data it contains.
/// This method also set the global variable \c SERVER_VERSION
///
///\param sockfd Socket file descripter
///\return nCha The nunber of characters needed to store the buffer
int getRequiredReadBufferLength(const int* sockfd) {
  int retVal;
  char buffer[HEADER_LENGTH];
  const int base = 10;
  char* endptr = NULL;
  int fla = 0;
  int nDbl = 0;
  int nInt = 0;
  int nBoo = 0;

  memset(buffer, '\0', HEADER_LENGTH);
#ifdef _MSC_VER
  // MSG_WAITALL is not in the winsock2.h file, at least not on my system...
#  define MSG_WAITALL 0x8  // do not complete until packet is completely filled
  retVal = recv(*sockfd, buffer, HEADER_LENGTH, MSG_PEEK);
#else
  retVal = recv(*sockfd, buffer, HEADER_LENGTH, MSG_PEEK);
#endif
  if (retVal < 1) {
    perror("Failed to peek at socket.");
    return retVal;
  }
  retVal = disassembleHeaderBuffer(buffer, &endptr, base, &fla, &nDbl, &nInt, &nBoo);
  if (retVal < 0) {
    perror("Failed to disassemble header buffer.");
    return retVal;
  }
  return getrequiredbufferlength(nDbl, nInt, nBoo);
}
/////////////////////////////////////////////////////////////////
/// Returns the required socket buffer length.
///
///\param nDbl Number of double values to read or write.
///\param nInt Number of integer values to read or write.
///\param nBoo Number of boolean values to read or write.
///\return nCha The nunber of characters needed to store the buffer
int getrequiredbufferlength(const int nDbl, const int nInt, const int nBoo) {
  int retVal;
  if ((nInt > 0) || (nBoo > 0)) {
    fprintf(stderr, "Error: Integers and booleans are currently not\n");
    fprintf(stderr, "       implemented in utilSocket:getrequiredbufferlength.\n");
    fprintf(stderr, "       Received %d integers and %d boolean.\n", nInt, nBoo);
    retVal = -1;
  } else {
    // Header has 4 integers and the current simulation time.
    // Each double has 21 characters plus one space behind it.
    // The last number is for the EOL character.
    retVal = HEADER_LENGTH + 21 + (21 + 1) * nDbl + 1;
  }
  return retVal;
}

/////////////////////////////////////////////////////////////////
/// Reads data from the socket.
///
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
                   int booValRea[]) {
  int retVal;
  char* inpBuf;
  /////////////////////////////////////////////////////
  // make sure that the socketFD is valid
  if (*sockfd < 0) {
    fprintf(stderr, "Error: Called read from socket with negative socket number.\n");
    fprintf(stderr, "       sockfd : %d\n", *sockfd);
#ifdef NDEBUG
    fprintf(f1, "Error: Called read from socket with negative socket number.\n");
    fprintf(f1, "       sockfd : %d\n", *sockfd);
    fflush(f1);
#endif
    return -1;  // return a negative value in case of an error
  }
  // In the first call, set the socket buffer length
  // This is done here since we know how many data we need to read.
  if (REQUIRED_READ_LENGTH < 1) {
    // Peak into the socket message to see how many data we need to read
    // This is required to assign enough storage for the buffer.
    // This call also sets the version number SERVER_VERSION
    // that is sent by the server.
    REQUIRED_READ_LENGTH = getRequiredReadBufferLength(sockfd);
    if (REQUIRED_READ_LENGTH <= 0) return -1;
    // Increase the buffer length for the socket
    //    retVal = setrequiredbufferlength(*sockfd, REQUIRED_READ_LENGTH, SO_RCVBUF);
    //    if ( retVal != 0 )
    //      return retVal;
  }
  // Increase the buffer that is used to store the data
  inpBuf = malloc(REQUIRED_READ_LENGTH * sizeof(char));
  if (inpBuf == NULL) {
    perror("malloc failed in readfromsocket.");
#ifdef NDEBUG
    fprintf(f1, "malloc failed in readfromsocket.\n");
#endif
    free(inpBuf);
    return -1;
  }

  memset(inpBuf, '\0', REQUIRED_READ_LENGTH);
  retVal = readbufferfromsocket(sockfd, inpBuf, &REQUIRED_READ_LENGTH);
  if (retVal < 0) {
#ifdef NDEBUG
#  ifdef _MSC_VER
    fprintf(f1, "Error reading: WSAGetLastError = %d\n", WSAGetLastError());
#  else
    fprintf(f1, "Error reading: %s\n", strerror(errno));
#  endif
    fflush(f1);
#endif
    free(inpBuf);
    return retVal;
  }
  //////////////////////////////////////////////////////
  // disassemble buffer and store values in function argument
  retVal = disassembleBuffer(inpBuf, flaRea, nDblRea, nIntRea, nBooRea, curSimTim, dblValRea, intValRea, booValRea);
#ifdef NDEBUG
  fprintf(f1, "Disassembled buffer.\n");
#endif
  free(inpBuf);
  return retVal;
}

/////////////////////////////////////////////////////////////////
/// Reads a character buffer from the socket.
///
/// This method is called by \c readfromsocket.
///
///\param sockfd The socket file descripter.
///\param buffer The buffer into which the values will be written.
///\param bufLen The buffer length prior to the call.
///\return The exit value of the \c read command.
int readbufferfromsocket(const int* sockfd, char* buffer, int* bufLen) {
  (void)bufLen;
  int retVal;
  int reachedEnd = 0;
  // The number 8192 needs to be the same as in Server.java
  int maxChaRea = 8192;
  int chaSta = 0;
  // Loop until we read the '\n' character
  do {
#ifdef _MSC_VER
    // MSG_WAITALL is not in the winsock2.h file, at least not on my system...
#  define MSG_WAITALL 0x8 /* do not complete until packet is completely filled */
    retVal = recv(*sockfd, buffer, *bufLen, 0);
#else
    retVal = read(*sockfd, &buffer[chaSta], maxChaRea);
#endif

#ifdef NDEBUG
    fprintf(f1, "In readbufferfromsocket: Read %d chars, maximum is %d.\n", retVal, REQUIRED_READ_LENGTH);
#endif
    if (retVal == 0) {
      fprintf(stderr, "Error: The server closed the socket while the client was reading.\n");
      return -1;
    }
    if (retVal < 0) {
      fprintf(stderr, "Error: Unspecified error when reading from socket.\n");
      return retVal;
    }

    // Check if we received '\n', in which case we finish the reading
    if (NULL == memchr(&buffer[chaSta], '\n', retVal)) {
      chaSta += retVal;
      if (SERVER_VERSION == 1) {
        fprintf(stderr, "Error: This version of the socket interface cannot process such large data.\n");
        fprintf(stderr, "       You will need to update to BCVTB 0.8.0 or higher.\n");
#ifdef NDEBUG
        fprintf(f1, "Error: This version of the socket interface cannot process such large data.\n");
        fprintf(f1, "       You will need to update to BCVTB 0.8.0 or higher.\n");
#endif
        return -1;
      }
    } else {
      reachedEnd = 1;  // found the end of the string
    }
  } while (reachedEnd == 0);
  return retVal;
}

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
                       double dblValRea[], int intValRea[], int booValRea[]) {
  int retVal;
#ifdef NDEBUG
  if (f1 == NULL) f1 = fopen("utilSocket.log", "w");
  if (f1 == NULL) {
    fprintf(stderr, "Cannot open file %s\n", "utilSocket.log");
    return -1;
  }
  rewind(f1);
  fprintf(f1, "*** BCVTB client log file.\n");
  fprintf(f1, "*************************.\n");
  fprintf(f1, "Writing to socket at time = %e\n", *simTimWri);
#endif

  // In the first call, set the socket buffer length
  // This is done here since we know how many data we need to send.
  if (REQUIRED_WRITE_LENGTH < 1) {
    REQUIRED_WRITE_LENGTH = getrequiredbufferlength(*nDblWri, *nIntWri, *nBooWri);
    if (REQUIRED_WRITE_LENGTH <= 0) return -1;
    // Increase the buffer length for the socket
    //   retVal = setrequiredbufferlength(*sockfd, REQUIRED_WRITE_LENGTH, SO_SNDBUF);
    //   if ( retVal != 0 )
    //     return retVal;
  }

  retVal = writetosocket(sockfd, flaWri, nDblWri, nIntWri, nBooWri, simTimWri, dblValWri, intValWri, booValWri);
  if (retVal >= 0) {
#ifdef NDEBUG
    fprintf(f1, "Reading from socket.\n");
    fflush(f1);
#endif
    retVal = readfromsocket(sockfd, flaRea, nDblRea, nIntRea, nBooRea, simTimRea, dblValRea, intValRea, booValRea);
  }
#ifdef NDEBUG
  fprintf(f1, "Finished exchanging data with socket: simTimRea=%e, flag=%d.\n", *simTimRea, retVal);
  fflush(f1);
#endif
  return retVal;
}

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
                              double dblValWri[], double* simTimRea, double dblValRea[]) {
  const int zer = 0;
  int nIntRea = 0;
  int nBooRea = 0;
  int intValRea[1];  // allocate array of non-zero size
  int booValRea[1];  // allocate array of non-zero size
  return exchangewithsocket(sockfd, flaWri, flaRea, nDblWri, &zer, &zer, nDblRea, &nIntRea, &nBooRea, simTimWri, dblValWri, NULL, NULL, simTimRea,
                            dblValRea, intValRea, booValRea);
}

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
                                 double dblValWri[], double* simTimRea, double dblValRea[], const int* flaExport) {
  const int zer = 0;
  int nIntRea = 0;
  int nBooRea = 0;
  int intValRea[1];  // allocate array of non-zero size
  int booValRea[1];  // allocate array of non-zero size
  if (*flaExport == 1) {
    FMUEXPORT = 1;
  }
  return exchangewithsocket(sockfd, flaWri, flaRea, nDblWri, &zer, &zer, nDblRea, &nIntRea, &nBooRea, simTimWri, dblValWri, NULL, NULL, simTimRea,
                            dblValRea, intValRea, booValRea);
}

///////////////////////////////////////////////////////////
/// Closes the inter process communication socket.
///
///\param sockfd Socket file descripter.
///\return The return value of the \c close function.
int closeipc(int* sockfd) {
#ifdef _MSC_VER
  return closesocket(*sockfd);
#else
  return close(*sockfd);
#endif
}
