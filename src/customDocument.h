#ifndef CUSTOMDOCUMENT_H
#define CUSTOMDOCUMENT_H

#include <QtGui>
#include <QStringList>
#include <QTextEdit>

class CustomDocument : public QTextEdit
{
    Q_OBJECT

public:
    explicit CustomDocument(QWidget *parent);
    void setDocumentPadding(int left, int top, int right, int bottom);
    bool eventFilter(QObject *obj, QEvent *event) override;
    bool openLinkAtCursorPosition();
    QString getMarkdownUrlAtPosition(const QString &text, int position);
    bool isValidUrl(const QString &urlString);
    void openUrl(const QString &urlString);
    QMap<QString, QString> parseMarkdownUrlsFromText(const QString &text);
    QUrl getUrlUnderMouse();
    void moveBlockUp();
    void moveBlockDown();
signals:
    void resized();
    void mouseMoved();

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    QStringList _ignoredClickUrlSchemata;
};

#endif // CUSTOMDOCUMENT_H
