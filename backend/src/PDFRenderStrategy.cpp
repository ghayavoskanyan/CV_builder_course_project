#include "PDFRenderStrategy.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <array>

namespace {

// Simple normalized RGB color (0.0 - 1.0), matching the PDF "rg"
// color-space operator used inside content streams.
struct RGB {
    double r;
    double g;
    double b;
};

// Parses a "#RRGGBB" hex color string into normalized RGB. Falls
// back to a neutral dark gray if the string is malformed, so a bad
// theme value never crashes PDF generation.
RGB hexToRgb(const std::string& hex) {
    RGB color{0.1, 0.1, 0.1};
    if (hex.size() == 7 && hex[0] == '#') {
        try {
            int r = std::stoi(hex.substr(1, 2), nullptr, 16);
            int g = std::stoi(hex.substr(3, 2), nullptr, 16);
            int b = std::stoi(hex.substr(5, 2), nullptr, 16);
            color.r = static_cast<double>(r) / 255.0;
            color.g = static_cast<double>(g) / 255.0;
            color.b = static_cast<double>(b) / 255.0;
        } catch (const std::exception&) {
            // Keep the fallback color on any parse failure.
        }
    }
    return color;
}

// Formats a floating point number with fixed precision, as required
// for numeric operands inside PDF content streams.
std::string formatNumber(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << value;
    return oss.str();
}

// Standard Adobe AFM glyph widths (in 1/1000 em units) for the core
// Helvetica font, covering printable ASCII 0x20 (' ') through 0x7E ('~').
// These are the same public font-metric values built into every PDF
// viewer's base-14 font handling; using them lets us estimate real text
// width without embedding a font program, which is what makes accurate
// word-wrapping possible.
constexpr std::array<int, 95> kHelveticaWidths = {
    278, 278, 355, 556, 556, 889, 667, 191, 333, 333, 389, 584, 278, 333, 278, 278, // ' ' .. '/'
    556, 556, 556, 556, 556, 556, 556, 556, 556, 556, 278, 278, 584, 584, 584, 556, // '0' .. '?'
    1015, 667, 667, 722, 722, 667, 611, 778, 722, 278, 500, 667, 556, 833, 722, 778, // '@' .. 'O'
    667, 778, 722, 667, 611, 722, 667, 944, 667, 667, 611, 278, 278, 278, 469, 556, // 'P' .. '_'
    333, 556, 556, 500, 556, 556, 278, 556, 556, 222, 222, 500, 222, 833, 556, 556, // '`' .. 'o'
    556, 556, 333, 500, 278, 556, 500, 722, 500, 500, 500, 334, 260, 334, 584       // 'p' .. '~'
};

// Approximate width, in text-space units (fractions of the font size),
// of a single character in Helvetica or Helvetica-Bold. Non-ASCII or
// control characters fall back to the average Helvetica width.
double charWidthUnits(char c, bool bold) {
    unsigned char uc = static_cast<unsigned char>(c);
    int units = 556; // fallback: average Helvetica glyph width
    if (uc >= 0x20 && uc <= 0x7E) {
        units = kHelveticaWidths[uc - 0x20];
    }
    double fraction = units / 1000.0;
    if (bold) {
        fraction *= 1.08; // Helvetica-Bold glyphs run slightly wider
    }
    return fraction;
}

// Total rendered width (in points) of a text run at a given font size.
double measureTextWidth(const std::string& text, double fontSize, bool bold) {
    double totalUnits = 0.0;
    for (char c : text) {
        totalUnits += charWidthUnits(c, bold);
    }
    return totalUnits * fontSize;
}

// Greedily word-wraps text so that no produced line exceeds maxWidth
// points when rendered at the given font size. A single word longer
// than maxWidth is placed on its own line rather than dropped, so no
// content is ever silently lost off the page.
std::vector<std::string> wrapTextToWidth(const std::string& text, double fontSize,
                                          bool bold, double maxWidth) {
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word;
    std::string currentLine;

    while (words >> word) {
        std::string candidate = currentLine.empty() ? word : currentLine + " " + word;
        if (!currentLine.empty() && measureTextWidth(candidate, fontSize, bold) > maxWidth) {
            lines.push_back(currentLine);
            currentLine = word;
        } else {
            currentLine = candidate;
        }
    }
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    if (lines.empty()) {
        lines.push_back("");
    }
    return lines;
}

} // namespace

std::string PDFRenderStrategy::render(const Document& document, const ThemeConfig& theme) const {
    std::ostringstream markup;
    const char SEP = '\x1F'; // ASCII Unit Separator - safe, never appears in user text

    markup << "TITLE" << SEP << "" << SEP << document.getFullName() << "\n";
    if (!document.getJobTitle().empty()) {
        markup << "SUBTITLE" << SEP << "" << SEP << document.getJobTitle() << "\n";
    }
    markup << "THEME" << SEP << theme.themeName << SEP << theme.primaryColor << "\n";

    auto personalInfo = document.findSection(SectionType::PERSONAL_INFO);
    if (personalInfo) {
        for (const auto& field : personalInfo->getFields()) {
            markup << "CONTACT" << SEP << field.first << SEP << field.second << "\n";
        }
    }

    for (const auto& section : document.getSections()) {
        if (!section || section->isEmpty()) {
            continue;
        }
        if (section->getType() == SectionType::PERSONAL_INFO) {
            continue; // already emitted as CONTACT lines above
        }

        markup << "SECTION" << SEP << "" << SEP << section->getTitle() << "\n";

        for (const auto& field : section->getFields()) {
            markup << "FIELD" << SEP << field.first << SEP << field.second << "\n";
        }

        for (const auto& entry : section->getEntries()) {
            markup << "ENTRY_START" << SEP << "" << SEP << "" << "\n";
            for (const auto& kv : entry) {
                markup << "FIELD" << SEP << kv.first << SEP << kv.second << "\n";
            }
            markup << "ENTRY_END" << SEP << "" << SEP << "" << "\n";
        }
    }

    return markup.str();
}

std::vector<PDFRenderStrategy::MarkupLine> PDFRenderStrategy::parseMarkup(const std::string& markup) const {
    std::vector<MarkupLine> lines;
    std::istringstream stream(markup);
    std::string rawLine;
    const char SEP = '\x1F';

    while (std::getline(stream, rawLine)) {
        if (rawLine.empty()) {
            continue;
        }
        MarkupLine ml;
        std::size_t firstSep = rawLine.find(SEP);
        if (firstSep == std::string::npos) {
            ml.tag = rawLine;
            lines.push_back(ml);
            continue;
        }
        ml.tag = rawLine.substr(0, firstSep);
        std::size_t secondSep = rawLine.find(SEP, firstSep + 1);
        if (secondSep == std::string::npos) {
            ml.label = rawLine.substr(firstSep + 1);
        } else {
            ml.label = rawLine.substr(firstSep + 1, secondSep - firstSep - 1);
            ml.value = rawLine.substr(secondSep + 1);
        }
        lines.push_back(ml);
    }
    return lines;
}

std::string PDFRenderStrategy::escapePdfText(const std::string& input) {
    std::string out;
    out.reserve(input.size() + 8);
    for (char c : input) {
        if (c == '\\' || c == '(' || c == ')') {
            out += '\\';
            out += c;
        } else if (static_cast<unsigned char>(c) < 0x20) {
            // Strip raw control characters (including embedded newlines);
            // PDF literal strings must not contain unescaped control bytes.
            continue;
        } else {
            out += c;
        }
    }
    return out;
}

std::string PDFRenderStrategy::buildPdfDocument(const std::vector<MarkupLine>& lines) const {
    const double pageWidth = 612.0;   // US Letter, points
    const double pageHeight = 792.0;
    const double marginLeft = 55.0;
    const double marginRight = 50.0;
    const double marginTop = 742.0;
    const double marginBottom = 50.0;

    RGB primaryColor{0.15, 0.25, 0.45};
    const RGB blackColor{0.0, 0.0, 0.0};
    const RGB grayColor{0.35, 0.35, 0.35};

    std::vector<std::string> pageContents;
    std::string content;
    double y = marginTop;

    auto startNewPage = [&]() {
        pageContents.push_back(content);
        content.clear();
        y = marginTop;
    };

    auto ensureSpace = [&](double needed) {
        if (y - needed < marginBottom) {
            startNewPage();
        }
    };

    // Draws exactly one physical line of text at the current cursor
    // position, then advances the cursor downward by one line height.
    auto drawPhysicalLine = [&](const std::string& text, double fontSize, bool bold,
                                 const RGB& color, double x) {
        ensureSpace(fontSize);
        content += "BT\n";
        content += formatNumber(color.r) + " " + formatNumber(color.g) + " " +
                    formatNumber(color.b) + " rg\n";
        content += std::string("/") + (bold ? "F2" : "F1") + " " + formatNumber(fontSize) + " Tf\n";
        content += formatNumber(x) + " " + formatNumber(y) + " Td\n";
        content += "(" + escapePdfText(text) + ") Tj\n";
        content += "ET\n";
        y -= fontSize * 1.18; // line spacing
    };

    // Word-wraps text to fit within the page's usable width (minus the
    // given left indent) and draws it as one or more physical lines,
    // so long descriptions never run off the right edge of the page.
    auto writeWrapped = [&](const std::string& text, double fontSize, bool bold,
                             const RGB& color, double indent, double gapAfter) {
        double maxWidth = pageWidth - marginLeft - marginRight - indent;
        if (maxWidth < 40.0) {
            maxWidth = 40.0;
        }
        std::vector<std::string> wrapped = wrapTextToWidth(text, fontSize, bold, maxWidth);
        for (const auto& physicalLine : wrapped) {
            drawPhysicalLine(physicalLine, fontSize, bold, color, marginLeft + indent);
        }
        y -= gapAfter;
    };

    for (const auto& line : lines) {
        if (line.tag == "TITLE") {
            writeWrapped(line.value, 22.0, true, primaryColor, 0.0, 10.0);
        } else if (line.tag == "SUBTITLE") {
            writeWrapped(line.value, 13.0, false, grayColor, 0.0, 8.0);
        } else if (line.tag == "CONTACT") {
            std::string text = line.label.empty() ? line.value : (line.label + ": " + line.value);
            writeWrapped(text, 10.0, false, grayColor, 0.0, 3.0);
        } else if (line.tag == "THEME") {
            if (!line.value.empty()) {
                primaryColor = hexToRgb(line.value);
            }
        } else if (line.tag == "SECTION") {
            ensureSpace(26.0);
            y -= 8.0;
            writeWrapped(line.value, 13.0, true, primaryColor, 0.0, 8.0);
        } else if (line.tag == "FIELD") {
            std::string text = line.label.empty() ? line.value : (line.label + ": " + line.value);
            writeWrapped(text, 10.5, false, blackColor, 14.0, 4.0);
        } else if (line.tag == "ENTRY_START") {
            y -= 2.0;
        } else if (line.tag == "ENTRY_END") {
            y -= 8.0;
        }
    }
    pageContents.push_back(content);

    // ---- Assemble PDF object graph ----
    // Object numbering scheme:
    //   1               Catalog
    //   2               Pages (parent)
    //   3               Font F1 (Helvetica)
    //   4               Font F2 (Helvetica-Bold)
    //   5..(5+n-1)      Page objects (n = numPages)
    //   (5+n)..(5+2n-1) Content stream objects (one per page)
    const std::size_t numPages = pageContents.size();
    const std::size_t catalogObj = 1;
    const std::size_t pagesObj = 2;
    const std::size_t fontRegularObj = 3;
    const std::size_t fontBoldObj = 4;
    const std::size_t firstPageObj = 5;
    const std::size_t firstContentObj = firstPageObj + numPages;
    const std::size_t totalObjects = firstContentObj + numPages - 1;

    std::string kidsStr;
    for (std::size_t i = 0; i < numPages; ++i) {
        kidsStr += std::to_string(firstPageObj + i) + " 0 R";
        if (i + 1 < numPages) {
            kidsStr += " ";
        }
    }

    std::vector<std::string> objectBodies(totalObjects + 1); // 1-indexed

    objectBodies[catalogObj] =
        "<< /Type /Catalog /Pages " + std::to_string(pagesObj) + " 0 R >>";
    objectBodies[pagesObj] =
        "<< /Type /Pages /Kids [" + kidsStr + "] /Count " + std::to_string(numPages) + " >>";
    objectBodies[fontRegularObj] =
        "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>";
    objectBodies[fontBoldObj] =
        "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >>";

    for (std::size_t i = 0; i < numPages; ++i) {
        const std::size_t pageObjNum = firstPageObj + i;
        const std::size_t contentObjNum = firstContentObj + i;

        std::string pageDict =
            "<< /Type /Page /Parent " + std::to_string(pagesObj) + " 0 R "
            "/MediaBox [0 0 " + formatNumber(pageWidth) + " " + formatNumber(pageHeight) + "] "
            "/Resources << /Font << /F1 " + std::to_string(fontRegularObj) + " 0 R "
            "/F2 " + std::to_string(fontBoldObj) + " 0 R >> >> "
            "/Contents " + std::to_string(contentObjNum) + " 0 R >>";
        objectBodies[pageObjNum] = pageDict;

        const std::string& streamData = pageContents[i];
        objectBodies[contentObjNum] =
            "<< /Length " + std::to_string(streamData.size()) + " >>\nstream\n" +
            streamData + "endstream";
    }

    // ---- Serialize with correct byte offsets for the xref table ----
    std::string pdf = "%PDF-1.4\n";
    std::vector<std::size_t> offsets(totalObjects + 1, 0);

    for (std::size_t objNum = 1; objNum <= totalObjects; ++objNum) {
        offsets[objNum] = pdf.size();
        pdf += std::to_string(objNum) + " 0 obj\n" + objectBodies[objNum] + "\nendobj\n";
    }

    const std::size_t xrefOffset = pdf.size();
    pdf += "xref\n";
    pdf += "0 " + std::to_string(totalObjects + 1) + "\n";
    pdf += "0000000000 65535 f \n";
    for (std::size_t objNum = 1; objNum <= totalObjects; ++objNum) {
        std::ostringstream offsetStream;
        offsetStream << std::setw(10) << std::setfill('0') << offsets[objNum];
        pdf += offsetStream.str() + " 00000 n \n";
    }

    pdf += "trailer\n";
    pdf += "<< /Size " + std::to_string(totalObjects + 1) +
           " /Root " + std::to_string(catalogObj) + " 0 R >>\n";
    pdf += "startxref\n" + std::to_string(xrefOffset) + "\n";
    pdf += "%%EOF";

    return pdf;
}

bool PDFRenderStrategy::save(const std::string& renderedContent, const std::string& filepath) const {
    std::vector<MarkupLine> lines = parseMarkup(renderedContent);
    std::string pdfBytes = buildPdfDocument(lines);

    std::ofstream out(filepath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }
    out.write(pdfBytes.data(), static_cast<std::streamsize>(pdfBytes.size()));
    bool ok = out.good();
    out.close();
    return ok;
}

std::string PDFRenderStrategy::getFileExtension() const {
    return ".pdf";
}

std::string PDFRenderStrategy::getStrategyName() const {
    return "PDF Render Strategy (native PDF writer)";
}
