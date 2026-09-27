#include "Section.h"

std::string sectionTypeToString(SectionType type) {
    switch (type) {
        case SectionType::PERSONAL_INFO:    return "Personal Information";
        case SectionType::SUMMARY:          return "Professional Summary";
        case SectionType::WORK_EXPERIENCE:  return "Work Experience";
        case SectionType::EDUCATION:        return "Education";
        case SectionType::SKILLS:           return "Skills";
        case SectionType::PROJECTS:         return "Projects";
        case SectionType::CERTIFICATIONS:   return "Certifications";
        case SectionType::LANGUAGES:        return "Languages";
        case SectionType::CUSTOM:           return "Custom Section";
    }
    return "Section";
}

std::string sectionTypeToId(SectionType type) {
    switch (type) {
        case SectionType::PERSONAL_INFO:    return "PERSONAL_INFO";
        case SectionType::SUMMARY:          return "SUMMARY";
        case SectionType::WORK_EXPERIENCE:  return "WORK_EXPERIENCE";
        case SectionType::EDUCATION:        return "EDUCATION";
        case SectionType::SKILLS:           return "SKILLS";
        case SectionType::PROJECTS:         return "PROJECTS";
        case SectionType::CERTIFICATIONS:   return "CERTIFICATIONS";
        case SectionType::LANGUAGES:        return "LANGUAGES";
        case SectionType::CUSTOM:           return "CUSTOM";
    }
    return "CUSTOM";
}

SectionType stringToSectionType(const std::string& typeName) {
    if (typeName == "PERSONAL_INFO")    return SectionType::PERSONAL_INFO;
    if (typeName == "SUMMARY")          return SectionType::SUMMARY;
    if (typeName == "WORK_EXPERIENCE")  return SectionType::WORK_EXPERIENCE;
    if (typeName == "EDUCATION")        return SectionType::EDUCATION;
    if (typeName == "SKILLS")           return SectionType::SKILLS;
    if (typeName == "PROJECTS")         return SectionType::PROJECTS;
    if (typeName == "CERTIFICATIONS")   return SectionType::CERTIFICATIONS;
    if (typeName == "LANGUAGES")        return SectionType::LANGUAGES;
    if (typeName == "CUSTOM")           return SectionType::CUSTOM;
    return SectionType::CUSTOM;
}

Section::Section(SectionType type, std::string title)
    : type_(type), title_(std::move(title)), order_(0) {}

void Section::addField(const std::string& key, const std::string& value) {
    fields_.emplace_back(key, value);
}

void Section::addEntry(const std::vector<std::pair<std::string, std::string>>& entry) {
    entries_.push_back(entry);
}

SectionType Section::getType() const {
    return type_;
}

const std::string& Section::getTitle() const {
    return title_;
}

void Section::setTitle(const std::string& title) {
    title_ = title;
}

const std::vector<std::pair<std::string, std::string>>& Section::getFields() const {
    return fields_;
}

const std::vector<std::vector<std::pair<std::string, std::string>>>& Section::getEntries() const {
    return entries_;
}

void Section::setOrder(int order) {
    order_ = order;
}

int Section::getOrder() const {
    return order_;
}

bool Section::isEmpty() const {
    return fields_.empty() && entries_.empty();
}
