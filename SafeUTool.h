#ifndef SAFEUTOOL_H
#define SAFEUTOOL_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class SafeUTool; }
QT_END_NAMESPACE

class SafeUTool : public QMainWindow
{
    Q_OBJECT

public:
    SafeUTool(QWidget *parent = nullptr);
    ~SafeUTool();


protected slots:
    void sltGetCap();
    void sltPartition();
    void sltCryptoConfig();
    void sltReadSector();

private:
    Ui::SafeUTool *ui;
};
#endif // SAFEUTOOL_H
