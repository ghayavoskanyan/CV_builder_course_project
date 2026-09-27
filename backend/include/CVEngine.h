#ifndef CV_ENGINE_H
#define CV_ENGINE_H

#include <memory>
#include <string>
#include "Document.h"
#include "Template.h"
#include "RenderStrategy.h"

// CVEngine is the Controller of the application: it drives an
// interactive command-line workflow that lets the user fill in each
// dynamic Section of a Document, pick a Template style and a
// RenderStrategy output format, and finally trigger export.
//
// It owns the in-progress Document plus the currently selected
// Template/RenderStrategy pair, and contains no rendering logic of
// its own - all formatting is delegated to the Strategy/Template
// Method collaborators.
class CVEngine {
public:
    CVEngine();

    // Runs the full interactive session until the user chooses to exit.
    void run();

private:
    Document document_;
    std::shared_ptr<Template> selectedTemplate_;
    std::shared_ptr<RenderStrategy> selectedStrategy_;

    void showMainMenu();

    void collectPersonalInfo();
    void collectSummary();
    void collectWorkExperience();
    void collectEducation();
    void collectSkills();
    void collectProjects();

    void chooseTemplateAndFormat();
    void exportCV();
    void showDocumentPreview() const;

    static std::string promptLine(const std::string& prompt);
    static int promptInt(const std::string& prompt, int minVal, int maxVal);
    static bool promptYesNo(const std::string& prompt);
};

#endif // CV_ENGINE_H
