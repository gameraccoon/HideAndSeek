#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include <QLineEdit>
#include <QString>
#include <QVector2D>
#include <QWidget>

#include <raccoon-ecs/delegates.h>

#include "GameData/EcsDefinitions.h"
#include "GameData/Geometry/Vector2D.h"
#include "GameData/Spatial/SpatialEntity.h"
#include "GameData/Spatial/CellPos.h"

#include "src/editorcommands/editorcommand.h"
#include "src/editorutils/entityreference.h"

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
	SpatialEntityManager::RecordsVector getCellsOnScreen();
	SpatialEntity getEntityUnderPoint(const QPoint& pos);

	void onCoordinateXChanged(const QString& newValueStr);
	void onCoordinateYChanged(const QString& newValueStr);

	void addEntitiesInRectToSelection(const Vector2D& start, const Vector2D& end);
	void setGroupCenter(Vector2D newCenter);
	void clearGroupCenter();
	void updateSelectedEntitiesPosition();

	QVector2D projectAbsolute(const Vector2D& worldPos) const;
	Vector2D deprojectAbsolute(const QVector2D& screenPos) const;
	QVector2D project(const Vector2D& pos) const;
	std::pair<CellPos, Vector2D> deproject(const QVector2D& screenPos) const;

	class World* mWorld = nullptr;
	MainWindow* mMainWindow;

	QLineEdit* mCoordinateXEdit = nullptr;
	QLineEdit* mCoordinateYEdit = nullptr;

	QVector2D mLastMousePos = QVector2D(0.0f, 0.0f);
	QVector2D mPressMousePos = QVector2D(0.0f, 0.0f);

	QVector2D mPosShift = QVector2D(0.0f, 0.0f);
	Vector2D mMoveShift = Vector2D(0.0f, 0.0f);
	float mScale = 1.0f;
	Vector2D mCursorObjectOffset;

	RaccoonEcs::SinglecastDelegate<std::vector<SpatialEntity>, const Vector2D&> OnEntitiesMoved;

	std::vector<SpatialEntity> mSelectedEntities;
	Vector2D mSelectedGroupCenter;

	bool mFreeMove = true;
	bool mIsMoved = false;
	bool mIsRectangleSelection = false;
	bool mIsCatchedSelectedEntity = false;
};

class TransformEditorToolbox : public QWidget
{
public:
	TransformEditorToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager);
	~TransformEditorToolbox();
	void show();
	bool isShown() const;

	std::pair<CellPos, Vector2D> getWidgetCenterWorldPosition() const;

	static const QString WidgetName;
	static const QString ToolboxName;

private:
	void updateWorld();
	void updateEntitiesFromChangeEntityGroupLocationCommand(const class ChangeEntityGroupLocationCommand& command);
	void onCommandExecuted(EditorCommand::EffectBitset effects, bool originalCall);
	void onEntitySelected(const std::optional<EntityReference>& entityRef);
	void onEntitiesMoved(std::vector<SpatialEntity> entities, const Vector2D& shift);
	void onFreeMoveChanged(int newValue);
	void onScaleChanged(int newValue);
	void showContextMenu(const QPoint& pos);
	void onCopyCommand();
	void onPasteCommand();
	void onDeleteCommand();
	void unselectNonExistingEntities();
	QVector2D getWidgetCenter() const;

private:
	MainWindow* mMainWindow;
	ads::CDockManager* mDockManager;
	TransformEditorWidget* mContent = nullptr;

	std::vector<nlohmann::json> mCopiedObjects;
	Vector2D mCopiedGroupCenter;

	RaccoonEcs::Delegates::Handle mOnWorldChangedHandle;
	RaccoonEcs::Delegates::Handle mOnSelectedEntityChangedHandle;
	RaccoonEcs::Delegates::Handle mOnCommandEffectHandle;
};
