#include "QtPdfExporter.hpp"

#include <QFont>
#include <QFontMetrics>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QString>

namespace {

QString toQString(const std::string &value) {
    return QString::fromStdString(value);
}

struct VerticalCursor {
    int left{0};
    int top{0};
    int width{0};
};

void drawParagraph(QPainter &painter, VerticalCursor &cursor, const QString &text, int lineSpacing = 28) {
    const QRect rect(cursor.left, cursor.top, cursor.width, 1000);
    painter.drawText(rect, Qt::TextWordWrap, text);

    const QFontMetrics metrics(painter.font());
    const QRect usedRect = metrics.boundingRect(QRect(0, 0, cursor.width, 1000), Qt::TextWordWrap, text);

    cursor.top += usedRect.height() + lineSpacing;
}

void drawPlotPlaceholder(QPainter &painter, VerticalCursor &cursor, int height, const QString &title) {
    const QRect rect(cursor.left, cursor.top, cursor.width, height);

    painter.drawRect(rect);
    painter.drawText(rect, Qt::AlignCenter, title);

    cursor.top += height + 20;
}

} // namespace

namespace infrastructure {

void QtPdfExporter::exportDocument(const application::dto::PdfDocumentModel &document, const std::string &filePath) {
    QPdfWriter writer(toQString(filePath));
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(20.0, 20.0, 20.0, 20.0));

    QPainter painter(&writer);
    if (!painter.isActive()) {
        return;
    }

    // constexpr int left = 100;
    // constexpr int pageWidth = 2200;
    // int top = 120;

    VerticalCursor cursor{.left = 100, .top = 120, .width = 2200};

    // Title
    QFont titleFont("Times New Roman", 18, QFont::Bold);
    painter.setFont(titleFont);
    drawParagraph(painter, cursor, toQString(document.title), 40);

    // First stanza
    QFont textFont("Times New Roman", 12);
    painter.setFont(textFont);

    QString firstStanza;
    for (const auto &line : document.firstStanza) {
        firstStanza += toQString(line);
        firstStanza += "\n";
    }

    drawParagraph(painter, cursor, toQString(document.title), 40);

    // Plot 1 placeholder
    drawPlotPlaceholder(painter, cursor, 500, QStringLiteral("Graph 1 placeholder"));

    drawParagraph(painter, cursor, toQString(document.title), 40);

    // Plot 2 placeholder
    drawPlotPlaceholder(painter, cursor, 500, QStringLiteral("Graph 2 placeholder"));

    drawParagraph(painter, cursor, toQString(document.title), 40);

    // Second stanza
    QString secondStanza;
    for (const auto &line : document.secondStanza) {
        secondStanza += toQString(line);
        secondStanza += "\n";
    }

    drawParagraph(painter, cursor, toQString(document.title), 40);

    painter.end();
}

} // namespace infrastructure
