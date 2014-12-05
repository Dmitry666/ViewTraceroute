/**************************************************************************
** GameX license
** Author: Oznabikhin Dmitry
** Email: gamexgroup@gmail.com
** Filename: main.cpp
** Copyright (c) 2014 GameX. All rights reserved.
**************************************************************************/

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
