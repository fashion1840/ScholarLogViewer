#include "scholarlogviewer.h"

#include "CommonHelper.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //CommonHelper::setStyle("");

    ScholarLogViewer w;
    w.show();

    return a.exec();
}
