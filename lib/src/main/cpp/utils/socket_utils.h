/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef UTILS_SOCKET_UTILS_H_
#define UTILS_SOCKET_UTILS_H_

#include <sys/socket.h>
#include <sys/un.h>

namespace profiler {

// Prefix used by gRPC to designate a Unix socket address.
const char* const kGrpcUnixSocketAddrPrefix = "unix:";

// Populates Unix socket address structure |addr_un| and socket lenth |addr_len|
// for a given Unix socket's |name|.
// If |name| starts with '@', replaces it by '\0' so that when |addr_un| is
// passed to bind(), the socket will be treated as an abstract domain socket.
void SetUnixSocketAddr(const char* name, struct sockaddr_un* addr_un,
                       socklen_t* addr_len);

// Creates a Unix socket and assign the |address| to it. Returns the socket's
// file descriptor. Exits the calling process on failures.
// Easy-to-use wrapper of socket() and bind().
int CreateUnixSocket(const char* address);

// Makes a socket accept incoming connection requests using accept().
// Returns the socket's |fd| on success. Exits the calling process on failures.
// Easy-to-use wrapper of listen().
int ListenToSocket(int fd);

// Convenient method to connect to a socket at |destination| and send data
// to it with retries.
int ConnectAndSendDataToSocket(const char* destination, int send_fd,
                               const char* data, int retry_count, int to_usec);

// Convenient method to accept connections and receive data from a socket.
int AcceptAndGetDataFromSocket(int socket_fd, int* receive_fd, char* buffer,
                               int length, int to_sec, int to_usec);

}  // namespace profiler

#endif  // UTILS_SOCKET_UTILS_H_
