#ifndef CV_SERVICE_H
#define CV_SERVICE_H

#include <string>
#include "Document.h"

// Application service that orchestrates CV export and preview operations.
// Keeps HTTP/controller code free of Template/Strategy wiring details.
class CVService {
public:
    struct ExportResult {
        bool success = false;
        std::string message;
        std::string filename;
        std::string mimeType;
        std::string content;
        std::string contentBase64;
    };

    ExportResult exportDocument(const Document& document,
                                const std::string& themeId,
                                const std::string& format) const;

    ExportResult previewDocument(const Document& document,
                                 const std::string& themeId) const;

    static std::string themesToJson();
};

#endif // CV_SERVICE_H
