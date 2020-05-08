#include "Window15Puzzle.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TForm15Puzzle w;
    w.show();
    return a.exec();
}
