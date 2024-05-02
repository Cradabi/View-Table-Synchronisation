#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("View Table Server");
    this->setMinimumSize(350, 300);
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("postgres");
    db.setUserName("postgres");
    db.setPassword("Your_password");
    if (!db.open()) {
        qDebug() << "Не удалось подключиться к базе данных.";
        exit(0);
    }
    // Создаем модель данных для таблицы
    model = new QSqlTableModel;
    model->setTable("syncho_table");
    model->setEditStrategy(QSqlTableModel::OnRowChange);
    model->select();

    // Создаем виджет QTableView и устанавливаем модель данных
    tableView = new QTableView;
    tableView->verticalHeader()->setVisible(false);
    tableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    tableView->setModel(model);

    QObject::connect(model, &QSqlTableModel::dataChanged, [&](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
            if (model->submitAll()) {
                qDebug() << "Changes submitted successfully";
            } else {
                qDebug() << "Failed to submit changes";
            }
        });

    // Соединяем сигнал нажатия кнопки с обновлением данных в модел

    label = new QLabel("Сервер", this);
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
    server = new QTcpServer(this);
    if(!server->listen(QHostAddress::Any, 2323)){
        server->close();
        return;
    }
    connect(server, SIGNAL(newConnection()), SLOT(slotNewConnection()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendToClient()
{
    qDebug() << "sending";
    QVector<QVector<QString>> vec;
    for (int i = 0; i < model->rowCount(); ++i) {
        QVector <QString> cur_vect;
        QString id = model->record(i).value("id").toString();
        QString name = model->record(i).value("name").toString();
        QString cost = model->record(i).value("cost").toString();
        //qDebug() << id << name << cost;
        cur_vect.append(id);
        cur_vect.append(name);
        cur_vect.append(cost);
        vec.append(cur_vect);
    }
    for(int i = 0; i < model->rowCount(); i++){
        qDebug() << vec[i][0] << vec[i][1] << vec[i][2];
    }
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_11);
    out << quint16(0) << vec;
    out.device()->seek(0);
    out << quint16(Data.size()-sizeof(quint16));
    for(int i = 0; i < Sockets.size(); i++){
        Sockets[i]->write(Data);
    }
}

void MainWindow::slotNewConnection()
{
    qDebug() << "connected";
    QTcpSocket* pClientSocket = server->nextPendingConnection();
    connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));
    connect(pClientSocket, SIGNAL(readyRead()), SLOT(slotReadClient()));
    Sockets.push_back(pClientSocket);
    sendToClient();
}

void MainWindow::slotReadClient()
{
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
            QVector<QVector<QString>> vect;
            in >> vect;
            nextblocksize = 0;
            int rcount = model->rowCount();
            for(int i = 0; i < rcount; i++){
                model->removeRow(rcount - 1 - i);
                model->select();
            }

            for(int i = 0; i < vect.size(); i++){
                model->insertRow(i);
                model->setData(model->index(i, 0), vect[i][0].toInt());
                model->setData(model->index(i, 1), vect[i][1]);
                model->setData(model->index(i, 2), vect[i][2].toInt());

            }
            //ui->textBrowser->append(str);
            sendToClient();
            break;
        }
    }else{
    }
}


void MainWindow::onsaveButtonClicked(){
    qDebug() << "saveButton clicked";
    sendToClient();
}

void MainWindow::onaddButtonClicked(){
    qDebug() << "addButton clicked";
    model->insertRow(model->rowCount());



}

void MainWindow::ondeleteButtonClicked(){
    qDebug() << "deleteButton clicked";
    qDebug() << model->rowCount();
    int rowCount = model->rowCount();
    if (rowCount > 0)
    {
        model->removeRow(rowCount - 1);
        model->select();
    }

    qDebug() << model->rowCount();

}

