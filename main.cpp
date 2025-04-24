#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    bool dbg = false;
    if (argc > 1 && std::string(argv[1]) == "dbg")
    {
        dbg = true;
    }
    MainWindow window(dbg);
    window.show();
    return app.exec();
}
