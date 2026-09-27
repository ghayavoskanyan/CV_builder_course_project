#include "HttpServer.h"
#include <iostream>
#include <sstream>
#include <cstring>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
constexpr socket_t kInvalidSocket = INVALID_SOCKET;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
using socket_t = int;
constexpr socket_t kInvalidSocket = -1;
#endif

HttpServer::HttpServer(int port)
    : port_(port), serverSocket_(static_cast<int>(kInvalidSocket)), running_(false) {}

HttpServer::~HttpServer() {
    stop();
    cleanupSocket();
}

void HttpServer::setHandler(HttpHandler handler) {
    handler_ = std::move(handler);
}

bool HttpServer::initializeSocket() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[HttpServer] WSAStartup failed.\n";
        return false;
    }
#endif

    serverSocket_ = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
    if (static_cast<socket_t>(serverSocket_) == kInvalidSocket) {
        std::cerr << "[HttpServer] Failed to create socket.\n";
        return false;
    }

    int opt = 1;
    setsockopt(static_cast<socket_t>(serverSocket_), SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<unsigned short>(port_));

    if (bind(static_cast<socket_t>(serverSocket_),
             reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::cerr << "[HttpServer] Failed to bind to port " << port_ << ".\n";
        return false;
    }

    if (listen(static_cast<socket_t>(serverSocket_), 8) != 0) {
        std::cerr << "[HttpServer] Failed to listen.\n";
        return false;
    }

    return true;
}

void HttpServer::cleanupSocket() {
    if (static_cast<socket_t>(serverSocket_) != kInvalidSocket) {
#ifdef _WIN32
        closesocket(static_cast<socket_t>(serverSocket_));
        WSACleanup();
#else
        close(serverSocket_);
#endif
        serverSocket_ = static_cast<int>(kInvalidSocket);
    }
}

HttpRequest HttpServer::parseRequest(const std::string& raw) const {
    HttpRequest request;
    std::istringstream stream(raw);
    std::string requestLine;
    if (!std::getline(stream, requestLine)) {
        return request;
    }

    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    std::istringstream lineStream(requestLine);
    lineStream >> request.method >> request.path;

    std::string headerLine;
    int contentLength = 0;
    while (std::getline(stream, headerLine)) {
        if (!headerLine.empty() && headerLine.back() == '\r') {
            headerLine.pop_back();
        }
        if (headerLine.empty()) {
            break;
        }
        if (headerLine.rfind("Content-Length:", 0) == 0) {
            contentLength = std::stoi(headerLine.substr(15));
        }
    }

    if (contentLength > 0) {
        request.body.resize(static_cast<std::size_t>(contentLength));
        stream.read(&request.body[0], contentLength);
    }

    return request;
}

std::string HttpServer::buildResponse(const HttpResponse& response) const {
    std::ostringstream out;
    out << "HTTP/1.1 " << response.statusCode << " OK\r\n";
    out << "Content-Type: " << response.contentType << "\r\n";
    out << "Content-Length: " << response.body.size() << "\r\n";
    out << "Access-Control-Allow-Origin: *\r\n";
    out << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    out << "Access-Control-Allow-Headers: Content-Type\r\n";
    out << "Connection: close\r\n";
    out << "\r\n";
    out << response.body;
    return out.str();
}

std::string HttpServer::recvFullRequest(int clientSocket) const {
    std::string raw;
    char buffer[8192];
    std::size_t headerEnd = std::string::npos;
    long long contentLength = 0;
    long long bodyStart = -1;

    while (true) {
#ifdef _WIN32
        int received = recv(static_cast<socket_t>(clientSocket), buffer, sizeof(buffer), 0);
#else
        ssize_t received = recv(clientSocket, buffer, sizeof(buffer), 0);
#endif
        if (received <= 0) {
            break; // connection closed or error — return whatever we have
        }
        raw.append(buffer, static_cast<std::size_t>(received));

        if (headerEnd == std::string::npos) {
            headerEnd = raw.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                bodyStart = static_cast<long long>(headerEnd) + 4;
                const std::string headers = raw.substr(0, headerEnd);
                std::size_t clPos = headers.find("Content-Length:");
                if (clPos == std::string::npos) {
                    clPos = headers.find("content-length:");
                }
                if (clPos != std::string::npos) {
                    std::size_t valueStart = headers.find(':', clPos) + 1;
                    std::size_t lineEnd = headers.find("\r\n", valueStart);
                    std::string valueStr = headers.substr(
                        valueStart, lineEnd == std::string::npos ? std::string::npos : lineEnd - valueStart);
                    try {
                        contentLength = std::stoll(valueStr);
                    } catch (const std::exception&) {
                        contentLength = 0;
                    }
                }
            }
        }

        if (headerEnd != std::string::npos) {
            const long long bodyReceived = static_cast<long long>(raw.size()) - bodyStart;
            if (bodyReceived >= contentLength) {
                break; // full request (headers + body) received
            }
        }
    }

    return raw;
}

void HttpServer::sendAll(int clientSocket, const std::string& data) const {
    std::size_t totalSent = 0;
    while (totalSent < data.size()) {
#ifdef _WIN32
        int sent = send(static_cast<socket_t>(clientSocket), data.c_str() + totalSent,
                         static_cast<int>(data.size() - totalSent), 0);
#else
        ssize_t sent = send(clientSocket, data.c_str() + totalSent,
                             data.size() - totalSent, 0);
#endif
        if (sent <= 0) {
            break; // socket error or peer closed — give up rather than loop forever
        }
        totalSent += static_cast<std::size_t>(sent);
    }
}

void HttpServer::handleClient(int clientSocket) {
    std::string raw = recvFullRequest(clientSocket);

    if (raw.empty()) {
#ifdef _WIN32
        closesocket(static_cast<socket_t>(clientSocket));
#else
        close(clientSocket);
#endif
        return;
    }

    HttpRequest request = parseRequest(raw);
    HttpResponse response;

    if (handler_) {
        response = handler_(request);
    } else {
        response.statusCode = 500;
        response.body = "{\"success\":false,\"message\":\"No handler configured\"}";
    }

    std::string rawResponse = buildResponse(response);
    sendAll(clientSocket, rawResponse);

#ifdef _WIN32
    closesocket(static_cast<socket_t>(clientSocket));
#else
    close(clientSocket);
#endif
}

void HttpServer::run() {
    if (!initializeSocket()) {
        return;
    }

    running_ = true;
    std::cout << "[HttpServer] JSON API listening on http://localhost:" << port_ << "\n";
    std::cout << "[HttpServer] Endpoints: GET /api/themes, POST /api/export, POST /api/preview\n";

    while (running_) {
        sockaddr_in clientAddr{};
#ifdef _WIN32
        int clientLen = sizeof(clientAddr);
#else
        socklen_t clientLen = sizeof(clientAddr);
#endif
        socket_t client = accept(static_cast<socket_t>(serverSocket_),
                                 reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (client == kInvalidSocket) {
            if (running_) {
                std::cerr << "[HttpServer] Accept failed.\n";
            }
            break;
        }
        handleClient(static_cast<int>(client));
    }
}

void HttpServer::stop() {
    running_ = false;
}
