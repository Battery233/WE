#include "bmw.h"

myBMW::myBMW(QWidget *parent) :
    QWidget(parent)
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(1);//间隔，单位ms
    connect(m_updateTimer,SIGNAL(timeout()),this,SLOT(UpdateAngle()));
    m_updateTimer->start();//启动定时器

    m_angle = 0;
    m_outerRadius = 0;
    setWindowFlags(Qt::FramelessWindowHint);//无窗体
    setAttribute(Qt::WA_TranslucentBackground);//背景透明
}

void myBMW::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);//设置反锯齿
  drawUnderCircle(&painter);//画外部圆
  drawBMW(&painter);//画宝马
}

void myBMW::drawUnderCircle(QPainter *painter)
{
  painter->save();

  m_outerRadius = width() > height() ? (qreal)height()/2-4 : (qreal)width()/2-4;//求最小值

  QPointF TopLeft(rect().center().x() - m_outerRadius,rect().center().y() - m_outerRadius);
  QPointF BottomRight(rect().center().x() + m_outerRadius,rect().center().y() + m_outerRadius);
  QRectF CircleRect(TopLeft,BottomRight);//大圆矩形

  painter->setPen(Qt::NoPen);
  QRadialGradient CircleGradient(CircleRect.center(),m_outerRadius,CircleRect.center());//设置渐变
  CircleGradient.setColorAt(0.0,OUTER_CIRCLE_START_COLOR);
  CircleGradient.setColorAt(1.0,OUTER_CIRCLE_END_COLOR);
  painter->setBrush(CircleGradient);

  painter->drawEllipse(CircleRect);//画椭圆

  painter->restore();
}

void myBMW::drawBMW(QPainter *painter)
{

  painter->save();

  //坐标转换的方法和下面直接用painter的rotate方法一样
  //QTransform t;
  //t.translate(rect().center().x(),rect().center().y());// move to center
  //t.rotate(m_angle,Qt::ZAxis);//绕Z轴旋转
  //painter->setTransform(t);

  painter->translate(rect().center().x(),rect().center().y());// move to center
  painter->rotate(m_angle);//旋转

  qreal InnerRadius = m_outerRadius * RADIUS_FACTOR;//内半径
  QPointF tTopLeft( -InnerRadius,-InnerRadius);
  QPointF tBottomRight(InnerRadius,InnerRadius);
  QRectF  tRect(tTopLeft,tBottomRight);

  qreal dAngle = 90 * 16;
  qreal StartAngle = 0;

  painter->setPen(Qt::NoPen);
  for(int AngleIndex = 0; AngleIndex < 4;AngleIndex++)
  {
      //交叉蓝色白色
      QRadialGradient PieGradient(tRect.center(),m_outerRadius,tRect.center());
      if(AngleIndex%2)//蓝色
      {
          PieGradient.setColorAt(0.0,BLUE_CIRCLE_START_COLOR);
          PieGradient.setColorAt(1.0,BLUE_CIRCLE_END_COLOR);
      }
      else//白色
      {
         PieGradient.setColorAt(0.0,WHITE_CIRCLE_START_COLOR);
         PieGradient.setColorAt(1.0,WHITE_CIRCLE_END_COLOR);
      }
      painter->setBrush(PieGradient);
      painter->drawPie(tRect,StartAngle,dAngle);

      //角度增加90度
      StartAngle += dAngle;
  }

  painter->restore();
}

void myBMW::UpdateAngle()
{
  m_angle += 1;
  if(m_angle > 360)
  {
    m_angle = 0;
  }

  //m_angle = ((m_angle + 1) % 360);与上面几行功能一样
  update();//刷新控件，会调用paintEvent函数
}
