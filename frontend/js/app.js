(function () {
  "use strict";

  let selectedTheme = "classic";
  let photoData = null;
  let photoMimeType = null;

  const els = {
    themeList: document.getElementById("theme-list"),
    previewFrame: document.getElementById("preview-frame"),
    statusMessage: document.getElementById("status-message"),
    usePhoto: document.getElementById("usePhoto"),
    photoUploadArea: document.getElementById("photo-upload-area"),
    photoInput: document.getElementById("photoInput"),
    photoPreview: document.getElementById("photoPreview"),
    photoPreviewImg: document.getElementById("photoPreviewImg"),
    removePhoto: document.getElementById("removePhoto"),
    experienceList: document.getElementById("experience-list"),
    educationList: document.getElementById("education-list"),
    projectsList: document.getElementById("projects-list"),
    experienceTemplate: document.getElementById("experience-template"),
    educationTemplate: document.getElementById("education-template"),
    projectTemplate: document.getElementById("project-template"),
  };

  function showStatus(message, type) {
    els.statusMessage.textContent = message;
    els.statusMessage.className = "status-message " + type;
    els.statusMessage.classList.remove("hidden");
    setTimeout(function () {
      els.statusMessage.classList.add("hidden");
    }, 5000);
  }

  function addEntryFromTemplate(listEl, templateEl) {
    const clone = templateEl.content.cloneNode(true);
    const card = clone.querySelector(".entry-card");
    card.querySelector(".remove-entry").addEventListener("click", function () {
      card.remove();
    });
    listEl.appendChild(clone);
  }

  function collectEntries(listEl, fieldMap) {
    const entries = [];
    listEl.querySelectorAll(".entry-card").forEach(function (card) {
      const entry = [];
      for (const key in fieldMap) {
        const input = card.querySelector('[data-field="' + key + '"]');
        const value = input ? input.value.trim() : "";
        if (value) {
          entry.push({ key: fieldMap[key], value: value });
        }
      }
      if (entry.length > 0) {
        entries.push(entry);
      }
    });
    return entries;
  }

  function buildDocumentJson() {
    const fullName = document.getElementById("fullName").value.trim();
    const jobTitle = document.getElementById("jobTitle").value.trim();
    const summary = document.getElementById("summary").value.trim();
    const skillsText = document.getElementById("skills").value.trim();

    const personalFields = [];
    const email = document.getElementById("email").value.trim();
    const phone = document.getElementById("phone").value.trim();
    const location = document.getElementById("location").value.trim();
    const link = document.getElementById("link").value.trim();
    if (email) personalFields.push({ key: "Email", value: email });
    if (phone) personalFields.push({ key: "Phone", value: phone });
    if (location) personalFields.push({ key: "Location", value: location });
    if (link) personalFields.push({ key: "Link", value: link });

    const sections = [];

    if (personalFields.length > 0) {
      sections.push({
        type: "PERSONAL_INFO",
        title: "Personal Information",
        order: 0,
        fields: personalFields,
        entries: [],
      });
    }

    if (summary) {
      sections.push({
        type: "SUMMARY",
        title: "Professional Summary",
        order: 1,
        fields: [{ key: "", value: summary }],
        entries: [],
      });
    }

    const experienceEntries = collectEntries(els.experienceList, {
      role: "Role",
      company: "Company",
      duration: "Duration",
      details: "Details",
    });
    if (experienceEntries.length > 0) {
      sections.push({
        type: "WORK_EXPERIENCE",
        title: "Work Experience",
        order: 2,
        fields: [],
        entries: experienceEntries,
      });
    }

    const educationEntries = collectEntries(els.educationList, {
      degree: "Degree",
      institution: "Institution",
      year: "Year",
      notes: "Notes",
    });
    if (educationEntries.length > 0) {
      sections.push({
        type: "EDUCATION",
        title: "Education",
        order: 3,
        fields: [],
        entries: educationEntries,
      });
    }

    if (skillsText) {
      const skillLines = skillsText.split("\n").map(function (s) { return s.trim(); }).filter(Boolean);
      sections.push({
        type: "SKILLS",
        title: "Skills",
        order: 4,
        fields: skillLines.map(function (skill) { return { key: "", value: skill }; }),
        entries: [],
      });
    }

    const projectEntries = collectEntries(els.projectsList, {
      project: "Project",
      stack: "Stack",
      details: "Details",
    });
    if (projectEntries.length > 0) {
      sections.push({
        type: "PROJECTS",
        title: "Projects",
        order: 5,
        fields: [],
        entries: projectEntries,
      });
    }

    const doc = {
      fullName: fullName || "Untitled Candidate",
      jobTitle: jobTitle,
      photo: null,
      sections: sections,
    };

    if (els.usePhoto.checked && photoData) {
      doc.photo = {
        mimeType: photoMimeType || "image/jpeg",
        data: photoData,
      };
    }

    return doc;
  }

  async function apiPost(path, body) {
    const response = await fetch(API_BASE_URL + path, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(body),
    });
    return response.json();
  }

  async function loadThemes() {
    try {
      const response = await fetch(API_BASE_URL + "/api/themes");
      const data = await response.json();
      els.themeList.innerHTML = "";

      data.themes.forEach(function (theme, index) {
        const card = document.createElement("label");
        card.className = "theme-card" + (index === 0 ? " selected" : "");
        card.innerHTML =
          '<input type="radio" name="theme" value="' + theme.id + '"' +
          (index === 0 ? " checked" : "") + ">" +
          '<span class="theme-swatch" style="background:' + theme.primaryColor + '"></span>' +
          '<div class="theme-info"><h3>' + theme.name + '</h3><p>' + theme.description + "</p></div>";

        card.querySelector('input[type="radio"]').addEventListener("change", function () {
          selectedTheme = theme.id;
          document.querySelectorAll(".theme-card").forEach(function (c) {
            c.classList.remove("selected");
          });
          card.classList.add("selected");
        });

        els.themeList.appendChild(card);
      });

      if (data.themes.length > 0) {
        selectedTheme = data.themes[0].id;
      }
    } catch (err) {
      els.themeList.innerHTML =
        '<p class="loading-text">Could not load themes. Make sure the backend server is running:<br><code>cv_builder --server</code></p>';
    }
  }

  async function previewCV() {
    const documentData = buildDocumentJson();
    try {
      const result = await apiPost("/api/preview", {
        document: documentData,
        theme: selectedTheme,
      });

      if (!result.success) {
        showStatus(result.message || "Preview failed.", "error");
        return;
      }

      els.previewFrame.innerHTML = "";
      const iframe = document.createElement("iframe");
      iframe.srcdoc = result.html;
      els.previewFrame.appendChild(iframe);
      showStatus("Preview updated.", "success");
    } catch (err) {
      showStatus("Could not reach backend. Start the server with: cv_builder --server", "error");
    }
  }

  async function exportCV(format) {
    const documentData = buildDocumentJson();
    try {
      const result = await apiPost("/api/export", {
        document: documentData,
        theme: selectedTheme,
        format: format,
      });

      if (!result.success) {
        showStatus(result.message || "Export failed.", "error");
        return;
      }

      let blob;
      if (format === "pdf" && result.contentBase64) {
        const binary = atob(result.contentBase64);
        const bytes = new Uint8Array(binary.length);
        for (let i = 0; i < binary.length; i++) {
          bytes[i] = binary.charCodeAt(i);
        }
        blob = new Blob([bytes], { type: result.mimeType });
      } else {
        blob = new Blob([result.content], { type: result.mimeType });
      }

      const url = URL.createObjectURL(blob);
      const link = document.createElement("a");
      link.href = url;
      link.download = result.filename;
      link.click();
      URL.revokeObjectURL(url);

      showStatus("Exported " + result.filename, "success");
    } catch (err) {
      showStatus("Could not reach backend. Start the server with: cv_builder --server", "error");
    }
  }

  function handlePhotoToggle() {
    if (els.usePhoto.checked) {
      els.photoUploadArea.classList.remove("hidden");
    } else {
      els.photoUploadArea.classList.add("hidden");
      photoData = null;
      photoMimeType = null;
      els.photoPreview.classList.add("hidden");
      els.photoInput.value = "";
    }
  }

  function handlePhotoSelect(event) {
    const file = event.target.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = function (e) {
      const dataUrl = e.target.result;
      const commaIndex = dataUrl.indexOf(",");
      photoMimeType = dataUrl.substring(5, dataUrl.indexOf(";"));
      photoData = dataUrl.substring(commaIndex + 1);
      els.photoPreviewImg.src = dataUrl;
      els.photoPreview.classList.remove("hidden");
    };
    reader.readAsDataURL(file);
  }

  function handleRemovePhoto() {
    photoData = null;
    photoMimeType = null;
    els.photoInput.value = "";
    els.photoPreview.classList.add("hidden");
  }

  document.getElementById("addExperience").addEventListener("click", function () {
    addEntryFromTemplate(els.experienceList, els.experienceTemplate);
  });
  document.getElementById("addEducation").addEventListener("click", function () {
    addEntryFromTemplate(els.educationList, els.educationTemplate);
  });
  document.getElementById("addProject").addEventListener("click", function () {
    addEntryFromTemplate(els.projectsList, els.projectTemplate);
  });

  document.getElementById("previewBtn").addEventListener("click", previewCV);
  document.getElementById("exportHtmlBtn").addEventListener("click", function () { exportCV("html"); });
  document.getElementById("exportPdfBtn").addEventListener("click", function () { exportCV("pdf"); });

  els.usePhoto.addEventListener("change", handlePhotoToggle);
  els.photoInput.addEventListener("change", handlePhotoSelect);
  els.removePhoto.addEventListener("click", handleRemovePhoto);

  addEntryFromTemplate(els.experienceList, els.experienceTemplate);
  addEntryFromTemplate(els.educationList, els.educationTemplate);
  loadThemes();
})();
