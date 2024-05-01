#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QSqlRecord>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QVBoxLayout *layout;
    QSqlTableModel *model;
    QTableView *tableView;
    QLabel *label;
    QPushButton *saveButton;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QHBoxLayout *adbuttonslayout;


private:
    QTcpServer* server;
    quint16 nextblocksize = 0;
    Ui::MainWindow *ui;
    std::vector <QTcpSocket*> Sockets;
    QByteArray Data;

    void sendToClient();

public slots:
    virtual void slotNewConnection();
    void slotReadClient();

private slots:
    void onsaveButtonClicked();
    void onaddButtonClicked();
    void ondeleteButtonClicked();
};
#endif // MAINWINDOW_H
