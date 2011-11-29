#ifndef V3D_PYTHON_CONSOLE_WINDOW_H
#define V3D_PYTHON_CONSOLE_WINDOW_H

#include <QMainWindow>
#include "c_array_struct.hpp"
#include "ui_main_python_console.h"
#include "PythonInterpreter.h"
#include "CommandRing.h"

class QWidget;

class PythonConsoleWindow : public QMainWindow, Ui::main_python_console
{
	Q_OBJECT

public:
	PythonConsoleWindow(QWidget *parent = NULL);
	void executeCommand(const QString& command);
    bool eventFilter(QObject *watched, QEvent *event);

signals:
	void returnPressed();
	void pasteAvailable(bool);
	void cutAvailable(bool);
	void commandIssued(QString);
	void pythonReadlineEntered(QString);

public slots:
    void runScript();

private slots:
	void onReturnPressed();
	void onCursorPositionChanged();
	void onClipboardDataChanged();
	void onSelectionChanged();
	void onCopyAvailable(bool);
	void onCommandComplete();
	void onIncompleteCommand(QString partialCmd);
	void onPythonReadline();
	void onOutput(QString msg);
	void about();
	void zoomIn();
	void zoomOut();
	void openRecentFile();
	
private:
	void setupMenus();
	QString getCurrentCommand();
	void placeNewPrompt(bool bMakeVisible=false);
	void setPrompt(const QString& newPrompt);
	bool cursorIsInEditingRegion(const QTextCursor& cursor);
	void showPreviousCommand();
	void showNextCommand();
    void replaceCurrentCommand(const QString& newCommand);
    void addRecent(const QString& fileName);
    void updateRecent();

    bool bPythonReadline;
	QString prompt;
	int promptLength;
	QTextCursor latestGoodCursorPosition;
	int currentCommandStartPosition;
	QString multilineCommand;
    CommandRing commandRing;

    // static const int maxRecentScripts = 10;
    // QAction* recentScripts[maxRecentScripts];
    c_array<QAction*, 10> recentScripts;

public:
    PythonInterpreter *pythonInterpreter;
};

#endif // V3D_PYTHON_CONSOLE_WINDOW_H
