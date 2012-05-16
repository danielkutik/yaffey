#ifndef DIALOGIMPORT_H
#define DIALOGIMPORT_H

#include <QDialog>

namespace Ui {
    class DialogImport;
}

class DialogImport : public QDialog {
    Q_OBJECT
    
public:
    explicit DialogImport(QWidget* parent = 0);
    ~DialogImport();
    
    enum Result {
        RESULT_CANCEL,
        RESULT_FILE,
        RESULT_DIRECTORY
    };

private slots:
    void on_pushImportFile_clicked();
    void on_pushImportDirectory_clicked();
    void on_pushCancel_clicked();

private:
    Ui::DialogImport* mUi;
};

#endif  //DIALOGIMPORT_H
