#include "mainwindow.h"
#include "ui_mainwindow.h"

MyTableModel::MyTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

Qt::ItemFlags MyTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool MyTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    modelData[index.row()][index.column()] = value;
    emit dynamic_cast<QAbstractItemModel*>(this)->dataChanged(index, index);
    return true;
}


void MyTableModel::setData(const QVector<QVector<QVariant>>& data)
{
    modelData = data;
}

int MyTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return modelData.size();
}

int MyTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return modelData.isEmpty() ? 0 : modelData.first().size();
}

QVariant MyTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    return modelData[index.row()][index.column()];
}

QVariant MyTableModel::get_data(int row, int col, int role){
    return modelData[row][col];
}

QVariant MyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return "id";
        case 1:
            return "name";
        case 2:
            return "cost";
        }
    }

    return QVariant();
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //connecttodb
    this->setMinimumSize(350, 300);

    // Создаем виджет QTableView и устанавливаем модель данных
    tableView = new QTableView;
    tableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    tableView->verticalHeader()->setVisible(false);
    //tableView->setModel(model);

    model = new MyTableModel(this);


    // Заполнение модели данными из QVector
    QVector<QVector<QVariant>> data = {
    };
    model->setData(data);
    tableView->setModel(dynamic_cast<QAbstractItemModel*>(model));


    label = new QLabel("Клиент", this);
    addButton = new QPushButton("+", this);
    deleteButton = new QPushButton("-", this);
    saveButton = new QPushButton("Сохранить", this);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onsaveButtonClicked);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onaddButtonClicked);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::ondeleteButtonClicked);


    adbuttonslayout = new QHBoxLayout();
    adbuttonslayout->addWidget(deleteButton);
    adbuttonslayout->addWidget(addButton);

    // Создание вертикального лэйаута
    layout = new QVBoxLayout();
    layout->addWidget(label);
    layout->addWidget(tableView);
    layout->addLayout(adbuttonslayout);
    layout->addWidget(saveButton);

    // Создание центрального виджета и установка лэйаута
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
    //start connection

    socket = new QTcpSocket(this);

    socket -> connectToHost("localhost", 2323);
    connect(socket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotReadyRead()
{
    qDebug() << "start read";
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_5_11);
    if(in.status() == QDataStream::Ok){
        for(;;){
            if(nextblocksize == 0){
                if(pClientSocket->bytesAvailable() < 2){
                    break;
                }
                in >> nextblocksize;
            }
            if (pClientSocket->bytesAvailable() < nextblocksize){
                break;
            }
            QVector<QVector<QString>> vec;
            QVector<QVector<QVariant>> vec_var;
            QVector<QVariant> vec_cur;
            in >> vec;
            nextblocksize = 0;
            for(int i=0; i < vec.size(); i++){
                vec_cur = {};
                vec_cur.append(vec[i][0].toInt());
                vec_cur.append(vec[i][1]);
                vec_cur.append(vec[i][2].toInt());
                vec_var.append(vec_cur);
            }
            model = new MyTableModel(this);
            model->setData(vec_var);
            tableView->setModel(dynamic_cast<QAbstractItemModel*>(model));
            //ui->textBrowser->append(str);
            break;
        }
    }else{
    }
}

void MainWindow::slotError(QAbstractSocket::SocketError)
{
    //ui->textBrowser->append("Slot_Error");
}

void MainWindow::slotSendToServer()
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_11);
    QVector<QVector<QString>> vec_str;
    QVector<QString> vec_cur;
    for (int i = 0; i < model->modelData.size(); i++){
        vec_cur = {};
        vec_cur.append(model->modelData[i][0].toString());
        vec_cur.append(model->modelData[i][1].toString());
        vec_cur.append(model->modelData[i][2].toString());
        vec_str.append(vec_cur);
    }
    for(int i = 0; i < vec_str.size(); i++){
        qDebug() << vec_str[i][0] << vec_str[i][1] << vec_str[i][2];
    }
    out << quint16(0) << vec_str;
    out.device()->seek(0);
    out << quint16(Data.size()-sizeof(quint16));
    socket->write(Data);
    //ui->lineEdit->clear();
}

void MainWindow::slotConnected()
{
    //ui->textBrowser->append("Slot_Connected");
}

void MainWindow::onsaveButtonClicked(){
    qDebug() << "saveButton clicked";
    slotSendToServer();
}

void MainWindow::onaddButtonClicked(){
    qDebug() << "addButton clicked";
}

void MainWindow::ondeleteButtonClicked(){
    qDebug() << "deleteButton clicked";
}
