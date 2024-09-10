#pragma once

#include <QVector2D>
#include <vector>

#include <QLineEdit>
#include <QString>
#include <QWidget>

#include "src/editorcommands/editorcommand.h"
#include "src/editorutils/editorentityreference.h"
#include <nlohmann/json.hpp>
#include <raccoon-ecs/delegates.h>

#include "EngineData/Geometry/Vector2D.h"

#include "GameData/Spatial/CellPos.h"
#include "GameData/Spatial/SpatialEntityManager.h"

class MainWindow;
class World;
class QPoint;

namespace ads
{
	class CDockManager;
}

class TransformEditorWidget final : public QWidget
{
public:
	explicit TransformEditorWidget(MainWindow* mainWindow);

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

	void onClick(const QPoint& pos);
	SpatialEntityManager::RecordsVector getCellsOnScreen();
	EditorEntityReference getEntityUnderPoint(const QPoint& pos);

	void onCoordinateXChanged(const QString& newValueStr);
	void onCoordinateYChanged(const QString& newValueStr);

	void addEntitiesInRectToSelection(const Vector2D& start, const Vector2D& end);
	void setGroupCenter(Vector2D newCenter);
	void clearGroupCenter();
	void updateSelectedEntitiesPosition();

	[[nodiscard]]
	QVector2D projectAbsolute(const Vector2D& worldPos) const;
	[[nodiscard]]
	Vector2D deprojectAbsolute(const QVector2D& screenPos) const;
	[[nodiscard]]
	QVector2D project(const Vector2D& pos) const;
	[[nodiscard]]
	std::pair<CellPos, Vector2D> deproject(const QVector2D& screenPos) const;

	World* mWorld = nullptr;
	MainWindow* mMainWindow;

	QLineEdit* mCoordinateXEdit = nullptr;
	QLineEdit* mCoordinateYEdit = nullptr;

	QVector2D mLastMousePos = QVector2D(0.0f, 0.0f);
	QVector2D mPressMousePos = QVector2D(0.0f, 0.0f);

	QVector2D mPosShift = QVector2D(0.0f, 0.0f);
	Vector2D mMoveShift = Vector2D(0.0f, 0.0f);
	float mScale = 1.0f;
	Vector2D mCursorObjectOffset = ZERO_VECTOR;

	RaccoonEcs::SinglecastDelegate<const std::vector<EditorEntityReference>&, const Vector2D&> OnEntitiesMoved;

	std::vector<EditorEntityReference> mSelectedEntities;
	Vector2D mSelectedGroupCenter = ZERO_VECTOR;

	bool mFreeMove = true;
	bool mIsMoved = false;
	bool mIsRectangleSelection = false;
	bool mHasCaughtSelectedEntity = false;
};

class TransformEditorToolbox final : public QWidget
{
public:
	TransformEditorToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager);
	~TransformEditorToolbox() override;
	void show();
	[[nodiscard]]
	bool isShown() const;

	[[nodiscard]]
	std::pair<CellPos, Vector2D> getWidgetCenterWorldPosition() const;

	static const QString WidgetName;
	static const QString ToolboxName;

private:
	void updateWorld();
	void updateEntitiesFromChangeEntityGroupLocationCommand(const class ChangeEntityGroupLocationCommand& command);
	void onCommandExecuted(EditorCommand::EffectBitset effects, bool originalCall);
	void onEntitySelected(const std::optional<EditorEntityReference>& entityRef);
	void onEntitiesMoved(const std::vector<EditorEntityReference>& entities, const Vector2D& shift);
	void onFreeMoveChanged(int newValue);
	void onScaleChanged(int newValue);
	void showContextMenu(const QPoint& pos);
	void onCopyCommand();
	void onPasteCommand();
	void onDeleteCommand();
	void unselectNonExistingEntities();
	[[nodiscard]]
	QVector2D getWidgetCenter() const;

private:
	MainWindow* mMainWindow;
	ads::CDockManager* mDockManager;
	TransformEditorWidget* mContent = nullptr;

	std::vector<nlohmann::json> mCopiedObjects;
	Vector2D mCopiedGroupCenter = ZERO_VECTOR;

	RaccoonEcs::Delegates::Handle mOnWorldChangedHandle;
	RaccoonEcs::Delegates::Handle mOnSelectedEntityChangedHandle;
	RaccoonEcs::Delegates::Handle mOnCommandEffectHandle;
};
