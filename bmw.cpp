#include "bmw.h"

myBMW::myBMW(QWidget *parent) :
    QWidget(parent)
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(1);//�������λms
    connect(m_updateTimer,SIGNAL(timeout()),this,SLOT(UpdateAngle()));
    m_updateTimer->start();//������ʱ��

    m_angle = 0;
    m_outerRadius = 0;
    setWindowFlags(Qt::FramelessWindowHint);//�޴���
    setAttribute(Qt::WA_TranslucentBackground);//����͸��
}

void myBMW::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);//���÷����
  drawUnderCircle(&painter);//���ⲿԲ
  drawBMW(&painter);//������
}

void myBMW::drawUnderCircle(QPainter *painter)
{
  painter->save();

  m_outerRadius = width() > height() ? (qreal)height()/2-4 : (qreal)width()/2-4;//����Сֵ

  QPointF TopLeft(rect().center().x() - m_outerRadius,rect().center().y() - m_outerRadius);
  QPointF BottomRight(rect().center().x() + m_outerRadius,rect().center().y() + m_outerRadius);
  QRectF CircleRect(TopLeft,BottomRight);//��Բ����

  painter->setPen(Qt::NoPen);
  QRadialGradient CircleGradient(CircleRect.center(),m_outerRadius,CircleRect.center());//���ý���
  CircleGradient.setColorAt(0.0,OUTER_CIRCLE_START_COLOR);
  CircleGradient.setColorAt(1.0,OUTER_CIRCLE_END_COLOR);
  painter->setBrush(CircleGradient);

  painter->drawEllipse(CircleRect);//����Բ

  painter->restore();
}

void myBMW::drawBMW(QPainter *painter)
{

  painter->save();

  //����ת���ķ���������ֱ����painter��rotate����һ��
  //QTransform t;
  //t.translate(rect().center().x(),rect().center().y());// move to center
  //t.rotate(m_angle,Qt::ZAxis);//��Z����ת
  //painter->setTransform(t);

  painter->translate(rect().center().x(),rect().center().y());// move to center
  painter->rotate(m_angle);//��ת

  qreal InnerRadius = m_outerRadius * RADIUS_FACTOR;//�ڰ뾶
  QPointF tTopLeft( -InnerRadius,-InnerRadius);
  QPointF tBottomRight(InnerRadius,InnerRadius);
  QRectF  tRect(tTopLeft,tBottomRight);

  qreal dAngle = 90 * 16;
  qreal StartAngle = 0;

  painter->setPen(Qt::NoPen);
  for(int AngleIndex = 0; AngleIndex < 4;AngleIndex++)
  {
      //������ɫ��ɫ
      QRadialGradient PieGradient(tRect.center(),m_outerRadius,tRect.center());
      if(AngleIndex%2)//��ɫ
      {
          PieGradient.setColorAt(0.0,BLUE_CIRCLE_START_COLOR);
          PieGradient.setColorAt(1.0,BLUE_CIRCLE_END_COLOR);
      }
      else//��ɫ
      {
         PieGradient.setColorAt(0.0,WHITE_CIRCLE_START_COLOR);
         PieGradient.setColorAt(1.0,WHITE_CIRCLE_END_COLOR);
      }
      painter->setBrush(PieGradient);
      painter->drawPie(tRect,StartAngle,dAngle);

      //�Ƕ�����90��
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

  //m_angle = ((m_angle + 1) % 360);�����漸�й���һ��
  update();//ˢ�¿ؼ��������paintEvent����
}
