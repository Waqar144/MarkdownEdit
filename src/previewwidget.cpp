#include "previewwidget.h"
#include "common.h"

#include <QUrl>
#include <QDesktopServices>
#include <QFile>


PreviewWidget::PreviewWidget(QWidget *parent)
    : QTextBrowser(parent)
{
    QTextBrowser::setOpenLinks(false);
    QTextBrowser::setOpenExternalLinks(true);
    QTextBrowser::setReadOnly(true);
    QTextBrowser::setUndoRedoEnabled(false);

    connect(this, &QTextBrowser::anchorClicked,
            this, &PreviewWidget::openUrl);
}

void PreviewWidget::openUrl(const QUrl &url)
{
    const QString s = url.toString();
    const QString basePath = currDir();
    const QString filePath = basePath + QChar('/') + s;

    if (s.startsWith(QLatin1String("http")))
        QDesktopServices::openUrl(url);

    if (QFile::exists(filePath))
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));

    if (s.startsWith(QChar('#')))
        scrollToHeader(s);
}

void PreviewWidget::scrollToHeader(QString name)
{
    if (name.startsWith(QChar('#')))
        name.remove(QChar('#'));
}
