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
#include "utils/socket_utils.h"

#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

namespace profiler {

// Field 'sun_path' in struct sockaddr_un is 108-char large.
const int kSunPathLength = 108;

namespace {

// Write the data in |ptr| to |fd| using the sendmsg API. If |sendfd| is
// specified (> -1), it will be included as ancillary data in the message.
ssize_t write_to_fd(int fd, void* ptr, size_t nbytes, int sendfd) {
  struct msghdr msg;
  struct iovec iov[1];
  memset(&msg, '\0', sizeof(msg));

  union {
    struct cmsghdr cm;
    char control[CMSG_SPACE(sizeof(int))];
  } control_un;

  if (sendfd >= 0) {
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    struct cmsghdr* cmptr;
    cmptr = CMSG_FIRSTHDR(&msg);
    cmptr->cmsg_len = CMSG_LEN(sizeof(int));
    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type = SCM_RIGHTS;
    *((int*)CMSG_DATA(cmptr)) = sendfd;
  }

  iov[0].iov_base = ptr;
  iov[0].iov_len = nbytes;
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;

  return (sendmsg(fd, &msg, 0));
}

// Helper method to read any data available in |fd| to |ptr| using the recvmsg
// API. If file descriptor ancillar data is available, it will be returned via
// |recvfd|.
ssize_t read_from_fd(int fd, void* ptr, size_t nbytes, int* recvfd) {
  struct msghdr msg;
  struct iovec iov[1];
  ssize_t n;
  memset(&msg, '\0', sizeof(msg));

  union {
    struct cmsghdr cm;
    char control[CMSG_SPACE(sizeof(int))];
  } control_un;
  struct cmsghdr* cmptr;
  msg.msg_control = control_un.control;
  msg.msg_controllen = sizeof(control_un.control);

  iov[0].iov_base = ptr;
  iov[0].iov_len = nbytes;
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;

  if ((n = recvmsg(fd, &msg, MSG_WAITALL)) <= 0) return (n);

  if ((cmptr = CMSG_FIRSTHDR(&msg)) != nullptr &&
      cmptr->cmsg_len == CMSG_LEN(sizeof(int))) {
    if (cmptr->cmsg_level != SOL_SOCKET) {
      printf("control level != SOL_SOCKET");
      exit(-1);
    }
    if (cmptr->cmsg_type != SCM_RIGHTS) {
      printf("control type != SCM_RIGHTS");
      exit(-1);
    }
    *recvfd = *((int*)CMSG_DATA(cmptr));
  } else
    *recvfd = -1; /* descriptor was not passed */

  return (n);
}

}  // namespace

void SetUnixSocketAddr(const char* name, struct sockaddr_un* addr_un,
                       socklen_t* addr_len) {
  memset(addr_un, 0, sizeof(*addr_un));
  addr_un->sun_family = AF_UNIX;
  // Field 'sun_path' in struct sockaddr_un is 108-char large.
  size_t length = strnlen(name, kSunPathLength);
  strncpy(addr_un->sun_path, name, kSunPathLength);
  *addr_len = offsetof(struct sockaddr_un, sun_path) + length;

  if (addr_un->sun_path[0] == '@') {
    addr_un->sun_path[0] = '\0';
  }
}

int CreateUnixSocket(const char* address) {
  int fd = -1;
  struct sockaddr_un addr_un;
  socklen_t addr_len;

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(-1);
  }

  SetUnixSocketAddr(address, &addr_un, &addr_len);
  if (bind(fd, (struct sockaddr*)&addr_un, addr_len) == -1) {
    perror("bind error");
    exit(-1);
  }
  return fd;
}

int ListenToSocket(int fd) {
  if (listen(fd, 5) == -1) {
    perror("listen error");
    exit(-1);
  }
  return fd;
}

int ConnectAndSendDataToSocket(const char* destination, int send_fd,
                               const char* data, int retry_count, int to_usec) {
  int through_fd;
  if ((through_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(-1);
  }

  struct sockaddr_un addr_un;
  socklen_t addr_len;
  SetUnixSocketAddr(destination, &addr_un, &addr_len);

  int result = -1, retry = 0;
  do {
    result = connect(through_fd, (struct sockaddr*)&addr_un, addr_len);
    if (result == -1) {
      perror("connect error");
      usleep(to_usec);
    } else {
      break;
    }
  } while (result == -1 && retry++ <= retry_count);

  int sent_length = -1;
  if (result != -1) {
    sent_length =
        profiler::write_to_fd(through_fd, (void*)data, strlen(data), send_fd);
  }
  close(through_fd);

  return sent_length;
}

int AcceptAndGetDataFromSocket(int socket_fd, int* receive_fd, char* buffer,
                               int length, int to_sec, int to_usec) {
  struct timeval timeout;
  timeout.tv_sec = to_sec;
  timeout.tv_usec = to_usec;

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(socket_fd, &fds);

  // Select, connect and read from the next available connection @ socket_fd
  int read_count = -1;
  if (select(socket_fd + 1, &fds, nullptr, nullptr, &timeout) >= 0) {
    if (FD_ISSET(socket_fd, &fds)) {
      int through_fd = -1;
      if ((through_fd = accept(socket_fd, nullptr, nullptr)) == -1) {
        perror("accept error");
        exit(-1);
      }

      read_count =
          profiler::read_from_fd(through_fd, (void*)buffer, length, receive_fd);
      close(through_fd);
    }
  }

  return read_count;
}

}  // namespace profiler
