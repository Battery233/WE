#ifndef BMW_H
#define BMW_H
//�ڲ�Բ�뾶��������
#define RADIUS_FACTOR 0.8

//�ⲿԲ��ʼ��ֹͣ��ɫ
#define OUTER_CIRCLE_START_COLOR QColor(65,65,65)
#define OUTER_CIRCLE_END_COLOR QColor(89,89,89)

//�ڲ�Բ����ɫ
#define BLUE_CIRCLE_START_COLOR QColor(0,133,203)
#define BLUE_CIRCLE_END_COLOR QColor(0,118,177)
//�ڲ�Բ�İ�ɫ
#define WHITE_CIRCLE_START_COLOR QColor(255,255,255)
#define WHITE_CIRCLE_END_COLOR QColor(233,233,233)

#include <QWidget>
#include <QtGui>

class myBMW : public QWidget
{
    Q_OBJECT
public:
    explicit myBMW(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);

    //��дsizeHint()
    QSize sizeHint() const
    {
      return QSize(300,300);
    }

private:
    void drawUnderCircle(QPainter* painter);//����Բ

    void drawBMW(QPainter* painter);//������

private:
    QTimer* m_updateTimer;//��ʱ��ʱ��

    qreal   m_angle;    //��ת�Ƕ�
    qreal   m_outerRadius;//��뾶

private slots:
    void UpdateAngle();//�Զ���ۣ����½Ƕ���ת

};


#endif // BMW_H
