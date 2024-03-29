/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the documentation of Qt. It was originally
** published as part of Qt Quarterly.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include <QtGui>
#include "documentwidget.h"

DocumentWidget::DocumentWidget(QWidget *parent)
    : QLabel(parent)
{
    currentPage = -1;
    doc = 0;
    scaleFactor = 1.0;
    setAlignment(Qt::AlignCenter);	
}

DocumentWidget::~DocumentWidget()
{
    delete doc;
}

Poppler::Document *DocumentWidget::document()
{
    return doc;
}

qreal DocumentWidget::scale() const
{
    return scaleFactor;
}

void DocumentWidget::showPage(int page)
{
    if (page != -1 && page != currentPage + 1) {
        currentPage = page - 1;
        emit pageChanged(page);
        emit pageChanged (QString::number(page)+"/"+QString::number(document()->numPages()));
    }

    QImage image = doc->page(currentPage)
                      ->renderToImage(scaleFactor * physicalDpiX(), scaleFactor * physicalDpiY());

    setPixmap(QPixmap::fromImage(image));
	resize(image.size());
	show();
}

bool DocumentWidget::setDocument(const QString &filePath)
{
    Poppler::Document *oldDocument = doc;

    doc = Poppler::Document::load(filePath);
    if (doc) {
        delete oldDocument;
        doc->setRenderHint(Poppler::Document::Antialiasing);
        doc->setRenderHint(Poppler::Document::TextAntialiasing);
        currentPage = -1;
        setPage(1);
    }
	
    return doc != 0;
}

void DocumentWidget::setPage(int page)
{

    printf ("Showing page: %d/%d\n", currentPage, page);
	fflush (stdout);

    if (page != currentPage + 1) {
        showPage(page);
    }
}

void DocumentWidget::nextPage () {

    if (currentPage+1 < document()->numPages()) {
        setPage (currentPage+2);
    }
}

void DocumentWidget::prevPage () {

    if (currentPage > 0) {
        setPage (currentPage);
    }

}


void DocumentWidget::setScale(qreal scale)
{
    if (scaleFactor != scale) {
        scaleFactor = scale;
        showPage();
    }
}

void DocumentWidget::setScale(QString scale)
{
    bool ok;

    QString scale_normalized = scale.remove ('%');

    printf ("Scale is: %s\n", qPrintable(scale_normalized));

    qreal scale_tmp = scale_normalized.toFloat (&ok);
    if (ok) {
        scale_tmp = scale_tmp / 100.0;
        setScale (scale_tmp);
    }

}

