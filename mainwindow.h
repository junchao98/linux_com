#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
\



#include <QtSerialPort/QSerialPort>

#include <QtSerialPort/QSerialPortInfo>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
       void on_pushButton_clicked();

   //    void on_pushButton_2_clicked();
       void readMyCom();
  //  int my_set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);

       void on_pushButton_clear_clicked();
       void _5v_curr_data();

       void all_curr_data();

       void i12_curr_data();
        void  u5_vol_data();


        void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;

      void paintEvent(QPaintEvent *);
       void  clearn_data(int num);

      QImage image;
     int fd;
      QTimer *timer;//（用于计时）

      QTimer *timer_all_cur;
      QTimer *timer_drow;
       QTimer *timer_i12;

        QTimer *timer_u5;

QSerialPort *my_serialPort;//(实例化一个指向串口的指针，可以用于访问串口)
 QByteArray requestData;//（用于存储从串口那读取的数据）



      struct data_set{

          double fault_bit;
          int  data_pre;


      }all_curr_set,i12_curr_set,u5_vol_set;
};

#endif // MAINWINDOW_H
