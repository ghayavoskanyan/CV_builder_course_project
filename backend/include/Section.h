#ifndef SECTION_H
#define SECTION_H

#include <string>
#include <vector>
#include <utility>

// Identifies the semantic type of a CV section. Used for lookups
// (e.g. Document::findSection) and for special-cased rendering
// (such as pulling contact fields into the header).
enum class SectionType {
    PERSONAL_INFO,
    SUMMARY,
    WORK_EXPERIENCE,
    EDUCATION,
    SKILLS,
    PROJECTS,
    CERTIFICATIONS,
    LANGUAGES,
    CUSTOM
};

// Returns a human-readable default title for a given SectionType.
std::string sectionTypeToString(SectionType type);

// Parses a SectionType from its JSON/API string identifier.
SectionType stringToSectionType(const std::string& typeName);

// Returns the JSON/API identifier for a SectionType.
std::string sectionTypeToId(SectionType type);

// Represents a single dynamic section of a CV/resume (e.g. "Work
// Experience", "Education", "Skills"). A Section can hold data in
// two shapes:
//
//   1. Simple fields  - flat key/value pairs (e.g. Email -> "a@b.com").
//      Used for sections like Personal Info or Skills (key can be
//      left empty for a plain list item).
//
//   2. Entries        - a list of "records", each of which is itself
//      a list of key/value pairs. Used for repeating sections like
//      Work Experience (one entry per job) or Education (one entry
//      per degree).
//
// This dual representation lets a single class model every dynamic
// section type requested by the project without needing a class
// hierarchy per section kind.
class Section {
public:
    Section(SectionType type, std::string title);

    // Adds a single flat key/value field to this section.
    void addField(const std::string& key, const std::string& value);

    // Adds a full multi-field record (e.g. one job, one degree).
    void addEntry(const std::vector<std::pair<std::string, std::string>>& entry);

    SectionType getType() const;
    const std::string& getTitle() const;
    void setTitle(const std::string& title);

    const std::vector<std::pair<std::string, std::string>>& getFields() const;
    const std::vector<std::vector<std::pair<std::string, std::string>>>& getEntries() const;

    // Controls the display order of sections within a Document.
    void setOrder(int order);
    int getOrder() const;

    // True if the section has no fields and no entries and therefore
    // should be skipped by renderers.
    bool isEmpty() const;

private:
    SectionType type_;
    std::string title_;
    std::vector<std::pair<std::string, std::string>> fields_;
    std::vector<std::vector<std::pair<std::string, std::string>>> entries_;
    int order_;
};

#endif // SECTION_H
