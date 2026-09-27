#include "ApiController.h"
#include "JsonSerializer.h"
#include <cctype>
#include <iostream>
#include <sstream>

ApiController::ApiController() = default;

HttpResponse ApiController::jsonResponse(int status, const std::string& body) const {
    HttpResponse response;
    response.statusCode = status;
    response.body = body;
    return response;
}

HttpResponse ApiController::handleOptions() const {
    return jsonResponse(204, "");
}

HttpResponse ApiController::handleThemes() const {
    return jsonResponse(200, service_.themesToJson());
}

std::string ApiController::extractJsonString(const std::string& json, const std::string& key) {
    const std::string search = "\"" + key + "\":\"";
    std::size_t pos = json.find(search);
    if (pos == std::string::npos) {
        return "";
    }
    pos += search.size();
    std::string out;
    while (pos < json.size()) {
        char c = json[pos++];
        if (c == '"') {
            break;
        }
        if (c == '\\' && pos < json.size()) {
            char esc = json[pos++];
            if (esc == 'n') out += '\n';
            else if (esc == 't') out += '\t';
            else if (esc == 'r') out += '\r';
            else out += esc;
        } else {
            out += c;
        }
    }
    return out;
}

std::string ApiController::extractJsonObject(const std::string& json, const std::string& key) {
    const std::string search = "\"" + key + "\":";
    std::size_t pos = json.find(search);
    if (pos == std::string::npos) {
        return "";
    }
    pos += search.size();
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }
    if (pos >= json.size() || json[pos] != '{') {
        return "";
    }

    int depth = 0;
    std::size_t start = pos;
    do {
        if (json[pos] == '{') ++depth;
        if (json[pos] == '}') --depth;
        ++pos;
    } while (pos < json.size() && depth > 0);

    return json.substr(start, pos - start);
}

HttpResponse ApiController::handleExport(const std::string& body) const {
    const std::string documentJson = extractJsonObject(body, "document");
    if (documentJson.empty()) {
        return jsonResponse(400, JsonSerializer::buildErrorResponse("Missing 'document' field."));
    }

    auto documentOpt = JsonSerializer::documentFromJson(documentJson);
    if (!documentOpt) {
        return jsonResponse(400, JsonSerializer::buildErrorResponse("Invalid document JSON."));
    }

    const std::string theme = extractJsonString(body, "theme");
    const std::string format = extractJsonString(body, "format");
    if (theme.empty() || format.empty()) {
        return jsonResponse(400, JsonSerializer::buildErrorResponse("Missing 'theme' or 'format'."));
    }

    CVService::ExportResult result = service_.exportDocument(*documentOpt, theme, format);
    if (!result.success) {
        return jsonResponse(400, JsonSerializer::buildErrorResponse(result.message));
    }

    std::ostringstream payload;
    payload << "\"filename\":\"" << JsonSerializer::escapeJsonString(result.filename) << "\"";
    payload << ",\"mimeType\":\"" << JsonSerializer::escapeJsonString(result.mimeType) << "\"";

    if (!result.content.empty()) {
        payload << ",\"content\":\"" << JsonSerializer::escapeJsonString(result.content) << "\"";
    }
    if (!result.contentBase64.empty()) {
        payload << ",\"contentBase64\":\"" << JsonSerializer::escapeJsonString(result.contentBase64) << "\"";
    }

    return jsonResponse(200, JsonSerializer::buildSuccessResponse(payload.str()));
}

HttpResponse ApiController::handlePreview(const std::string& body) const {
    const std::string documentJson = extractJsonObject(body, "document");
    if (documentJson.empty()) {
        return jsonResponse(400, JsonSerializer::buildErrorResponse("Missing 'document' field."));
    }

    auto documentOpt = JsonSerializer::documentFromJson(documentJson);
    if (!documentOpt) {
        return jsonResponse(400, JsonSerializer::buildErrorResponse("Invalid document JSON."));
    }

    const std::string theme = extractJsonString(body, "theme");
    if (theme.empty()) {
        return jsonResponse(400, JsonSerializer::buildErrorResponse("Missing 'theme'."));
    }

    CVService::ExportResult result = service_.previewDocument(*documentOpt, theme);
    if (!result.success) {
        return jsonResponse(400, JsonSerializer::buildErrorResponse(result.message));
    }

    std::ostringstream payload;
    payload << "\"html\":\"" << JsonSerializer::escapeJsonString(result.content) << "\"";
    return jsonResponse(200, JsonSerializer::buildSuccessResponse(payload.str()));
}

HttpResponse ApiController::handleRequest(const HttpRequest& request) const {
    if (request.method == "OPTIONS") {
        return handleOptions();
    }

    if (request.method == "GET" && request.path == "/api/themes") {
        return handleThemes();
    }

    if (request.method == "POST" && request.path == "/api/export") {
        return handleExport(request.body);
    }

    if (request.method == "POST" && request.path == "/api/preview") {
        return handlePreview(request.body);
    }

    return jsonResponse(404, JsonSerializer::buildErrorResponse("Endpoint not found."));
}

void ApiController::startServer(int port) {
    HttpServer server(port);
    server.setHandler([this](const HttpRequest& request) {
        return handleRequest(request);
    });
    server.run();
}
