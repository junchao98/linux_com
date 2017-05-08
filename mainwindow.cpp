#include "mainwindow.h"
#include "ui_mainwindow.h"


////////////////////
#include <QTimer>
#include <QDebug>
#include <stdlib.h>
#include <QPaintEvent> //用于绘画事件
#include <QtGui> //引入用到的控件

#include <QtSerialPort/QSerialPort>

#include <QtSerialPort/QSerialPortInfo>


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define inf 0x3f3f3f3f
#define DATA_MAX 500

int busy =1;

int  point_num =15;         //用折线点的个数
double data_cache[DATA_MAX]={0};

int fault_bit = 10;
int nByte=0;
char buffer[540]={0};


 QString  ser_data;





int my_set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);







        timer_drow = new QTimer();
        connect( timer_drow, SIGNAL( timeout() ), this, SLOT( update()) );
        timer_drow->start(1000);//每秒读一次

}

MainWindow::~MainWindow()
{

  \
    delete ui;
}


int my_set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    if  ( tcgetattr( fd,&oldtio)  !=  0) {
        perror("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag  |=  CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch( nBits )
    {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }

    switch( nSpeed )
    {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
            break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
    }
    if( nStop == 1 )
        newtio.c_cflag &=  ~CSTOPB;
    else if ( nStop == 2 )
        newtio.c_cflag |=  CSTOPB;
        newtio.c_cc[VTIME]  = 0;
        newtio.c_cc[VMIN] = 0;
        tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }

    //	printf("set done!\n\r");
    return 0;
}







void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    static  double tem_max=0;
    QString tem_str=NULL;

   int pointx=35,pointy=280;//确定坐标轴起点坐标，这里定义(35,280)
   int width=580-pointx,height=260;//确定坐标轴宽度跟高度 上文定义画布为600X300，宽高依此而定。

   //绘制坐标轴 坐标轴原点(35，280)

   QColor backColor = qRgb(255,255,255);    //画布初始化背景色使用白色
   image.fill(backColor);//对画布进行填充

    painter.setRenderHint(QPainter::Antialiasing, true);//设置反锯齿模式，好看一点
   painter.drawRect(5,5,600-5,300-5);//外围的矩形，从(5,5)起，到(590,290)结束，周围留了5的间隙。

   painter.drawLine(pointx,pointy,width+pointx,pointy);//坐标轴x宽度为width
   painter.drawLine(pointx,pointy-height,pointx,pointy);//坐标轴y高度为height



  // srand(time(NULL));

   //获得数据中最大值和最小值、平均数
   int n=point_num;//n为数据个数
   double sum=0;
   double ave=0;
   double _ma=0;//数组里的最大值
   int _mi=inf;


    QString ser_data;   //读取的串口一行的数据

  //int a[n];//数据储存在数组a中，大小为n

   //for(int i=0;i<n;i++)a[i]=rand()%40+20;

    int maxpos=0,minpos=0;

   for(int i=0;i<n;i++)
   {
       sum+=data_cache[i];
       if(data_cache[i]>_ma){
           _ma=data_cache[i];
           maxpos=i;
       }
       if(data_cache[i]<_mi){
           _mi=data_cache[i];
           minpos=i;
       }
   }
   ave=sum/n;//平均数

   double kx=(double)width/(n-1); //x轴的系数
   double ky=(double)height/_ma;//y方向的比例系数
   QPen pen,penPoint;
   pen.setColor(Qt::black);
   pen.setWidth(2);

   penPoint.setColor(Qt::blue);
   penPoint.setWidth(5);
   for(int i=0;i<n-1;i++)
   {
       //由于y轴是倒着的，所以y轴坐标要pointy-a[i]*ky 其中ky为比例系数
       painter.setPen(pen);//黑色笔用于连线
       painter.drawLine(pointx+kx*i,pointy-data_cache[i]*ky+5,pointx+kx*(i+1),pointy-data_cache[i+1]*ky+5);
       painter.setPen(penPoint);//蓝色的笔，用于标记各个点
       painter.drawPoint(pointx+kx*i,pointy-data_cache[i]*ky+5);
   }
   painter.drawPoint(pointx+kx*(n-1),pointy-data_cache[n-1]*ky+5);//绘制最后一个点

    //绘制平均线
    QPen penAve;
    penAve.setColor(Qt::red);//选择红色
    penAve.setWidth(2);
    penAve.setStyle(Qt::DotLine);//线条类型为虚线
    painter.setPen(penAve);
    painter.drawLine(pointx,pointy-ave*ky,pointx+width,pointy-ave*ky);

//    //绘制最大值和最小值
//    QPen penMaxMin;
//    penMaxMin.setColor(Qt::darkGreen);//暗绿色
//    painter.setPen(penMaxMin);
//    painter.drawText(pointx+kx*maxpos-kx,pointy-a[maxpos]*ky-5,
//                     "最大值"+QString::number(_ma));
//    painter.drawText(pointx+kx*minpos-kx,pointy-a[minpos]*ky+15,
//                     "最小值"+QString::number(_mi));

//    penMaxMin.setColor(Qt::red);
//    penMaxMin.setWidth(7);
//    painter.setPen(penMaxMin);
//    painter.drawPoint(pointx+kx*maxpos,pointy-a[maxpos]*ky);//标记最大值点
//    painter.drawPoint(pointx+kx*minpos,pointy-a[minpos]*ky);//标记最小值点


   //绘制刻度线
   QPen penDegree;
   penDegree.setColor(Qt::black);
   penDegree.setWidth(2);
   painter.setPen(penDegree);
   //画上x轴刻度线

   for(int i=0;i<10;i++)//分成10份
   {
       //选取合适的坐标，绘制一段长度为4的直线，用于表示刻度
       painter.drawLine(pointx+(i+1)*width/10,pointy,pointx+(i+1)*width/10,pointy+4);
       painter.drawText(pointx+(i+0.65)*width/10,
                        pointy+10,QString::number((int)((i+1)*((double)n/10))));
   }
   //y轴刻度线
   double _maStep=(double)_ma/10;//y轴刻度间隔需根据最大值来表示
   for(int i=0;i<10;i++)
   {
       //代码较长，但是掌握基本原理即可。
       //主要就是确定一个位置，然后画一条短短的直线表示刻度。

       painter.drawLine(pointx,pointy-(i+1)*height/10,
                        pointx-4,pointy-(i+1)*height/10);
       painter.drawText(pointx-20,pointy-(i+0.85)*height/10,
                        QString::number((int)(_maStep*(i+1))));
   }


   if(_ma>tem_max)
     {

       tem_max=_ma;
       tem_str= QString::number(tem_max,3,2);
      ui->label_max->setText("MAX="+tem_str);

   }
}





void MainWindow::readMyCom()
{


    QString tem;
    //QString data_cp;

    static QString cache="i 5";

    int flag=0;
    static  int i=0;

    static  double cp=100;

    int data_long=200;    //how long once
    int data_pre=3;
    QString data_last;




    nByte = read(fd, buffer, data_long);



    ser_data=buffer;

    //data="i Z=2.499170";

    ui->textBrowser_tem->append(ser_data);


     tem= ui->comboBox_choses->currentText();     //取出选择的显示

     if(tem=="temp")

     {
         tem="wd=";
         fault_bit=2;
          qDebug()<<"temp";

     }
     if(tem=="humi")
     {

         tem="sd=";
         fault_bit=2;
           qDebug()<<"humi";

     }
     if(tem=="5v_current")
     {
         tem="i 5";
         fault_bit= 0;
         cp=5;
         data_pre=6;
          qDebug()<<"5v_curr";
     }


     //if(tem=="temp")tem="wd";



     if((i-1)>0 && data_cache[i-2]==0)i=0;


     if(tem != cache )                //判断选定要显示的参数是否变化
     {
         cache = tem;  //c存入缓存
         clearn_data(DATA_MAX);
     }



    if (ser_data.indexOf(tem)>=0)            //读取传感器数据
    {
         flag=ser_data.indexOf(tem);
         flag=flag+4;
         qDebug()<<"flag= ";
         qDebug()<<flag;
        //tem= data.mid(flag,flag+data_pre);
           tem="";
         for(int i=0;i<data_pre;i++ )      //get
         {
              tem += buffer[flag+i];
              if(buffer[flag+i]=='\n' || buffer[flag+i]==' ')
              {
                  tem = data_last;
                  break;
              }

          }



      //  if((data_long-flag)>=0)  //数据大小 long限定        tem.toDouble()<90  &&
      //  {


          if((tem.toDouble()-cp) < fault_bit ||(tem.toDouble()-cp)<fault_bit)  //两次数据落差限定
          {
                data_cache[i] =tem.toDouble();

                if(data_cache[i]==0.00)data_cache[i]==data_cache[-i];
                ui->textBrowser_data->append(tem);
                i+=1;
                qDebug()<< "i="<<i<<"\n";
           }

    }
    else
    {
     qDebug()<< "one_cut";
    }


       flag=0;
       if(i==point_num) i=0;

        memset(buffer,0,data_pre);


}





void MainWindow::on_pushButton_clicked()
{



     //baudrate = ui->comboBox_baudRate->currentText().toInt() ;

    if((fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY))<0)printf("open is failed");

    my_set_opt(fd,115200, 8, 'N', 1);


 timer =  new QTimer();

 connect(timer,SIGNAL(timeout()),this,SLOT(readMyCom()));

 timer->start(1000);



 timer_all_cur =  new QTimer();

 connect(timer_all_cur,SIGNAL(timeout()),this,SLOT(all_curr_data()));

 timer_all_cur->start(1000);


         timer_i12 =  new QTimer();

         connect(timer_i12,SIGNAL(timeout()),this,SLOT(i12_curr_data()));

         timer_i12->start(1000);



                 timer_u5 =  new QTimer();

                 connect(timer_u5,SIGNAL(timeout()),this,SLOT(u5_vol_data()));

                 timer_u5->start(1000);

}


void MainWindow::all_curr_data()
{

     int flag=0;
     double cp=2.48;
     QString tem="i Z";

         all_curr_set.data_pre=4;
        all_curr_set.fault_bit=0.2;


   if (ser_data.indexOf(tem)>=0)            //读取传感器数据
   {
        flag=ser_data.indexOf(tem);
        flag=flag+4;
        qDebug()<<"flag_all_curr= ";
        qDebug()<<flag;
       //tem= data.mid(flag,flag+data_pre);
          tem="";
        for(int i=0;i<all_curr_set.data_pre;i++ )      //get
        {
             tem += buffer[flag+i];
              qDebug()<< "all_c="<<tem;

             if(buffer[flag+i]=='\n' || buffer[flag+i]==' ')
             {
                 tem ="2.486279";
                 break;
             }

          }



       if(tem.toDouble()<90)  //数据大小 long限定
       {


           if((tem.toDouble()-cp)< all_curr_set.fault_bit ||(tem.toDouble()-cp)< all_curr_set.fault_bit)  //两次数据落差限定
           {

               ui->label_5v_c->setText("all_curr="+tem);
           }
           else
           {

                ui->label_5v_c->setText("all_curr=2.48");
           }
       }
   }
   else
   {
    qDebug()<< "one_cut";
   }


}

void MainWindow::i12_curr_data()
{

     int flag=0;
     double cp=0.056;
     QString tem="i12";

         i12_curr_set.data_pre=5;
         i12_curr_set.fault_bit=0.2;


   if (ser_data.indexOf(tem)>=0)            //读取传感器数据
   {
        flag=ser_data.indexOf(tem);
        flag=flag+4;
        qDebug()<<"flag_all_curr= ";
        qDebug()<<flag;
       //tem= data.mid(flag,flag+data_pre);
          tem="";
        for(int i=0;i<all_curr_set.data_pre;i++ )      //get
        {
             tem += buffer[flag+i];
              qDebug()<< "all_c="<<tem;

             if(buffer[flag+i]=='\n' || buffer[flag+i]==' ')
             {
                 tem ="2.486279";
                 break;
             }

          }



       if(tem.toDouble()<90)  //数据大小 long限定
       {


           if((tem.toDouble()-cp)< all_curr_set.fault_bit ||(tem.toDouble()-cp)< all_curr_set.fault_bit)  //两次数据落差限定
           {

               ui->label_i12->setText("i12_curr="+tem);
           }
           else
           {

                ui->label_i12->setText("i12_curr=0.056");
           }
       }
   }
   else
   {
    qDebug()<< "one_cut";
   }


}

void MainWindow::u5_vol_data()
{

     int flag=0;
     double cp=5;
     QString tem="u 5";

         u5_vol_set.data_pre=5;
         u5_vol_set.fault_bit=0.2;


   if (ser_data.indexOf(tem)>=0)            //读取传感器数据
   {
        flag=ser_data.indexOf(tem);
        flag=flag+4;
        qDebug()<<"flag_all_curr= ";
        qDebug()<<flag;
       //tem= data.mid(flag,flag+data_pre);
          tem="";
        for(int i=0;i<all_curr_set.data_pre;i++ )      //get
        {
             tem += buffer[flag+i];
              qDebug()<< "all_c="<<tem;

             if(buffer[flag+i]=='\n' || buffer[flag+i]==' ')
             {
                 tem ="2.486279";
                 break;
             }

          }



       if(tem.toDouble()<90)  //数据大小
       {


           if((tem.toDouble()-cp)< all_curr_set.fault_bit ||(tem.toDouble()-cp)< all_curr_set.fault_bit)  //两次数据落差限定
           {

               ui->label_u5->setText("5V_out="+tem);
           }
           else
           {

                ui->label_u5->setText("5V_out=0.01");
           }
       }
   }
   else
   {
    qDebug()<< "one_cut";
   }


}


void MainWindow::clearn_data(int num)
{

    for(int i=0;i<num;i++)
    {

        data_cache[i]=0;         //清除数据

    }


}
\

void MainWindow::on_pushButton_clear_clicked()
{

    clearn_data(100);
}

void MainWindow::on_pushButton_2_clicked()
{
    QString cmd;
    \
    char * send_cmd ;

    cmd =  ui->lineEdit->text();
   QByteArray ba = cmd.toLatin1();

   send_cmd = ba.data();

    write(fd,send_cmd,1);

  qDebug()<<send_cmd;
}
