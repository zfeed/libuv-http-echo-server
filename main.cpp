#include "uv.h"
#include "server.h"
#include <iostream>

unsigned int MAX_HTTP_MESSAGE_SIZE = 1024 * 1;
unsigned int PORT = 8181;
char HOST[] = "127.0.0.1";

void onRead(ssize_t nread, const uv_buf_t* buf, server::Socket* socket) {
    std::cout << "Read, nread: " << nread << std::endl;

    if (nread < 0) {
        std::cout << "Socket read error: " << nread << std::endl;
        socket->close();
        free(buf->base);

        return;
    }

    char responseStart[] = "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n\r\n"
        "<pre>";

    char responseEnd[] = "</pre>";

    int length = (sizeof(responseStart) / sizeof(responseStart[0])) + (sizeof(responseEnd) / sizeof(responseEnd[0])) + nread - 2;

    char* base = (char*) malloc(length);

    if (base == NULL) {
        throw std::bad_alloc();
    }

    uv_buf_t* buffer = new uv_buf_t;

    buffer->base = base;
    buffer->len = length - 1;

    std::memcpy(
        base, 
        responseStart,
        sizeof(responseStart) - 1
    );

    std::memcpy(
        base + sizeof(responseStart) - 1, 
        buf->base,
        nread
    );

    std::memcpy(
        base + nread + sizeof(responseStart) - 1, 
        responseEnd,
        sizeof(responseEnd) - 1
    );

    socket->stopRead();

    socket->write(buffer);

    free(buf->base);
}


void onClose() {
    std::cout << "Close " << std::endl;
}

void allocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(MAX_HTTP_MESSAGE_SIZE);

    if (buf->base == NULL) {
        throw std::bad_alloc();
    }

    buf->len = MAX_HTTP_MESSAGE_SIZE;
}

void onWrite(int status, server::Socket* socket, const uv_buf_t* buf) {
    std::cout << "Write status: " << status <<  std::endl;

    socket->close();

    free(buf->base);
    free((uv_buf_t*) buf);
}

void onConnection(int status, server::Socket* socket) {
    socket->throwException(status);

    std::cout << "New connection" << std::endl;

    socket->setOnReadCallback(onRead);
    socket->setAllocationCallback(allocBuffer);
    socket->setOnCloseCallback(onClose);
    socket->setOnWriteCallback(onWrite);

    socket->read();
}

int main(int argc, char* argv[])
{
    uv_loop_t* loop = uv_default_loop();
    server::TCP tcp = server::TCP(loop, onConnection);

    tcp.listen(HOST, PORT);

    return uv_run(loop, UV_RUN_DEFAULT);
}
