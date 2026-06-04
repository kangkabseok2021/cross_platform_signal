#pragma once
#include <QWizard>

class TutorialWizard : public QWizard {
    Q_OBJECT
public:
    explicit TutorialWizard(QWidget* parent = nullptr);
};
