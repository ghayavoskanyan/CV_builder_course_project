#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <functional>
#include <string>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
};

struct HttpResponse {
    int statusCode = 200;
    std::string contentType = "application/json; charset=utf-8";
    std::string body;
};

using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

// Minimal HTTP server for serving the JSON API. Uses platform sockets
// (Winsock on Windows, BSD sockets elsewhere) with no third-party deps.
class HttpServer {
public:
    explicit HttpServer(int port = 8080);
    ~HttpServer();

    void setHandler(HttpHandler handler);
    void run();
    void stop();

private:
    int port_;
    int serverSocket_;
    bool running_;
    HttpHandler handler_;

    bool initializeSocket();
    void cleanupSocket();
    HttpRequest parseRequest(const std::string& raw) const;
    std::string buildResponse(const HttpResponse& response) const;
    void handleClient(int clientSocket);
    // Reads a full HTTP request off the socket, looping over recv() until
    // the header block is complete AND every byte promised by
    // Content-Length has arrived. A single recv() call only returns
    // "some" of the available bytes (never guaranteed to be the whole
    // request), so anything beyond ~64KB — e.g. a document with an
    // embedded photo — was previously getting silently truncated.
    std::string recvFullRequest(int clientSocket) const;
    // Loops over send() until every byte of `data` has been written.
    // Like recv(), a single send() call is not guaranteed to write the
    // whole buffer (especially for larger payloads such as a base64
    // encoded PDF), so without this loop the client could receive a
    // truncated, unparsable response.
    void sendAll(int clientSocket, const std::string& data) const;
};

#endif // HTTP_SERVER_H
