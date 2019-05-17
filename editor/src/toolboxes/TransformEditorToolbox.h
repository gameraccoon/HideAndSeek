#pragma once

#include <QString>
#include <QWidget>
#include <QVector2D>

#include "Core/Vector2D.h"
#include "Core/Entity.h"
#include "Core/Delegates.h"

class MainWindow;

namespace ads
{
	class CDockManager;
}

class TransformEditorWidget : public QWidget
{
public:
	TransformEditorWidget(MainWindow* mainWindow);

	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent* event) override;

	void onClick(const class QPoint& pos);
	NullableEntity getEntityUnderPoint(const QPoint& pos);

	QVector2D project(const Vector2D& worldPos);
	Vector2D deproject(const QVector2D& screenPos);

	class World* mWorld = nullptr;
	MainWindow* mMainWindow;

	QVector2D mLastMousePos = QVector2D(0.0f, 0.0f);
	QVector2D mPressMousePos = QVector2D(0.0f, 0.0f);

	QVector2D mPosShift = QVector2D(0.0f, 0.0f);
	float mScale = 1.0f;
	bool mIsMoved = false;
	bool mIsCatchedSelectedEntity = false;
	Vector2D mCursorObjectOffset;
	bool mFreeMove = true;

	NullableEntity mSelectedEntity;
};

class TransformEditorToolbox : public QWidget
{
public:
	TransformEditorToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager);
	~TransformEditorToolbox();
	void show();

	static const QString WidgetName;
	static const QString ToolboxName;
private:
	void updateWorld();
	void onEntitySelected(NullableEntity entity);
	void onFreeMoveChanged(int newValue);

private:
	MainWindow* mMainWindow;
	ads::CDockManager* mDockManager;
	TransformEditorWidget* mContent = nullptr;

	Delegates::Handle mOnWorldChangedHandle;
	Delegates::Handle mOnSelectedEntityChangedHandle;
};