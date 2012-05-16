#include "DialogImport.h"
#include "ui_DialogImport.h"

DialogImport::DialogImport(QWidget* parent) : QDialog(parent),
                                              mUi(new Ui::DialogImport) {
    mUi->setupUi(this);
}

DialogImport::~DialogImport() {
    delete mUi;
}

void DialogImport::on_pushImportFile_clicked() {
    done(RESULT_FILE);
}

void DialogImport::on_pushImportDirectory_clicked() {
    done(RESULT_DIRECTORY);
}

void DialogImport::on_pushCancel_clicked() {
    done(RESULT_CANCEL);
}
