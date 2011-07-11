#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QWidget>
#include <QListWidgetItem>
#include <QSystemTrayIcon>
#include <QMenu>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_lstDevices_itemDoubleClicked(QListWidgetItem *item);
    void handleTrayMenu(QAction* action);

private:
    Ui::MainWindow *ui;
    void loadScreenInfos();
    QSystemTrayIcon* tray;
    QMenu* trayMenu;
};

#endif // MAINWINDOW_HPP
