#include "Document.h"
#include <algorithm>

Document::Document()
    : fullName_("Untitled Candidate"), jobTitle_("") {}

Document::Document(std::string fullName)
    : fullName_(std::move(fullName)), jobTitle_("") {}

void Document::setFullName(const std::string& name) {
    fullName_ = name;
}

const std::string& Document::getFullName() const {
    return fullName_;
}

void Document::setJobTitle(const std::string& title) {
    jobTitle_ = title;
}

const std::string& Document::getJobTitle() const {
    return jobTitle_;
}

void Document::setPhoto(const std::string& base64Data, const std::string& mimeType) {
    photoBase64_ = base64Data;
    photoMimeType_ = mimeType.empty() ? "image/jpeg" : mimeType;
    hasPhoto_ = !photoBase64_.empty();
}

void Document::clearPhoto() {
    photoBase64_.clear();
    photoMimeType_.clear();
    hasPhoto_ = false;
}

bool Document::hasPhoto() const {
    return hasPhoto_;
}

const std::string& Document::getPhotoBase64() const {
    return photoBase64_;
}

const std::string& Document::getPhotoMimeType() const {
    return photoMimeType_;
}

void Document::addSection(const std::shared_ptr<Section>& section) {
    if (section) {
        sections_.push_back(section);
    }
}

const std::vector<std::shared_ptr<Section>>& Document::getSections() const {
    return sections_;
}

std::shared_ptr<Section> Document::findSection(SectionType type) const {
    for (const auto& section : sections_) {
        if (section && section->getType() == type) {
            return section;
        }
    }
    return nullptr;
}

void Document::sortSectionsByOrder() {
    std::stable_sort(
        sections_.begin(), sections_.end(),
        [](const std::shared_ptr<Section>& a, const std::shared_ptr<Section>& b) {
            return a->getOrder() < b->getOrder();
        });
}

bool Document::isEmpty() const {
    return sections_.empty();
}
