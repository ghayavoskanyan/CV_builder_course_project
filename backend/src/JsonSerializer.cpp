#include "JsonSerializer.h"
#include "Section.h"
#include <sstream>
#include <cctype>

namespace {

void skipWhitespace(const std::string& json, std::size_t& pos) {
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }
}

bool expectChar(const std::string& json, std::size_t& pos, char expected) {
    skipWhitespace(json, pos);
    if (pos >= json.size() || json[pos] != expected) {
        return false;
    }
    ++pos;
    return true;
}

std::optional<std::string> parseJsonString(const std::string& json, std::size_t& pos) {
    skipWhitespace(json, pos);
    if (pos >= json.size() || json[pos] != '"') {
        return std::nullopt;
    }
    ++pos;

    std::string out;
    while (pos < json.size()) {
        char c = json[pos++];
        if (c == '"') {
            return out;
        }
        if (c == '\\' && pos < json.size()) {
            char esc = json[pos++];
            switch (esc) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case '/':  out += '/';  break;
                case 'b':  out += '\b'; break;
                case 'f':  out += '\f'; break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                case 't':  out += '\t'; break;
                case 'u':  pos += 4; break;
                default:   out += esc;  break;
            }
        } else {
            out += c;
        }
    }
    return std::nullopt;
}

std::optional<int> parseJsonInt(const std::string& json, std::size_t& pos) {
    skipWhitespace(json, pos);
    std::size_t start = pos;
    if (pos < json.size() && (json[pos] == '-' || json[pos] == '+')) {
        ++pos;
    }
    while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }
    if (start == pos) {
        return std::nullopt;
    }
    try {
        return std::stoi(json.substr(start, pos - start));
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

// Returns true and advances pos past "null" if that's what's at the
// current position, false (leaving pos untouched) otherwise. This must
// be a plain bool, not std::optional<bool>: every call site does
// `if (parseJsonNull(json, pos))`, and an optional<bool> is always
// "truthy" in that context regardless of the value it wraps (it only
// checks has_value(), which is always true here) — so returning
// optional<bool>(false) still took the "it was null" branch. That
// falsely treated non-null values (like a real {"data":...} photo
// object) as null, leaving pos stuck at the unconsumed '{' and
// corrupting the rest of the parse — which is exactly what broke
// export/preview any time a CV included a profile photo.
bool parseJsonNull(const std::string& json, std::size_t& pos) {
    skipWhitespace(json, pos);
    if (json.compare(pos, 4, "null") == 0) {
        pos += 4;
        return true;
    }
    return false;
}

std::optional<std::vector<std::pair<std::string, std::string>>> parseFieldArray(
    const std::string& json, std::size_t& pos);

std::optional<std::vector<std::vector<std::pair<std::string, std::string>>>> parseEntriesArray(
    const std::string& json, std::size_t& pos);

std::optional<std::shared_ptr<Section>> parseSectionObject(
    const std::string& json, std::size_t& pos) {

    if (!expectChar(json, pos, '{')) {
        return std::nullopt;
    }

    std::string typeStr;
    std::string title;
    int order = 0;
    std::vector<std::pair<std::string, std::string>> fields;
    std::vector<std::vector<std::pair<std::string, std::string>>> entries;

    skipWhitespace(json, pos);
    while (pos < json.size() && json[pos] != '}') {
        auto keyOpt = parseJsonString(json, pos);
        if (!keyOpt) {
            return std::nullopt;
        }
        if (!expectChar(json, pos, ':')) {
            return std::nullopt;
        }

        const std::string& key = *keyOpt;
        if (key == "type") {
            auto val = parseJsonString(json, pos);
            if (!val) return std::nullopt;
            typeStr = *val;
        } else if (key == "title") {
            auto val = parseJsonString(json, pos);
            if (!val) return std::nullopt;
            title = *val;
        } else if (key == "order") {
            auto val = parseJsonInt(json, pos);
            if (!val) return std::nullopt;
            order = *val;
        } else if (key == "fields") {
            auto val = parseFieldArray(json, pos);
            if (!val) return std::nullopt;
            fields = *val;
        } else if (key == "entries") {
            auto val = parseEntriesArray(json, pos);
            if (!val) return std::nullopt;
            entries = *val;
        } else {
            // Skip unknown values
            skipWhitespace(json, pos);
            if (pos < json.size() && json[pos] == '"') {
                if (!parseJsonString(json, pos)) return std::nullopt;
            } else if (pos < json.size() && json[pos] == '{') {
                int depth = 0;
                do {
                    if (json[pos] == '{') ++depth;
                    if (json[pos] == '}') --depth;
                    ++pos;
                } while (pos < json.size() && depth > 0);
            } else if (pos < json.size() && json[pos] == '[') {
                int depth = 0;
                do {
                    if (json[pos] == '[') ++depth;
                    if (json[pos] == ']') --depth;
                    ++pos;
                } while (pos < json.size() && depth > 0);
            } else {
                while (pos < json.size() && json[pos] != ',' && json[pos] != '}') {
                    ++pos;
                }
            }
        }

        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
        }
        skipWhitespace(json, pos);
    }

    if (!expectChar(json, pos, '}')) {
        return std::nullopt;
    }

    SectionType type = stringToSectionType(typeStr);
    if (title.empty()) {
        title = sectionTypeToString(type);
    }

    auto section = std::make_shared<Section>(type, title);
    section->setOrder(order);
    for (const auto& field : fields) {
        section->addField(field.first, field.second);
    }
    for (const auto& entry : entries) {
        section->addEntry(entry);
    }
    return section;
}

std::optional<std::vector<std::pair<std::string, std::string>>> parseFieldArray(
    const std::string& json, std::size_t& pos) {

    std::vector<std::pair<std::string, std::string>> fields;
    if (!expectChar(json, pos, '[')) {
        return std::nullopt;
    }

    skipWhitespace(json, pos);
    while (pos < json.size() && json[pos] != ']') {
        if (!expectChar(json, pos, '{')) {
            return std::nullopt;
        }

        std::string key;
        std::string value;
        skipWhitespace(json, pos);
        while (pos < json.size() && json[pos] != '}') {
            auto fieldKey = parseJsonString(json, pos);
            if (!fieldKey) return std::nullopt;
            if (!expectChar(json, pos, ':')) return std::nullopt;

            if (*fieldKey == "key") {
                auto val = parseJsonString(json, pos);
                if (!val) return std::nullopt;
                key = *val;
            } else if (*fieldKey == "value") {
                auto val = parseJsonString(json, pos);
                if (!val) return std::nullopt;
                value = *val;
            } else {
                skipWhitespace(json, pos);
                if (pos < json.size() && json[pos] == '"') {
                    if (!parseJsonString(json, pos)) return std::nullopt;
                }
            }

            skipWhitespace(json, pos);
            if (pos < json.size() && json[pos] == ',') {
                ++pos;
            }
        }

        if (!expectChar(json, pos, '}')) {
            return std::nullopt;
        }
        fields.emplace_back(key, value);

        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
        }
    }

    if (!expectChar(json, pos, ']')) {
        return std::nullopt;
    }
    return fields;
}

std::optional<std::vector<std::vector<std::pair<std::string, std::string>>>> parseEntriesArray(
    const std::string& json, std::size_t& pos) {

    std::vector<std::vector<std::pair<std::string, std::string>>> entries;
    if (!expectChar(json, pos, '[')) {
        return std::nullopt;
    }

    skipWhitespace(json, pos);
    while (pos < json.size() && json[pos] != ']') {
        auto entryFields = parseFieldArray(json, pos);
        if (!entryFields) {
            return std::nullopt;
        }
        entries.push_back(*entryFields);

        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
        }
    }

    if (!expectChar(json, pos, ']')) {
        return std::nullopt;
    }
    return entries;
}

} // namespace

std::string JsonSerializer::escapeJsonString(const std::string& input) {
    std::string out;
    out.reserve(input.size() + 8);
    for (char c : input) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    continue;
                }
                out += c;
                break;
        }
    }
    return out;
}

std::string JsonSerializer::documentToJson(const Document& document) {
    std::ostringstream json;
    json << "{";
    json << "\"fullName\":\"" << escapeJsonString(document.getFullName()) << "\"";
    json << ",\"jobTitle\":\"" << escapeJsonString(document.getJobTitle()) << "\"";

    if (document.hasPhoto()) {
        json << ",\"photo\":{";
        json << "\"mimeType\":\"" << escapeJsonString(document.getPhotoMimeType()) << "\"";
        json << ",\"data\":\"" << escapeJsonString(document.getPhotoBase64()) << "\"";
        json << "}";
    } else {
        json << ",\"photo\":null";
    }

    json << ",\"sections\":[";
    const auto& sections = document.getSections();
    for (std::size_t i = 0; i < sections.size(); ++i) {
        if (i > 0) json << ',';
        const auto& section = sections[i];
        json << "{";
        json << "\"type\":\"" << sectionTypeToId(section->getType()) << "\"";
        json << ",\"title\":\"" << escapeJsonString(section->getTitle()) << "\"";
        json << ",\"order\":" << section->getOrder();
        json << ",\"fields\":[";
        const auto& fields = section->getFields();
        for (std::size_t f = 0; f < fields.size(); ++f) {
            if (f > 0) json << ',';
            json << "{\"key\":\"" << escapeJsonString(fields[f].first) << "\"";
            json << ",\"value\":\"" << escapeJsonString(fields[f].second) << "\"}";
        }
        json << "],\"entries\":[";
        const auto& entries = section->getEntries();
        for (std::size_t e = 0; e < entries.size(); ++e) {
            if (e > 0) json << ',';
            json << "[";
            for (std::size_t k = 0; k < entries[e].size(); ++k) {
                if (k > 0) json << ',';
                json << "{\"key\":\"" << escapeJsonString(entries[e][k].first) << "\"";
                json << ",\"value\":\"" << escapeJsonString(entries[e][k].second) << "\"}";
            }
            json << "]";
        }
        json << "]}";
    }
    json << "]}";
    return json.str();
}

std::optional<Document> JsonSerializer::documentFromJson(const std::string& json) {
    std::size_t pos = 0;
    if (!expectChar(json, pos, '{')) {
        return std::nullopt;
    }

    Document document;
    std::string fullName;
    std::string jobTitle;
    bool hasPhoto = false;
    std::string photoData;
    std::string photoMime;

    skipWhitespace(json, pos);
    while (pos < json.size() && json[pos] != '}') {
        auto keyOpt = parseJsonString(json, pos);
        if (!keyOpt) return std::nullopt;
        if (!expectChar(json, pos, ':')) return std::nullopt;

        if (*keyOpt == "fullName") {
            auto val = parseJsonString(json, pos);
            if (!val) return std::nullopt;
            fullName = *val;
        } else if (*keyOpt == "jobTitle") {
            auto val = parseJsonString(json, pos);
            if (!val) return std::nullopt;
            jobTitle = *val;
        } else if (*keyOpt == "photo") {
            skipWhitespace(json, pos);
            if (parseJsonNull(json, pos)) {
                hasPhoto = false;
            } else if (expectChar(json, pos, '{')) {
                skipWhitespace(json, pos);
                while (pos < json.size() && json[pos] != '}') {
                    auto photoKey = parseJsonString(json, pos);
                    if (!photoKey) return std::nullopt;
                    if (!expectChar(json, pos, ':')) return std::nullopt;
                    if (*photoKey == "data" || *photoKey == "base64") {
                        auto val = parseJsonString(json, pos);
                        if (!val) return std::nullopt;
                        photoData = *val;
                    } else if (*photoKey == "mimeType") {
                        auto val = parseJsonString(json, pos);
                        if (!val) return std::nullopt;
                        photoMime = *val;
                    } else {
                        skipWhitespace(json, pos);
                        if (pos < json.size() && json[pos] == '"') {
                            if (!parseJsonString(json, pos)) return std::nullopt;
                        }
                    }
                    skipWhitespace(json, pos);
                    if (pos < json.size() && json[pos] == ',') ++pos;
                }
                expectChar(json, pos, '}');
                hasPhoto = !photoData.empty();
            }
        } else if (*keyOpt == "sections") {
            if (!expectChar(json, pos, '[')) return std::nullopt;
            skipWhitespace(json, pos);
            while (pos < json.size() && json[pos] != ']') {
                auto section = parseSectionObject(json, pos);
                if (!section) return std::nullopt;
                document.addSection(*section);
                skipWhitespace(json, pos);
                if (pos < json.size() && json[pos] == ',') ++pos;
            }
            if (!expectChar(json, pos, ']')) return std::nullopt;
        } else {
            skipWhitespace(json, pos);
            if (pos < json.size() && json[pos] == '"') {
                if (!parseJsonString(json, pos)) return std::nullopt;
            } else if (pos < json.size() && (json[pos] == '{' || json[pos] == '[')) {
                int depth = 0;
                char open = json[pos];
                char close = (open == '{') ? '}' : ']';
                do {
                    if (json[pos] == open) ++depth;
                    if (json[pos] == close) --depth;
                    ++pos;
                } while (pos < json.size() && depth > 0);
            } else {
                while (pos < json.size() && json[pos] != ',' && json[pos] != '}') ++pos;
            }
        }

        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') ++pos;
    }

    if (!expectChar(json, pos, '}')) {
        return std::nullopt;
    }

    document.setFullName(fullName.empty() ? "Untitled Candidate" : fullName);
    document.setJobTitle(jobTitle);
    if (hasPhoto) {
        document.setPhoto(photoData, photoMime);
    } else {
        document.clearPhoto();
    }

    return document;
}

std::string JsonSerializer::buildErrorResponse(const std::string& message) {
    return "{\"success\":false,\"message\":\"" + escapeJsonString(message) + "\"}";
}

std::string JsonSerializer::buildSuccessResponse(const std::string& payloadJson) {
    if (payloadJson.empty()) {
        return "{\"success\":true}";
    }
    return "{\"success\":true," + payloadJson + "}";
}
