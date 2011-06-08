#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QWidget>
#include <QListWidgetItem>

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

private:
    Ui::MainWindow *ui;
    void loadScreenInfos();
};

#endif // MAINWINDOW_HPP
