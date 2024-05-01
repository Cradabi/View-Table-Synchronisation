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
#include <QAbstractTableModel>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MyTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MyTableModel(QObject *parent = nullptr);
    void setData(const QVector<QVector<QVariant>>& data);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant get_data(int row, int col, int role = Qt::DisplayRole);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVector<QVector<QVariant>> modelData;
};



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QVBoxLayout *layout;
    //QSqlTableModel model;
    QTableView *tableView;
    QLabel *label;
    QPushButton *saveButton;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QHBoxLayout *adbuttonslayout;
    MyTableModel* model;

private:
    QByteArray Data;
    QTcpSocket* socket;
    quint16 nextblocksize = 0;
    Ui::MainWindow *ui;

private slots:
    void slotReadyRead();
    void slotError(QAbstractSocket::SocketError);
    void slotSendToServer();
    void slotConnected();
    void onsaveButtonClicked();
    void onaddButtonClicked();
    void ondeleteButtonClicked();
};
#endif // MAINWINDOW_H
