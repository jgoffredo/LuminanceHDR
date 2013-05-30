/**
 * This file is a part of Luminance HDR package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2012 Franco Comida
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ---------------------------------------------------------------------- 
 *
 * @author Franco Comida <fcomida@users.sourceforge.net>
 */

#include <QDebug>

#include <QGraphicsScene>
#include <QApplication>
#include <QPainter>

#include "Viewers/IGraphicsPixmapItem.h"
#include "AntiGhostingWidget.h"

AntiGhostingWidget::AntiGhostingWidget(QGraphicsScene *parent, QImage *mask): 
    QGraphicsScene(parent),
    m_agMask(mask),
    m_savedMask(NULL),
    m_agcursorPixmap(NULL),
    m_mx(0),
    m_my(0),
    m_drawingMode(BRUSH) 
{
    qDebug() << "AntiGhostingWidget::AntiGhostingWidget";
    //set internal brush values to their default
    m_brushAddMode = true;
    setBrushSize(32);
    m_previousPixmapSize = -1;
    setBrushStrength(255);
    m_previousPixmapStrength = -1;
    m_previousPixmapColor = QColor();
    fillAntiGhostingCursorPixmap();
    //setMouseTracking(true);
    mPixmap = new IGraphicsPixmapItem();
    addItem(mPixmap);
    QPainter p(m_agMask);
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(QColor().red(), Qt::SolidPattern));
    p.drawEllipse(QPoint(100,100), 200, 200);
    mPixmap->setPixmap(QPixmap::fromImage(*m_agMask));
} 

AntiGhostingWidget::~AntiGhostingWidget()
{
    qDebug() << "~AntiGhostingWidget::AntiGhostingWidget";
    if (m_agcursorPixmap)
        delete m_agcursorPixmap;
    if (m_savedMask)
        delete m_savedMask;
}

void AntiGhostingWidget::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "AntiGhostingWidget::mousePressEvent";
    if (event->buttons() == Qt::MidButton) {
        QApplication::setOverrideCursor( QCursor(Qt::ClosedHandCursor) );
        m_mousePos = event->globalPos();
    }
    else if (event->buttons() == Qt::LeftButton) {
        if (m_drawingMode == PATH) {
            QPoint relativeToWidget = event->pos();
            int sx = relativeToWidget.x()/m_scaleFactor - m_mx;
            int sy = relativeToWidget.y()/m_scaleFactor - m_my;
            QPoint scaled(sx,sy);
            m_firstPoint = m_lastPoint = m_currentPoint = scaled;
            m_path = QPainterPath(m_firstPoint);
            m_drawingPathEnded = false;
        }
        m_timerid = this->startTimer(0);
    }
    event->ignore();
}

void AntiGhostingWidget::mouseMoveEvent(QMouseEvent *event)
{
    qDebug() << "AntiGhostingWidget::mouseMoveEvent";
    if (event->buttons() == Qt::MidButton) {
        //moving mouse with middle button pans the preview
        QPoint diff = (event->globalPos() - m_mousePos);
        if (event->modifiers() == Qt::ShiftModifier)
            diff *= 5;
        emit moved(diff);
        m_mousePos = event->globalPos();
    }
    else if (event->buttons() == Qt::LeftButton && m_drawingMode == PATH) {
        QPointF relativeToWidget = event->pos();
        int sx = relativeToWidget.x()/m_scaleFactor - m_mx;
        int sy = relativeToWidget.y()/m_scaleFactor - m_my;
        QPoint scaled(sx,sy);
        m_currentPoint = scaled;
    }
    event->ignore();
}

void AntiGhostingWidget::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "AntiGhostingWidget::mouseReleaseEvent";
    if (event->button() == Qt::LeftButton) {
        this->killTimer(m_timerid);
        if (m_drawingMode == PATH) {
            m_path.lineTo(m_firstPoint);
            m_drawingPathEnded = true;
            drawPath();
        }
    }
    else if (event->button() == Qt::MidButton) {
        QApplication::restoreOverrideCursor();      
        if (m_drawingMode == BRUSH) {
            fillAntiGhostingCursorPixmap();
            //this->unsetCursor();
            //this->setCursor(*m_agcursorPixmap);
        }
        else {
            //this->unsetCursor();
            //this->setCursor( QCursor(Qt::CrossCursor) );
        }
    }
    event->ignore();
}

void AntiGhostingWidget::paintEvent(QPaintEvent *event)
{
    QRect paintrect = event->rect();
    QRect srcrect = QRect(paintrect.topLeft().x()/m_scaleFactor - m_mx, paintrect.topLeft().y()/m_scaleFactor - m_my,
                          paintrect.width()/m_scaleFactor, paintrect.height()/m_scaleFactor);
    QPainter p(0);
    p.drawImage(paintrect, *m_agMask, srcrect);
    mPixmap->setPixmap(QPixmap::fromImage(*m_agMask));
}


void AntiGhostingWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "AntiGhostingWidget::resizeEvent";
    //m_scaleFactor = (float)(event->size().width())/(float)(m_agMask->size().width());
    //m_scaleFactor = ((PreviewWidget *) parent())->getScaleFactor();
}

void AntiGhostingWidget::scale(PreviewWidget *pw)
{
    m_scaleFactor = pw->getScaleFactor();
}

void AntiGhostingWidget::timerEvent(QTimerEvent *) 
{
    (m_drawingMode == BRUSH) ? drawWithBrush() : drawPath();
}

void AntiGhostingWidget::drawWithBrush()
{
    //QPoint relativeToWidget = mapFromScene(QCursor::pos());
    QPoint relativeToWidget = QCursor::pos();
    int sx = relativeToWidget.x()/m_scaleFactor - m_mx;
    int sy = relativeToWidget.y()/m_scaleFactor - m_my;
    QPoint scaled(sx,sy);
    QPainter p(m_agMask);
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(m_requestedPixmapColor, Qt::SolidPattern));
    if (!m_brushAddMode)
        p.setCompositionMode(QPainter::CompositionMode_Clear);
    int pixSize = m_requestedPixmapSize/(2*m_scaleFactor);
    p.drawEllipse(scaled, pixSize, pixSize);
    update();
    mPixmap->setPixmap(QPixmap::fromImage(*m_agMask));
}

void AntiGhostingWidget::drawPath()
{
    QPainter painter(m_agMask);
    painter.setPen(QPen(m_requestedLassoColor, 0, Qt::SolidLine,
                     Qt::FlatCap, Qt::MiterJoin));
    painter.setBrush(QBrush());

    if (m_drawingPathEnded) {
        painter.setCompositionMode(QPainter::CompositionMode_Clear); // Nasty hack, QPen does not draw semi transparent
        painter.drawPath(m_path); 
        if (m_brushAddMode)
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.fillPath(m_path, m_requestedPixmapColor);
        if (m_brushAddMode) { // redraw the path
            painter.setPen(QPen(m_requestedPixmapColor, 0, Qt::SolidLine,
                             Qt::FlatCap, Qt::MiterJoin));
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawPath(m_path);
        }
    }
    else {
        m_path.lineTo(m_lastPoint);
        m_lastPoint = m_currentPoint;
        painter.drawPath(m_path);
    }
    update();
    mPixmap->setPixmap(QPixmap::fromImage(*m_agMask));
}

void AntiGhostingWidget::setDrawWithBrush()
{
    m_drawingMode = BRUSH;
    //unsetCursor();
    fillAntiGhostingCursorPixmap();
}

void AntiGhostingWidget::setDrawPath()
{
    m_drawingMode = PATH;
    //unsetCursor();
    //setCursor(Qt::CrossCursor);
}

void AntiGhostingWidget::setBrushSize (const int newsize) {
    m_requestedPixmapSize = newsize;
}

void AntiGhostingWidget::setBrushMode(bool removemode) {
    m_requestedPixmapStrength *= -1;
    m_brushAddMode = !removemode;
}

void AntiGhostingWidget::setBrushStrength (const int newstrength) {
    m_requestedPixmapStrength = newstrength;
    m_requestedPixmapColor.setAlpha(qMax(60,m_requestedPixmapStrength));
    m_requestedPixmapStrength *= (!m_brushAddMode) ? -1 : 1;
}

void AntiGhostingWidget::setBrushColor (const QColor newcolor) {
    m_requestedPixmapColor = newcolor;
    update();
}

void AntiGhostingWidget::setLassoColor (const QColor newcolor) {
    m_requestedLassoColor = newcolor;
    update();
}


void AntiGhostingWidget::enterEvent(QEvent *) {
    if (m_drawingMode == BRUSH) {
        fillAntiGhostingCursorPixmap();
        //this->unsetCursor();
        //this->setCursor(*m_agcursorPixmap);
    }
}

void AntiGhostingWidget::switchAntighostingMode(bool ag) {
    if (ag) {
        if (m_drawingMode == BRUSH)
            ;//this->setCursor(*m_agcursorPixmap);
        else
            ;//this->setCursor(Qt::CrossCursor);
    } else {
        //this->unsetCursor();
    }
}

void AntiGhostingWidget::fillAntiGhostingCursorPixmap() {
    if (m_agcursorPixmap)
        delete m_agcursorPixmap;
    m_previousPixmapSize = m_requestedPixmapSize;
    m_previousPixmapStrength = m_requestedPixmapStrength;
    m_previousPixmapColor = m_requestedPixmapColor;
    m_agcursorPixmap = new QPixmap(m_requestedPixmapSize,m_requestedPixmapSize);
    m_agcursorPixmap->fill(Qt::transparent);
    QPainter painter(m_agcursorPixmap);
    painter.setPen(Qt::DashLine);
    painter.setBrush(QBrush(m_requestedPixmapColor,Qt::SolidPattern));
    painter.drawEllipse(0,0,m_requestedPixmapSize,m_requestedPixmapSize);
}

void AntiGhostingWidget::updateVertShift(int v) {
    m_my = v;
}

void AntiGhostingWidget::updateHorizShift(int h) {
    m_mx = h;
}

void AntiGhostingWidget::saveAgMask()
{
    if (m_savedMask)
        delete m_savedMask;
    m_savedMask = new QImage(*m_agMask);
}

QImage * AntiGhostingWidget::getSavedAgMask()
{
    return m_savedMask;
}
