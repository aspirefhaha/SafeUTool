#include "SafeUTool.h"
#include "ui_SafeUTool.h"
#include <QDebug>
#include "sdcryptostor.h"
#include <QMessageBox>
#include "CFormatDlg.h"
#include "UDiskEncryptConfig.h"

#define REVSECCOUNT	2

SafeUTool::SafeUTool(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SafeUTool)
{
    ui->setupUi(this);
}

SafeUTool::~SafeUTool()
{
    delete ui;
}


void SafeUTool::sltGetCap()
{
    char sName[256] = { 0 };
    char * psName = sName;
    DWORD len = 0;
    int enuret = sd_EnumDevice((char **)&sName, &len);
    if (enuret == SDR_OK) {
        HDEV hDevice = 0;
        int openret = sd_OpenDevice(sName, &hDevice);
        if (openret != SDR_OK) {

            return;
        }

        ULONG cap = 0;
        if (SDR_OK == sd_GetCapcity(hDevice, &cap)) {
            unsigned char tmpdata[1024] = { 0 };
            if (sd_ReadData(hDevice, cap - REVSECCOUNT, sizeof(tmpdata), (UCHAR *)tmpdata) == SDR_OK) {
                if (tmpdata[0] == 0x5a && tmpdata[1] == 0xa5) {
					ui->pbReadSector->setEnabled(true);
                    ui->lbMax->setText(QString::number(cap - REVSECCOUNT));
                }
                else {
					QMessageBox::warning(this, tr("NoAuth"), tr("Device not Authorized!"));
                }
            }
			sd_CloseDevice(hDevice);
			return;

        }
        else {
            QMessageBox::warning(this, tr("GetCap Failed"), tr("Get TF Capacity Failed!"));
            sd_CloseDevice(hDevice);

            return;
        }
        sd_CloseDevice(hDevice);
        return;
    }
    else {
		QMessageBox::warning(this, tr("No Disk"), tr("TF Card Not Found!"));
    }
}

void SafeUTool::sltPartition()
{
    CFormatDlg partitionDlg;
    partitionDlg.setModal(true);
    partitionDlg.exec();
}

void SafeUTool::sltCryptoConfig()
{
    UDiskEncryptConfig diskEncConfDlg;
    diskEncConfDlg.setModal(true);
    diskEncConfDlg.exec();
}

void SafeUTool::sltReadSector()
{
    char sName[256] = { 0 };
    char * psName = sName;
    DWORD len = 0;
    int enuret = sd_EnumDevice((char **)&sName, &len);
    if (enuret == SDR_OK) {
        HDEV hDevice = 0;
        int openret = sd_OpenDevice(sName, &hDevice);
        if (openret != SDR_OK) {

            return;
        }

        ULONG cap = ui->lbMax->text().toUInt();
		ui->teData->setText("");
        //if (SDR_OK == sd_GetCapcity(hDevice, &cap)) {
		if(cap>0){
            unsigned char tmpdata[1024] = { 0 };
            if (sd_ReadData(hDevice, cap, sizeof(tmpdata), (UCHAR *)tmpdata) == SDR_OK) {
                if (tmpdata[0] == 0x5a && tmpdata[1] == 0xa5u) {

                    unsigned int startindex = ui->leStartSector->text().toUInt();
                    unsigned int readnumber = ui->leSectorNumber->text().toUInt();

                    if(readnumber > 100){
                        QMessageBox::warning(this,tr("Too Many"),tr("Too Many Sectors(>100) may Failed!"));
                    }
                    if(startindex + readnumber > cap - REVSECCOUNT){
                        QMessageBox::warning(this,tr("Read Error"),tr("Read Range Overflow!"));
                        sd_CloseDevice(hDevice);

                        return;
                    }
                    else{
                        unsigned char * preadBuf = new unsigned char[readnumber*512];
                        if(preadBuf==nullptr){
                            QMessageBox::warning(this,tr("Memory Failed"),tr("Not Enough Memory"));
                            sd_CloseDevice(hDevice);

                            return;

                        }
                        if(0!=sd_ReadData(hDevice,startindex,readnumber*MAX_SECTOR_SIZE,preadBuf)){
                            QMessageBox::warning(this,tr("Memory Failed"),tr("Not Enough Memory"));
                            delete[] preadBuf;
                            sd_CloseDevice(hDevice);
                            return;

                        }
                        else{
                            QString showtext;
                            for(unsigned int i = 0 ;i<readnumber*512;i++){
                                showtext += QString::number(preadBuf[i],16).toUpper() + " ";
                            }
                            ui->teData->setText(showtext);
                            delete[] preadBuf;
                        }
                    }


                }
                else {
                    QMessageBox::warning(this, tr("NoAuth"), tr("Device not Authorized!"));
                }
            }
            sd_CloseDevice(hDevice);
            return;

        }
        else {
            QMessageBox::warning(this, tr("GetCap Failed"), tr("Get TF Capacity Failed!"));
            sd_CloseDevice(hDevice);

            return;
        }
        sd_CloseDevice(hDevice);
        return;
    }
    else {
        //freshTimer = startTimer(1000);
    }
}
