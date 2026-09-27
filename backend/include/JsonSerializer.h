#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include <string>
#include <optional>
#include "Document.h"

// Serializes and deserializes Document objects to/from JSON.
// This is the sole bridge format between the independent frontend
// and backend — neither side depends on the other's implementation.
class JsonSerializer {
public:
    static std::string documentToJson(const Document& document);
    static std::optional<Document> documentFromJson(const std::string& json);

    static std::string escapeJsonString(const std::string& input);
    static std::string buildErrorResponse(const std::string& message);
    static std::string buildSuccessResponse(const std::string& payloadJson);
};

#endif // JSON_SERIALIZER_H
