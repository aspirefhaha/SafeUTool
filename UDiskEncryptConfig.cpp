#include "UDiskEncryptConfig.h"
#include "ui_UDiskEncryptConfig.h"

#include <QMessageBox>
#include <QDebug>
#include <QCryptographicHash>
#include <QDateTime>
#include "sdcryptostor.h"
#define REVSECCOUNT	2

UDiskEncryptConfig::UDiskEncryptConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UDiskEncryptConfig)
{
    ui->setupUi(this);
}

UDiskEncryptConfig::~UDiskEncryptConfig()
{
    delete ui;
}

void UDiskEncryptConfig::sltNoEnc(bool)
{
    ui->lePassword->setText("");
    ui->lePassword->setEnabled(false);
}

void UDiskEncryptConfig::sltAES(bool)
{
	ui->lePassword->setEnabled(true);
}
void UDiskEncryptConfig::sltSM4(bool)
{
    ui->lePassword->setEnabled(true);
}

void UDiskEncryptConfig::sltConfirm()
{
    if (ui->rbSM4->isChecked() || ui->rbAES->isChecked()) {
        if (ui->lePassword->text().isEmpty()) {
            QMessageBox::warning(this, tr("Encrypt Key Empty"), tr("Please Input Encrypt/Decrypt Key"));
            return;
        }
    }
    QString md5;
    QString key=ui->lePassword->text();
    QByteArray bb;
    bb = QCryptographicHash::hash ( key.toUtf8(), QCryptographicHash::Md5 );
    //bb.at(0)==0x3f;
	QByteArray cc;
	QDateTime qdt1 = QDateTime::currentDateTime();
	QString timeStr = qdt1.toString("yyyyMMddhhmmsszzz");
	cc = QCryptographicHash::hash(timeStr.toUtf8(), QCryptographicHash::Md5);
	bb.append(cc);
    md5.append(bb.toHex());
    char * pdata = bb.data();
    //qDebug() << md5 << " result: " << (bb.at(0)=='\x41') << "data:" << QString::number(pdata[0],16)
    //        << " " << QString::number(pdata[1],16);

    char sName[256] = { 0 };
    char * psName = sName;
    DWORD len = 0;
    int enuret = sd_EnumDevice((char **)&sName,&len);
    if(enuret == SDR_OK){
        HDEV hDevice = 0;
        int openret = sd_OpenDevice(sName, &hDevice);
        if(openret != SDR_OK){
            QMessageBox::warning(this,tr("Open Failed"),tr("Open TF Card!"));
            return;
        }

        ULONG cap = 0;
        if(SDR_OK == sd_GetCapcity(hDevice,&cap)){
            unsigned char tmpdata[1024] = {0};
            if (sd_ReadData(hDevice, cap - REVSECCOUNT , sizeof(tmpdata), (UCHAR *)tmpdata) == SDR_OK) {
                if (tmpdata[0] != 0x5a || tmpdata[1] != 0xa5) {
                    QMessageBox::warning(this, tr("Operation Failed"),tr("Not Authorized!"),QMessageBox::Yes , QMessageBox::Yes);
                    sd_CloseDevice(hDevice);
                    return;

                }
            }
            if(tmpdata[2]!=0){
                if(QMessageBox::warning(this,tr("Operation warning!"),tr("Already Set Encrypt Key,Continue?"),
                                        QMessageBox::Yes | QMessageBox::No,QMessageBox::Yes) == QMessageBox::No){
                    sd_CloseDevice(hDevice);
                    return;

                }
            }

            if (ui->rbNoEnc->isChecked()) {
                tmpdata[2] = 0;
            }
            else if(ui->rbSM4->isChecked()){
                tmpdata[2] = 0x1;
                memcpy(&(tmpdata[3]), pdata, 32);
                memcpy(&(tmpdata[512]), pdata, 32);
            }
			else if (ui->rbAES->isChecked()) {
				tmpdata[2] = 0x2;
				memcpy(&(tmpdata[3]), pdata, 32);
				memcpy(&(tmpdata[512]), pdata, 32);
			}

            if(sd_WriteData(hDevice,cap - REVSECCOUNT ,sizeof(tmpdata),(UCHAR *)tmpdata)!=SDR_OK){
                QMessageBox::warning(this,tr("Write Failed"),tr("Set Authority Data Failed!"));
                sd_CloseDevice(hDevice);
            }
            else{
                QMessageBox::information(this,tr("Info"),tr("Config Succeed!"));
                sd_CloseDevice(hDevice);
                QDialog::accept();
            }
        }
        else{
            QMessageBox::warning(this,tr("GetCap Failed"),tr("Get TF Capacity Failed!"));
            sd_CloseDevice(hDevice);
            QDialog::accept();
            return;
        }
    } else {

        QMessageBox::warning(this,tr("Enum Failed"),tr("Not Found TF Card!"));
        return;
    }

}
