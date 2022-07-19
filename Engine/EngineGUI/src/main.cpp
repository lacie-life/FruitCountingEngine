#include "MainWindow.h"
#include <QApplication>
#include <GL/glew.h>
#include <GL/freeglut.h>

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
