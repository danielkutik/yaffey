#include <QDebug>

#include <QFileDialog>
#include <QFileInfo>
#include <QCloseEvent>

#include "DialogFastboot.h"
#include "ui_DialogFastboot.h"

#ifdef _WIN32
    static const QString fastboot = "fastboot.exe";
#else
    static const QString fastboot = "fastboot";
#endif  //_WIN32

static const char* RED_CROSS = ":/icons/icons/red_cross.png";
static const char* GREEN_TICK = ":/icons/icons/green_tick.png";

DialogFastboot::DialogFastboot(QWidget* parent) : QDialog(parent),
                                                  mUi(new Ui::DialogFastboot) {
    mUi->setupUi(this);

    mGotFastbootFile = false;
    mGotImageFile = false;
    mGotDeviceSelected = false;

    mUi->boxPartition->addItem("boot");
    mUi->boxPartition->addItem("system");
    mUi->boxPartition->addItem("recovery");
}

DialogFastboot::~DialogFastboot() {
    delete mUi;
}

void DialogFastboot::closeEvent(QCloseEvent* event) {
    if (event) {
        event->ignore();
        hide();
    }
}

void DialogFastboot::on_pushBrowseFastboot_clicked() {
    QString filename = QFileDialog::getOpenFileName(this, "Get fastboot location", ".", fastboot);
    if (filename.length() > 0) {
        mUi->lineFastbootFile->setText(filename);
    }
}

void DialogFastboot::on_pushBrowseImage_clicked() {
    QString filename = QFileDialog::getOpenFileName(this, "Get fastboot location", ".");
    if (filename.length() > 0) {
        mUi->lineImageFile->setText(filename);
    }
}

void DialogFastboot::on_lineFastbootFile_textChanged(const QString& text) {
    QFileInfo fileInfo(text);
    if (fileInfo.fileName() == fastboot && fileInfo.exists()) {
        mGotFastbootFile = true;
    } else {
        mGotFastbootFile = false;
    }

    analyzeSelections();
}

void DialogFastboot::on_lineImageFile_textChanged(const QString& text) {
    QFileInfo fileInfo(text);
    if (fileInfo.exists()) {
        mGotImageFile = true;
    } else {
        mGotImageFile = false;
    }

    analyzeSelections();
}

void DialogFastboot::on_pushFlash_clicked() {
    qDebug() << "Flashing...";

    analyzeSelections();

    QString fastbootFilename = mUi->lineFastbootFile->text();

    //create fastboot process and setup signals
    mProcess = new QProcess(this);
    connect(mProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(on_processStdOutput()));
    connect(mProcess, SIGNAL(readyReadStandardError()), this, SLOT(on_processStdError()));
    connect(mProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_processFinished(int, QProcess::ExitStatus)));

    //setup arguments
    QString partition = mUi->boxPartition->currentText();
    QStringList args;
    args.append("flash");
    args.append(partition);

    //start process
    mProcess->start(fastbootFilename, args);
    if (mProcess->waitForStarted()) {

    } else {
/*        QMessageBox::critical(this, windowTitle(), KFailedToStart.arg(KProgram));
        ui->statusBar->showMessage(KAnalysisFailed);*/
    }
}

void DialogFastboot::on_pushClose_clicked() {
    hide();
}

void DialogFastboot::on_listDevices_itemChanged(QListWidgetItem* item) {
    qDebug() << "on_listDevices_itemChanged";
    if (item) {
        mGotDeviceSelected = true;
    } else {
        mGotDeviceSelected = false;
    }

    analyzeSelections();
}

void DialogFastboot::on_processStdOutput() {
    qDebug() << "on_processStdOutput";

    QByteArray output = mProcess->readAllStandardOutput();
    mUi->textOutput->setTextColor(Qt::black);
    mUi->textOutput->append(output);
}

void DialogFastboot::on_processStdError() {
    qDebug() << "on_processStdError";

    QByteArray output = mProcess->readAllStandardError();
    mUi->textOutput->setTextColor(Qt::red);
    mUi->textOutput->append(output);
}

void DialogFastboot::on_processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << "on_processFinished";

    mUi->textOutput->setTextColor(Qt::blue);
    if (exitStatus == QProcess::NormalExit) {
        mUi->textOutput->append(" -- process finished, code: " + QString::number(exitCode) + ", status: NormalExit");
    } else {
        mUi->textOutput->append(" -- process finished, code: " + QString::number(exitCode) + ", status: CrashExit");
    }
}

void DialogFastboot::on_boxPartition_currentIndexChanged(const QString& text) {
    analyzeSelections();
}

void DialogFastboot::analyzeSelections() {
    QString commandLine;

    if (mGotFastbootFile) {
        //execute fastboot with devices parameter and parse output

        mUi->labelFastbootFileStatus->setPixmap(QPixmap(QString::fromUtf8(GREEN_TICK)));
        commandLine += fastboot;
    } else {
        mUi->labelFastbootFileStatus->setPixmap(QPixmap(QString::fromUtf8(RED_CROSS)));
    }

    commandLine += " flash ";
    commandLine += mUi->boxPartition->currentText() + " ";

    if (mGotImageFile) {
        mUi->labelImageFileStatus->setPixmap(QPixmap(QString::fromUtf8(GREEN_TICK)));
        commandLine += mUi->lineImageFile->text();
    } else {
        mUi->labelImageFileStatus->setPixmap(QPixmap(QString::fromUtf8(RED_CROSS)));
    }

    mUi->lineCommandLine->setText(commandLine);
}
