#ifndef ALGCONFIGFORM_H
#define ALGCONFIGFORM_H

#include <QWidget>
#include "Common/common.h"

namespace Ui {
class AlgConfigForm;
}

class AlgConfigForm : public QWidget
{
    Q_OBJECT

public:
    explicit AlgConfigForm(QWidget *parent = nullptr);
    ~AlgConfigForm();

    void readConfig();
    void saveConfig();
    InThresholdParam getAlgParam() const;

signals:
    void algParamChanged(const InThresholdParam &param);

private slots:
    void on_applyButton_clicked();
    void on_saveButton_clicked();
    void on_resetButton_clicked();

private:
    Ui::AlgConfigForm *ui;
    void updateUiFromParam(const InThresholdParam &param);
    void updateParamFromUi(InThresholdParam &param);
};

#endif // ALGCONFIGFORM_H
