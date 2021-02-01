#include "CFormatDlg.h"
#include "ui_CFormatDlg.h"
#ifdef WIN32
#include "dl_meth.h"
#include <Windows.h>
#include <QMessageBox>

#include "sdcryptostor.h"
#endif
CFormatDlg::CFormatDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFormatDlg),
	hideDiskSize(10*1000),
	m_Step(0)
{
    ui->setupUi(this);
	ui->progressBar->hide();
}

void CFormatDlg::setProgress(int value)
{
	ui->progressBar->setValue(value);
}
void CFormatDlg::setInfo(const char * text)
{
	ui->lbStatus->setText(text);
}

void SetProgress(void* p, int n)
{
	CFormatDlg * pDlg = (CFormatDlg*)p;
	
	pDlg->setProgress(n);
}

void ShowInfo(void* p, const char *szInfo)
{
	CFormatDlg * pDlg = (CFormatDlg*)p;
	QString showInfo = QString::fromUtf16((const char16_t *)szInfo);
	pDlg->setInfo(showInfo.toUtf8().data());
	
}
void ShowInfoEx(void* p, const char *szInfo)
{
	CFormatDlg * pDlg = (CFormatDlg*)p;
	pDlg->setInfo(szInfo);
}


void CFormatDlg::sltConfirm()
{
#ifdef WIN32
	DL_METHOD* m_methNor = NULL;
	unsigned int i = 0, iRet = 0, vpid = 0;
	int flags = DLF_TYPE_SCSI | DLF_DRIVE_REMOVABLE;
	HMODULE m_hLibrary = LoadLibraryA("dl_meth.dll");
	char * szDevList = NULL;
	if (m_hLibrary)
	{
		FDL_MethodEx fun = (FDL_MethodEx)GetProcAddress(m_hLibrary, "DL_MethodEx");

		if (fun)
		{
			m_methNor = fun(FDL_TYPE_SD | FDL_MODE_NOR);
			if (m_methNor == NULL)
			{
				return;
			}
			
		}
	}
	else {
		ui->lbStatus->setText(tr("Load Library Failed"));
		return;
	}


	iRet = m_methNor->enumerate(&szDevList, flags, vpid);
	if (iRet)
	{
		ui->lbStatus->setText(tr("TF Card Not Found"));
		FreeModule(m_hLibrary);
		return ;
	}

	
	int ret = 0;
	void *ctx = NULL;
	unsigned char buf[256] = { 0 };
	char szInfo[256] = { 0 };
	char szDev[32] = { 0 };
	SCAN_DATA sd;
	DL_CB cb = { this, SetProgress, ShowInfo };
	

	
	ret = m_methNor->open(szDevList, DLF_TYPE_SCSI, &ctx);
	if (ret)
	{
		ui->lbStatus->setText(tr("Open TF Card Failed:0x%1").arg(QString::number(ret,16)));
		FreeModule(m_hLibrary);
		return;
	}
	memset(&sd, 0, sizeof(SCAN_DATA));
	sd.bUser = 1;
	sd.reserved_cap = hideDiskSize;
	sd.reserved_cap *= 2048;
	sd.step = 2;

	ret = m_methNor->ctrl(ctx, DLF_CTL_CHECK_SD_MPFLAG, NULL, &cb);
	if (ret)
	{
		m_methNor->close(ctx);
		if (ret == 0xFFFFFFFF)
		{
			ui->lbStatus->setText(tr( "Unplug and Plug TF Card"));
			ui->pbConfirm->setText(tr("Next"));
			ui->progressBar->show();
			ui->lbSize->hide();
			ui->hsSize->hide();
		}
		else
		{
			ui->lbStatus->setText(tr("Partition Device Failed:0x%1").arg(QString::number(ret,16)));

		}
		FreeModule(m_hLibrary);
		return;
	}
	ui->pbConfirm->setEnabled(false);
	ret = m_methNor->ctrl(ctx, DLF_CTL_HIGH_SCAN, &sd, &cb);
	if (ret)
	{
		m_methNor->close(ctx);
		ui->lbStatus->setText(tr("Partition Device Failed:0x%1").arg(QString::number(ret, 16)));
		FreeModule(m_hLibrary);
		
		return;
	}
	m_methNor->close(ctx);

	
    QMessageBox::information(this, tr("Need Replug"), tr("Replug TF Device Again!"), QMessageBox::Yes , QMessageBox::Yes);
	
	FreeModule(m_hLibrary);
	{
		char sName[256] = { 0 };
		char * psName = sName;
		DWORD len = 0;
		int enuret = sd_EnumDevice((char **)&sName, &len);
		if (enuret == SDR_OK) {
			HDEV hDevice = 0;
			int openret = sd_OpenDevice(sName, &hDevice);
			if (openret != SDR_OK) {
				QMessageBox::warning(this, tr("Open Failed"), tr("Open TF Card For Clear Fs Failed!"));
				return;
			}


			char tmpdata[512] = { 0 };
			memset(tmpdata, 0, 512);

			if (sd_WriteData(hDevice, 0, sizeof(tmpdata), (UCHAR *)tmpdata) != SDR_OK) {
				QMessageBox::warning(this, tr("Write Failed"), tr("Clear Fs Data Failed!"));
			}
			else {
				QMessageBox::information(this, tr("Partition"), tr("Partition Succeed!"));
			}
			sd_CloseDevice(hDevice);

		}
		else {

			QMessageBox::warning(this, tr("Enum Failed"), tr("Not Found TF Card!"));
			return;
		}
	}
	
#endif
	accept();
}

CFormatDlg::~CFormatDlg()
{
    delete ui;
}

void CFormatDlg::sltValueChanged(int newValue)
{
	ui->lbSize->setText(QString("%1 G").arg(newValue));
	hideDiskSize = newValue * 1000;
}

