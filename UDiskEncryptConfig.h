#ifndef UDISKENCRYPTCONFIG_H
#define UDISKENCRYPTCONFIG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class UDiskEncryptConfig; }
QT_END_NAMESPACE

class UDiskEncryptConfig : public QDialog
{
    Q_OBJECT

public:
    UDiskEncryptConfig(QWidget *parent = nullptr);
    ~UDiskEncryptConfig();

protected slots:
    void sltConfirm();
    void sltNoEnc(bool);
    void sltSM4(bool);
	void sltAES(bool);

private:
    Ui::UDiskEncryptConfig *ui;
};
#endif // UDISKENCRYPTCONFIG_H
