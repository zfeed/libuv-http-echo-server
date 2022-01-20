#ifndef SERVER_H
#define SERVER_H

#include "uv.h"
#include <stdexcept>

namespace server {

class Utility {
    public:
        void throwException(int status);
        std::runtime_error createRuntimeException(int status);
};

class Socket;

typedef void (*OnRead)(ssize_t nread, const uv_buf_t* buf, Socket* socket);
typedef void (*OnWrite)(int status, Socket* socket, const uv_buf_t* buf);
typedef void (*OnClose)();

class Socket : public Utility {
    public:
        void* context; 
        uv_stream_t *server;
        uv_loop_t* loop;
        int status;
        uv_tcp_t *client;
        OnRead onReadCallback;
        OnWrite onWriteCallback;
        OnClose onCloseCallback;
        uv_alloc_cb allocationCallback;

        Socket(uv_stream_t *server, int status, uv_loop_t* loop);
        void read();
        void write(const uv_buf_t* buf);
        void stopRead();
        void close();
        void setOnReadCallback(OnRead onReadCallback);
        void setOnWriteCallback(OnWrite OnWriteCallback);
        void setOnCloseCallback(OnClose onCloseCallback);
        void setAllocationCallback(uv_alloc_cb allocationCallback);
        static void onReadUV(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
        static void onWriteUV(uv_write_t *req, int status);
        static void onCloseUV(uv_handle_t* handle);
};

typedef struct {
    uv_write_t req;
    Socket* socket;
    const uv_buf_t* buf;
} write_req_t;

typedef void (*OnConnection)(int status, Socket* socket);

class TCP : public Utility {
    private:
        uv_loop_t* loop;
        uv_tcp_t server;
        sockaddr_in addr;
        OnConnection onConnection;

        static void uvConnectionCallback(uv_stream_t* server, int status);

    public:
        TCP(uv_loop_t* loop, OnConnection onConnection);
        void listen(const char* ip, int port);
        void handleTCPConnection(uv_stream_t *server, int status);
};

}

#endif /* SERVER_H */
