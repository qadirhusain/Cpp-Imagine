#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFont appFont("Segoe UI", 10); // or 12, depending on your style
    QGuiApplication::setFont(appFont);

    // Set VS Code–like stylesheet globally
    app.setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1e1e;
            color: #d4d4d4;
            font-family: "Segoe UI";
            font-size: 13px;
        }

        QMenuBar {
            background-color: #2d2d2d;
            color: #cccccc;
        }

        QMenuBar::item {
            spacing: 3px;
            padding: 4px 10px;
            background: transparent;
        }

        QMenuBar::item:selected {
            background: #3e3e3e;
        }

        QMenu {
            background-color: #252526;
            color: #d4d4d4;
            border: 1px solid #333;
        }

        QMenu::item {
            padding: 5px 24px;
            background-color: transparent;
        }

        QMenu::item:selected {
            background-color: #094771;
        }

        QMenu::separator {
            height: 1px;
            background: #333;
            margin: 5px 0;
        }

        QToolTip {
            color: white;
            background-color: #252526;
            border: 1px solid white;
            font: "Segoe UI";
        }
    )");

    MainWindow w;
    w.show();
    return app.exec();
}
