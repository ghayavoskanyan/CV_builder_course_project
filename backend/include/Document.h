#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
#include <vector>
#include <memory>
#include "Section.h"

// Document aggregates the full set of Section objects that make up
// one resume/CV, along with top-level identity data (name, headline).
// It owns its sections via shared_ptr so that the same Section could,
// in principle, be referenced by multiple in-progress documents
// (e.g. duplicating a CV to try a different template) without a deep
// copy.
class Document {
public:
    Document();
    explicit Document(std::string fullName);

    void setFullName(const std::string& name);
    const std::string& getFullName() const;

    void setJobTitle(const std::string& title);
    const std::string& getJobTitle() const;

    // Optional profile photo stored as raw base64 (no data-URL prefix).
    // When hasPhoto() is false, renderers omit the photo block entirely.
    void setPhoto(const std::string& base64Data, const std::string& mimeType);
    void clearPhoto();
    bool hasPhoto() const;
    const std::string& getPhotoBase64() const;
    const std::string& getPhotoMimeType() const;

    void addSection(const std::shared_ptr<Section>& section);
    const std::vector<std::shared_ptr<Section>>& getSections() const;

    // Returns the first section matching the given type, or nullptr
    // if no such section has been added yet.
    std::shared_ptr<Section> findSection(SectionType type) const;

    // Stable-sorts sections by Section::getOrder() ascending. Called
    // before export so that sections appear in a predictable order
    // regardless of the order the user filled them in.
    void sortSectionsByOrder();

    bool isEmpty() const;

private:
    std::string fullName_;
    std::string jobTitle_;
    std::string photoBase64_;
    std::string photoMimeType_;
    bool hasPhoto_ = false;
    std::vector<std::shared_ptr<Section>> sections_;
};

#endif // DOCUMENT_H
