#include "scholarlogviewer.h"

#include "CommonHelper.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CommonHelper::loadQss(":/qss/style1.qss");

    ScholarLogViewer w;
    w.show();

    return a.exec();
}
