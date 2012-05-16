#ifndef DIALOGFASTBOOT_H
#define DIALOGFASTBOOT_H

#include <QDialog>

#include <QListWidgetItem>
#include <QProcess>

namespace Ui {
    class DialogFastboot;
}

class DialogFastboot : public QDialog {
    Q_OBJECT
    
public:
    explicit DialogFastboot(QWidget* parent = 0);
    ~DialogFastboot();

protected:
    //from QDialog
    void closeEvent(QCloseEvent* event);
    
private slots:
    void on_pushBrowseFastboot_clicked();
    void on_pushBrowseImage_clicked();
    void on_lineFastbootFile_textChanged(const QString& text);
    void on_lineImageFile_textChanged(const QString& text);
    void on_pushFlash_clicked();
    void on_pushClose_clicked();
    void on_listDevices_itemChanged(QListWidgetItem* item);
    void on_processStdOutput();
    void on_processStdError();
    void on_processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_boxPartition_currentIndexChanged(const QString &arg1);

private:
    void analyzeSelections();

private:
    Ui::DialogFastboot* mUi;
    bool mGotFastbootFile;
    bool mGotImageFile;
    bool mGotDeviceSelected;
    QProcess* mProcess;
};

#endif  //DIALOGFASTBOOT_H
