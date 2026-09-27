#ifndef API_CONTROLLER_H
#define API_CONTROLLER_H

#include "CVService.h"
#include "HttpServer.h"

// Facade/Controller for the REST-like JSON API. Routes incoming HTTP
// requests to CVService methods and returns JSON responses.
class ApiController {
public:
    ApiController();

    HttpResponse handleRequest(const HttpRequest& request) const;
    void startServer(int port);

private:
    CVService service_;

    HttpResponse handleThemes() const;
    HttpResponse handleExport(const std::string& body) const;
    HttpResponse handlePreview(const std::string& body) const;
    HttpResponse handleOptions() const;
    HttpResponse jsonResponse(int status, const std::string& body) const;

    static std::string extractJsonString(const std::string& json, const std::string& key);
    static std::string extractJsonObject(const std::string& json, const std::string& key);
};

#endif // API_CONTROLLER_H
