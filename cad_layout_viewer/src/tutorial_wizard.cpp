#include "tutorial_wizard.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWizardPage>

static QWizardPage* make_page(const QString& title, const QString& body) {
    auto* page = new QWizardPage;
    page->setTitle(title);
    auto* lbl = new QLabel(body);
    lbl->setWordWrap(true);
    auto* lay = new QVBoxLayout;
    lay->addWidget(lbl);
    page->setLayout(lay);
    return page;
}

TutorialWizard::TutorialWizard(QWidget* parent) : QWizard(parent) {
    setWindowTitle(tr("CAD Viewer – Einführung"));
    setWizardStyle(QWizard::ModernStyle);

    addPage(make_page(
        tr("1. Layout öffnen"),
        tr("Klicken Sie auf <b>Datei → Öffnen</b> und wählen Sie eine "
           "JSON-Layout-Datei. Das Chip-Koordinatensystem wird automatisch "
           "in die Ansicht eingepasst.")));

    addPage(make_page(
        tr("2. Layer erkunden"),
        tr("Im <b>Layer-Panel</b> links können Sie einzelne Lagen ein- und "
           "ausblenden. Jeder Layer erhält eine eigene Farbe. Zoomen Sie mit "
           "dem Mausrad, pannen Sie mit der mittleren Maustaste.")));

    addPage(make_page(
        tr("3. Simulation konfigurieren"),
        tr("Im <b>Eigenschaften-Panel</b> rechts stellen Sie Zeitschritt Δt "
           "und die Anzahl Iterationsschritte ein. Achten Sie auf die "
           "CFL-Warnung: bei zu großem Δt divergiert der Solver.")));

    addPage(make_page(
        tr("4. Heatmap interpretieren"),
        tr("<b>Blau</b> = kalt, <b>Grün</b> = mittel, <b>Rot</b> = heiß. "
           "Die Heatmap wird halbtransparent über das Layout geblendet. "
           "Hotspots erscheinen als rote Regionen – kritische Stellen im "
           "Chip-Design.")));

    addPage(make_page(
        tr("5. Hotspots exportieren"),
        tr("Die Hotspot-Tabelle rechts unten listet alle erkannten Regionen "
           "mit Schwerpunkt, Maximaltemperatur und Fläche. Über "
           "<b>Datei → Exportieren</b> können Sie die Tabelle als CSV "
           "speichern.")));
}
