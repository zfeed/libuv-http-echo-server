#include "uv.h"
#include "server.h"
#include <stdexcept>

// g++ -I./deps/libuv/include -c -g -O0 -o server.o server.cpp

namespace server {

TCP::TCP(uv_loop_t* loop, OnConnection onConnection) {
    this->loop = loop;
    this->onConnection = onConnection;
    this->throwException(uv_tcp_init(this->loop, &this->server));
    server.data = this;
}

void TCP::listen(const char* ip, int port) {
    this->throwException(uv_ip4_addr(ip, port, &addr));
    this->throwException(uv_tcp_bind(&this->server, (const struct sockaddr*)&this->addr, 0));
    this->throwException(uv_listen((uv_stream_t*) &this->server, 10, this->uvConnectionCallback));
}

void TCP::uvConnectionCallback(uv_stream_t* server, int status) {
    TCP* tcp = (TCP*) server->data;
    tcp->handleTCPConnection(server, status);
}

void TCP::handleTCPConnection(uv_stream_t *server, int status) {
    Socket *socket = new Socket(server, status, this->loop);

    this->throwException(uv_accept(server, (uv_stream_t*) socket->client));

    this->onConnection(status, socket);
}

std::runtime_error Utility::createRuntimeException(int status) {
    return std::runtime_error(uv_err_name(status));
}

void Utility::throwException(int status) {
    if (status >= 0) {
        return;
    }

   throw this->createRuntimeException(status);
}

Socket::Socket(uv_stream_t *server, int status, uv_loop_t* loop) {
    this->server = server;
    this->status = status;
    this->context = NULL;
    this->client = new uv_tcp_t;
    this->client->data = this;
    this->throwException(uv_tcp_init(loop, this->client));
    this->loop = loop;
}

void Socket::write(const uv_buf_t* buf) {
    write_req_t* req = new write_req_t;
    req->socket = this;
    req->buf = buf;
    this->throwException(uv_write((uv_write_t*) req, (uv_stream_t*) this->client, buf, 1, this->onWriteUV));
}

void Socket::close() {
    uv_close((uv_handle_t*) this->client, this->onCloseUV);
}

void Socket::onCloseUV(uv_handle_t* handle) {
    Socket* socket = (Socket*) handle->data;
    socket->onCloseCallback();
    delete handle;
    delete socket;
}

void Socket::onWriteUV(uv_write_t *req, int status) {
    write_req_t* request = (write_req_t*) req;
    request->socket->onWriteCallback(status, request->socket, request->buf);
    delete req;
}

void Socket::setOnReadCallback(OnRead onRead) {
    this->onReadCallback = onRead;
}

void Socket::setOnCloseCallback(OnClose onClose) {
    this->onCloseCallback = onClose;
}

void Socket::setOnWriteCallback(OnWrite onWrite) {
    this->onWriteCallback = onWrite;
}

void Socket::setAllocationCallback(uv_alloc_cb allocationCallback) {
    this->allocationCallback = allocationCallback;
}

void Socket::read() {
    this->throwException(status);

    this->throwException(uv_read_start((uv_stream_t*) this->client, allocationCallback, this->onReadUV));
}

void Socket::stopRead() {
    this->throwException(uv_read_stop((uv_stream_t*) this->client));
}

void Socket::onReadUV(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
    Socket* socket = (Socket*) client->data;
    socket->onReadCallback(nread, buf, socket);
}

};
