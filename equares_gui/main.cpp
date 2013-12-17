#include <QApplication>
#include "MainWindow.h"
#include "equares_core/equares_core.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
