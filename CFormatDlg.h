#ifndef CFORMATDLG_H
#define CFORMATDLG_H

#include <QDialog>

namespace Ui {
class CFormatDlg;
}

class CFormatDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CFormatDlg(QWidget *parent = nullptr);
    ~CFormatDlg();
	int hideDiskSize;
	void setProgress(int value);
	void setInfo(const char * text);

protected Q_SLOTS:
	void sltValueChanged(int);
    void sltConfirm();

private:
    Ui::CFormatDlg *ui;
	int m_Step;
};

#endif // CFORMATDLG_H
