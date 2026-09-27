#include "CVEngine.h"
#include "Section.h"
#include "HTMLRenderStrategy.h"
#include "PDFRenderStrategy.h"
#include "ClassicTemplate.h"
#include "ModernTemplate.h"

#include <iostream>
#include <vector>
#include <utility>
#include <stdexcept>

CVEngine::CVEngine()
    : document_(), selectedTemplate_(nullptr), selectedStrategy_(nullptr) {}

std::string CVEngine::promptLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int CVEngine::promptInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        try {
            std::size_t pos = 0;
            int value = std::stoi(line, &pos);
            if (pos == line.size() && value >= minVal && value <= maxVal) {
                return value;
            }
        } catch (const std::exception&) {
            // fall through to the error message below
        }
        std::cout << "Invalid input. Please enter a whole number between "
                  << minVal << " and " << maxVal << ".\n";
    }
}

bool CVEngine::promptYesNo(const std::string& prompt) {
    while (true) {
        std::string line = promptLine(prompt + " (y/n): ");
        if (!line.empty() && (line[0] == 'y' || line[0] == 'Y')) {
            return true;
        }
        if (!line.empty() && (line[0] == 'n' || line[0] == 'N')) {
            return false;
        }
        std::cout << "Please answer 'y' or 'n'.\n";
    }
}

void CVEngine::collectPersonalInfo() {
    std::cout << "\n--- Personal Information ---\n";

    std::string name = promptLine("Full Name: ");
    document_.setFullName(name.empty() ? "Unnamed Candidate" : name);

    std::string jobTitle = promptLine("Desired Job Title / Headline: ");
    document_.setJobTitle(jobTitle);

    auto personal = std::make_shared<Section>(SectionType::PERSONAL_INFO, "Personal Information");
    personal->setOrder(0);

    std::string email = promptLine("Email: ");
    if (!email.empty()) personal->addField("Email", email);

    std::string phone = promptLine("Phone: ");
    if (!phone.empty()) personal->addField("Phone", phone);

    std::string address = promptLine("Location (City, Country): ");
    if (!address.empty()) personal->addField("Location", address);

    std::string link = promptLine("LinkedIn / Portfolio URL: ");
    if (!link.empty()) personal->addField("Link", link);

    document_.addSection(personal);
    std::cout << "Personal information saved.\n";
}

void CVEngine::collectSummary() {
    std::cout << "\n--- Professional Summary ---\n";
    std::string summary = promptLine("Enter a short professional summary (or leave blank to skip): ");
    if (summary.empty()) {
        std::cout << "Skipped summary section.\n";
        return;
    }
    auto section = std::make_shared<Section>(SectionType::SUMMARY, "Professional Summary");
    section->setOrder(1);
    section->addField("", summary);
    document_.addSection(section);
    std::cout << "Summary saved.\n";
}

void CVEngine::collectWorkExperience() {
    std::cout << "\n--- Work Experience ---\n";
    auto section = std::make_shared<Section>(SectionType::WORK_EXPERIENCE, "Work Experience");
    section->setOrder(2);

    bool addMore = true;
    while (addMore) {
        std::vector<std::pair<std::string, std::string>> entry;

        std::string role = promptLine("  Job Title: ");
        std::string company = promptLine("  Company Name: ");
        std::string duration = promptLine("  Duration (e.g. Jan 2022 - Present): ");
        std::string description = promptLine("  Description / Achievements: ");

        if (!role.empty())        entry.emplace_back("Role", role);
        if (!company.empty())     entry.emplace_back("Company", company);
        if (!duration.empty())    entry.emplace_back("Duration", duration);
        if (!description.empty()) entry.emplace_back("Details", description);

        if (!entry.empty()) {
            section->addEntry(entry);
            std::cout << "  Entry added.\n";
        }

        addMore = promptYesNo("Add another work experience entry?");
    }

    if (!section->isEmpty()) {
        document_.addSection(section);
    } else {
        std::cout << "No work experience entries were added.\n";
    }
}

void CVEngine::collectEducation() {
    std::cout << "\n--- Education ---\n";
    auto section = std::make_shared<Section>(SectionType::EDUCATION, "Education");
    section->setOrder(3);

    bool addMore = true;
    while (addMore) {
        std::vector<std::pair<std::string, std::string>> entry;

        std::string degree = promptLine("  Degree / Program: ");
        std::string institution = promptLine("  Institution: ");
        std::string year = promptLine("  Graduation Year: ");
        std::string notes = promptLine("  Notes (GPA, Honors, etc.): ");

        if (!degree.empty())      entry.emplace_back("Degree", degree);
        if (!institution.empty()) entry.emplace_back("Institution", institution);
        if (!year.empty())        entry.emplace_back("Year", year);
        if (!notes.empty())       entry.emplace_back("Notes", notes);

        if (!entry.empty()) {
            section->addEntry(entry);
            std::cout << "  Entry added.\n";
        }

        addMore = promptYesNo("Add another education entry?");
    }

    if (!section->isEmpty()) {
        document_.addSection(section);
    } else {
        std::cout << "No education entries were added.\n";
    }
}

void CVEngine::collectSkills() {
    std::cout << "\n--- Skills ---\n";
    auto section = std::make_shared<Section>(SectionType::SKILLS, "Skills");
    section->setOrder(4);

    std::cout << "Enter one skill at a time. Leave blank and press Enter to finish.\n";
    while (true) {
        std::string skill = promptLine("  Skill: ");
        if (skill.empty()) {
            break;
        }
        section->addField("", skill);
    }

    if (!section->isEmpty()) {
        document_.addSection(section);
        std::cout << "Skills saved.\n";
    } else {
        std::cout << "No skills were added.\n";
    }
}

void CVEngine::collectProjects() {
    if (!promptYesNo("\nWould you like to add a Projects section?")) {
        return;
    }

    auto section = std::make_shared<Section>(SectionType::PROJECTS, "Projects");
    section->setOrder(5);

    bool addMore = true;
    while (addMore) {
        std::vector<std::pair<std::string, std::string>> entry;

        std::string projectName = promptLine("  Project Name: ");
        std::string techStack = promptLine("  Technologies Used: ");
        std::string details = promptLine("  Description: ");

        if (!projectName.empty()) entry.emplace_back("Project", projectName);
        if (!techStack.empty())   entry.emplace_back("Stack", techStack);
        if (!details.empty())    entry.emplace_back("Details", details);

        if (!entry.empty()) {
            section->addEntry(entry);
            std::cout << "  Project added.\n";
        }

        addMore = promptYesNo("Add another project?");
    }

    if (!section->isEmpty()) {
        document_.addSection(section);
    }
}

void CVEngine::chooseTemplateAndFormat() {
    std::cout << "\n--- Choose a Template Style ---\n";
    std::cout << "1. Classic (Traditional, serif, single-column)\n";
    std::cout << "2. Modern  (Clean, sans-serif, accented two-column)\n";
    int templateChoice = promptInt("Select a template [1-2]: ", 1, 2);

    std::cout << "\n--- Choose an Export Format ---\n";
    std::cout << "1. HTML\n";
    std::cout << "2. PDF\n";
    int formatChoice = promptInt("Select a format [1-2]: ", 1, 2);

    if (formatChoice == 1) {
        selectedStrategy_ = std::make_shared<HTMLRenderStrategy>();
    } else {
        selectedStrategy_ = std::make_shared<PDFRenderStrategy>();
    }

    if (templateChoice == 1) {
        selectedTemplate_ = std::make_shared<ClassicTemplate>(selectedStrategy_);
    } else {
        selectedTemplate_ = std::make_shared<ModernTemplate>(selectedStrategy_);
    }

    std::cout << "Selected '" << selectedTemplate_->getTemplateName()
              << "' template with '" << selectedStrategy_->getStrategyName() << "'.\n";
}

void CVEngine::showDocumentPreview() const {
    std::cout << "\n=== Document Preview ===\n";
    std::cout << "Name: " << document_.getFullName() << "\n";
    if (!document_.getJobTitle().empty()) {
        std::cout << "Headline: " << document_.getJobTitle() << "\n";
    }
    if (document_.getSections().empty()) {
        std::cout << "(No sections added yet.)\n";
    }
    for (const auto& section : document_.getSections()) {
        std::cout << "- " << section->getTitle()
                  << " (" << section->getFields().size() << " field(s), "
                  << section->getEntries().size() << " entrie(s))\n";
    }
    if (selectedTemplate_ && selectedStrategy_) {
        std::cout << "Template: " << selectedTemplate_->getTemplateName()
                  << " | Format: " << selectedStrategy_->getStrategyName() << "\n";
    } else {
        std::cout << "Template/Format: not yet selected.\n";
    }
    std::cout << "========================\n";
}

void CVEngine::exportCV() {
    if (!selectedTemplate_ || !selectedStrategy_) {
        std::cout << "No template/format selected yet. Let's choose one first.\n";
        chooseTemplateAndFormat();
    }

    document_.sortSectionsByOrder();

    std::string filename = promptLine("Enter output filename (without extension): ");
    if (filename.empty()) {
        filename = "my_resume";
    }

    std::string extension = selectedTemplate_->getStrategy()->getFileExtension();
    std::string fullPath = filename + extension;

    bool success = selectedTemplate_->exportDocument(document_, fullPath);
    if (success) {
        std::cout << "\nYour CV has been generated at: " << fullPath << "\n";
    } else {
        std::cout << "\nFailed to generate the CV. Please check your input (a name and "
                     "at least one section are required) and try again.\n";
    }
}

void CVEngine::showMainMenu() {
    bool running = true;
    while (running) {
        std::cout << "\n============================\n";
        std::cout << "   TEMPLATE CV BUILDER\n";
        std::cout << "============================\n";
        std::cout << "1. Enter/Edit Personal Information\n";
        std::cout << "2. Enter/Edit Professional Summary\n";
        std::cout << "3. Add Work Experience\n";
        std::cout << "4. Add Education\n";
        std::cout << "5. Add Skills\n";
        std::cout << "6. Add Projects (optional)\n";
        std::cout << "7. Choose Template & Format\n";
        std::cout << "8. Preview Document\n";
        std::cout << "9. Export CV\n";
        std::cout << "0. Exit\n";

        int choice = promptInt("Select an option: ", 0, 9);

        switch (choice) {
            case 1: collectPersonalInfo();     break;
            case 2: collectSummary();          break;
            case 3: collectWorkExperience();   break;
            case 4: collectEducation();        break;
            case 5: collectSkills();           break;
            case 6: collectProjects();         break;
            case 7: chooseTemplateAndFormat(); break;
            case 8: showDocumentPreview();     break;
            case 9: exportCV();                break;
            case 0:
                running = false;
                std::cout << "Goodbye! Thanks for using Template CV Builder.\n";
                break;
            default:
                break;
        }
    }
}

void CVEngine::run() {
    std::cout << "Welcome to the Template CV Builder!\n";
    std::cout << "Fill in your details, then choose a style to generate your resume.\n";
    showMainMenu();
}
