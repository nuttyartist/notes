#ifndef CUSTOMDOCUMENT_H
#define CUSTOMDOCUMENT_H

#include <QtGui>
#include <QStringList>
#include <QTextEdit>

class CustomDocument : public QTextEdit
{
    Q_OBJECT
public:
    CustomDocument(QWidget *parent = nullptr);
    void setDocumentPadding(int left, int top, int right, int bottom);
    bool eventFilter(QObject *obj, QEvent *event);
    bool openLinkAtCursorPosition();
    QString getMarkdownUrlAtPosition(const QString &text, int position);
    bool isValidUrl(const QString &urlString);
    void openUrl(const QString &urlString);
    QMap<QString, QString> parseMarkdownUrlsFromText(const QString &text);
signals:
    void resized();

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);

    QStringList _ignoredClickUrlSchemata;
};

#endif // CUSTOMDOCUMENT_H
