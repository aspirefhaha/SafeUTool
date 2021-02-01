#include "SafeUTool.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	QTranslator * pTranslator = new QTranslator();
	if (pTranslator->load("./SafeUTool_zh_CN.qm")) {
		a.installTranslator(pTranslator);
	}
    SafeUTool w;
    w.show();
    return a.exec();
}
