#include "TransformEditorToolbox.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QSlider>

#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidget.h"
#include "src/editorcommands/addentitygroupcommand.h"
#include "src/editorcommands/changeentitygrouplocationcommand.h"
#include "src/editorcommands/removeentitiescommand.h"
#include "src/editorutils/editoridutils.h"
#include "src/mainwindow.h"

#include "EngineCommon/Math/Float.h"

#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Serialization/Json/EntityManager.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"

const QString TransformEditorToolbox::WidgetName = "TransformEditor";
const QString TransformEditorToolbox::ToolboxName = WidgetName + "Toolbox";

static bool IsCtrlPressed()
{
	return (QApplication::keyboardModifiers() & Qt::ControlModifier) != 0;
}

static Vector2D GetEntityPosition(EditorEntityReference entityRef, World* world)
{
	if (world == nullptr)
	{
		return ZERO_VECTOR;
	}

	if (!entityRef.cellPos.has_value())
	{
		return ZERO_VECTOR;
	}

	WorldCell* cell = world->getSpatialData().getCell(*entityRef.cellPos);

	if (cell == nullptr)
	{
		return ZERO_VECTOR;
	}

	const OptionalEntity entity = Utils::GetEntityFromId(entityRef.editorUniqueId, cell->getEntityManager());

	if (!entity.isValid())
	{
		return ZERO_VECTOR;
	}

	if (auto [transform] = cell->getEntityManager().getEntityComponents<const TransformComponent>(entity.getEntity()); transform)
	{
		return transform->getLocation();
	}
	return ZERO_VECTOR;
}

static Vector2D GetEntityGroupPosition(const std::vector<EditorEntityReference>& entities, World* world)
{
	Vector2D result = ZERO_VECTOR;
	size_t entitiesProcessed = 0;
	for (const EditorEntityReference& entityRef : entities)
	{
		if (!entityRef.cellPos.has_value())
		{
			continue;
		}
		const OptionalEntity entity = Utils::GetEntityFromId(entityRef.editorUniqueId, world->getSpatialData().getOrCreateCell(*entityRef.cellPos).getEntityManager());
		if (!entity.isValid())
		{
			continue;
		}
		result = result * static_cast<float>(entitiesProcessed) / static_cast<float>(entitiesProcessed + 1) + GetEntityPosition(entityRef, world) / static_cast<float>(entitiesProcessed + 1);
		++entitiesProcessed;
	}
	return result;
}

TransformEditorToolbox::TransformEditorToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager)
	: mMainWindow(mainWindow)
	, mDockManager(dockManager)
{
	mOnWorldChangedHandle = mMainWindow->OnWorldChanged.bind([this] { updateWorld(); });
	mOnSelectedEntityChangedHandle = mMainWindow->OnSelectedEntityChanged.bind([this](const auto& entityRef) { onEntitySelected(entityRef); });
	mOnCommandEffectHandle = mMainWindow->OnCommandEffectApplied.bind([this](EditorCommand::EffectBitset effects, bool originalCall) { onCommandExecuted(effects, originalCall); });
}

TransformEditorToolbox::~TransformEditorToolbox()
{
	mMainWindow->OnWorldChanged.unbind(mOnWorldChangedHandle);
	mMainWindow->OnSelectedEntityChanged.unbind(mOnSelectedEntityChangedHandle);
	mMainWindow->OnCommandEffectApplied.unbind(mOnCommandEffectHandle);
}

void TransformEditorToolbox::show()
{
	if (ads::CDockWidget* dockWidget = mDockManager->findDockWidget(ToolboxName))
	{
		if (dockWidget->isVisible())
		{
			return;
		}
		mDockManager->layout()->removeWidget(dockWidget);
	}

	mContent = HS_NEW TransformEditorWidget(mMainWindow);
	ads::CDockWidget* dockWidget = HS_NEW ads::CDockWidget(QString("Transform Editor"));
	dockWidget->setObjectName(ToolboxName);
	dockWidget->setWidget(mContent);
	dockWidget->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
	dockWidget->setIcon(mMainWindow->style()->standardIcon(QStyle::SP_DialogOpenButton));
	dockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, true);
	mDockManager->addDockWidget(ads::RightDockWidgetArea, dockWidget);
	dockWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(dockWidget, &QListWidget::customContextMenuRequested, this, &TransformEditorToolbox::showContextMenu);

	QHBoxLayout* layout = HS_NEW QHBoxLayout();
	layout->addStretch();
	layout->setAlignment(Qt::AlignmentFlag::AlignBottom | Qt::AlignmentFlag::AlignRight);
	mContent->setLayout(layout);

	const auto dv = HS_NEW QDoubleValidator(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(), 10);
	dv->setNotation(QDoubleValidator::StandardNotation);

	mContent->mCoordinateXEdit = HS_NEW QLineEdit();
	mContent->mCoordinateXEdit->setValidator(dv);
	layout->addWidget(mContent->mCoordinateXEdit);

	mContent->mCoordinateYEdit = HS_NEW QLineEdit();
	mContent->mCoordinateYEdit->setValidator(dv);
	layout->addWidget(mContent->mCoordinateYEdit);

	QCheckBox* freeMoveCheckbox = HS_NEW QCheckBox();
	freeMoveCheckbox->setText("Free Move");
	freeMoveCheckbox->setChecked(mContent->mFreeMove);
	QObject::connect(freeMoveCheckbox, &QCheckBox::stateChanged, this, &TransformEditorToolbox::onFreeMoveChanged);
	layout->addWidget(freeMoveCheckbox);

	QSlider* scaleSlider = HS_NEW QSlider();
	scaleSlider->setRange(10, 1000);
	scaleSlider->setValue(100);
	scaleSlider->setOrientation(Qt::Orientation::Horizontal);
	QObject::connect(scaleSlider, &QSlider::valueChanged, this, &TransformEditorToolbox::onScaleChanged);
	layout->addWidget(scaleSlider);

	mContent->OnEntitiesMoved.assign([this](const std::vector<EditorEntityReference>& entities, const Vector2D& shift) { onEntitiesMoved(entities, shift); });
}

bool TransformEditorToolbox::isShown() const
{
	if (const ads::CDockWidget* dockWidget = mDockManager->findDockWidget(ToolboxName))
	{
		return dockWidget->isVisible();
	}
	return false;
}

void TransformEditorToolbox::updateWorld()
{
	if (mContent == nullptr)
	{
		return;
	}

	mContent->mWorld = mMainWindow->getCurrentWorld();
	mContent->repaint();
}

void TransformEditorToolbox::updateEntitiesFromChangeEntityGroupLocationCommand(const ChangeEntityGroupLocationCommand& command)
{
	const bool isLastUndo = mMainWindow->getCommandStack().isLastExecutedUndo();

	const std::vector<size_t>& entities = command.getEditorEntityReferences();
	const std::vector<CellPos>& newCells = isLastUndo ? command.getOriginalCellsOfEntities() : command.getModifiedCellsOfEntities();

	std::unordered_map<size_t, size_t> transformMap;

	for (size_t i = 0; i < entities.size(); ++i)
	{
		transformMap[entities[i]] = i;
	}

	for (EditorEntityReference& entityReerence : mContent->mSelectedEntities)
	{
		if (const auto it = transformMap.find(entityReerence.editorUniqueId); it != transformMap.end())
		{
			entityReerence.cellPos = newCells[it->second];
		}
	}
}

void TransformEditorToolbox::onCommandExecuted(EditorCommand::EffectBitset effects, bool /*originalCall*/)
{
	// if we have any selected entities
	if (!mContent->mSelectedEntities.empty())
	{
		// check that they didn't change their cells
		if (effects.hasAnyOf(EditorCommand::EffectType::ComponentAttributes, EditorCommand::EffectType::Entities))
		{
			const std::weak_ptr<const EditorCommand> lastCommand = mMainWindow->getCommandStack().getLastExecutedCommand();
			if (const std::shared_ptr<const EditorCommand> command = lastCommand.lock())
			{
				if (const auto changeLocationCommand = dynamic_cast<const ChangeEntityGroupLocationCommand*>(command.get()))
				{
					updateEntitiesFromChangeEntityGroupLocationCommand(*changeLocationCommand);
				}
			}
			mContent->updateSelectedEntitiesPosition();
		}
	}

	unselectNonExistingEntities();

	mContent->repaint();
}

void TransformEditorToolbox::onEntitySelected(const std::optional<EditorEntityReference>& entityRef)
{
	if (mContent == nullptr)
	{
		return;
	}

	World* world = mMainWindow->getCurrentWorld();
	if (world == nullptr)
	{
		return;
	}

	mContent->mSelectedEntities.clear();
	if (entityRef.has_value() && entityRef->cellPos.has_value())
	{
		WorldCell* cell = world->getSpatialData().getCell(*entityRef->cellPos);
		if (cell == nullptr)
		{
			return;
		}

		const OptionalEntity entity = Utils::GetEntityFromId(entityRef->editorUniqueId, cell->getEntityManager());
		if (!entity.isValid())
		{
			return;
		}

		EntityManager& cellEntityManager = cell->getEntityManager();
		if (cellEntityManager.doesEntityHaveComponent<TransformComponent>(entity.getEntity()))
		{
			mContent->mSelectedEntities.emplace_back(entityRef->editorUniqueId, *entityRef->cellPos);
		}
	}
	mContent->updateSelectedEntitiesPosition();
	mContent->repaint();
}

void TransformEditorToolbox::onEntitiesMoved(const std::vector<EditorEntityReference>& entities, const Vector2D& shift)
{
	const CommandExecutionContext context = mMainWindow->getCommandExecutionContext();
	if (context.world == nullptr)
	{
		return;
	}

	mMainWindow->getCommandStack().executeNewCommand<ChangeEntityGroupLocationCommand>(context, entities, shift);
}

void TransformEditorToolbox::onFreeMoveChanged(int newValue)
{
	if (mContent)
	{
		mContent->mFreeMove = (newValue != 0);
	}
}

void TransformEditorToolbox::onScaleChanged(int newValue)
{
	if (mContent)
	{
		const float newScale = static_cast<float>(newValue) * 0.01f;
		const Vector2D halfScreenSizeInWorld = 0.5f / mContent->mScale * Vector2D(mContent->size().width(), mContent->size().height());
		const Vector2D shiftChange = halfScreenSizeInWorld * (mContent->mScale - newScale) / newScale;
		mContent->mPosShift += QVector2D(static_cast<int>(shiftChange.x), static_cast<int>(shiftChange.y));
		mContent->mScale = newScale;
		mContent->repaint();
	}
}

void TransformEditorToolbox::showContextMenu(const QPoint& pos)
{
	QMenu contextMenu(tr("Context menu"), this);

	QAction actionCopyComponent("Copy", this);
	connect(&actionCopyComponent, &QAction::triggered, this, &TransformEditorToolbox::onCopyCommand);
	contextMenu.addAction(&actionCopyComponent);

	QAction actionPasteComponent("Paste", this);
	connect(&actionPasteComponent, &QAction::triggered, this, &TransformEditorToolbox::onPasteCommand);
	contextMenu.addAction(&actionPasteComponent);

	QAction actionDeleteComponent("Delete", this);
	connect(&actionDeleteComponent, &QAction::triggered, this, &TransformEditorToolbox::onDeleteCommand);
	contextMenu.addAction(&actionDeleteComponent);

	contextMenu.exec(mContent->mapToGlobal(pos));
}

void TransformEditorToolbox::onCopyCommand()
{
	World* world = mMainWindow->getCurrentWorld();
	if (world == nullptr)
	{
		return;
	}

	if (mContent == nullptr)
	{
		return;
	}

	const auto& jsonSerializationHolder = mMainWindow->getComponentSerializationHolder();

	Vector2D center{ ZERO_VECTOR };
	mCopiedObjects.clear();
	for (EditorEntityReference entityRef : mContent->mSelectedEntities)
	{
		if (!entityRef.cellPos.has_value())
		{
			continue;
		}

		WorldCell* cell = world->getSpatialData().getCell(*entityRef.cellPos);
		if (cell == nullptr)
		{
			continue;
		}

		nlohmann::json serializedEntity;
		EntityManager& cellEntityManager = cell->getEntityManager();
		const OptionalEntity entity = Utils::GetEntityFromId(entityRef.editorUniqueId, cellEntityManager);
		if (!entity.isValid())
		{
			continue;
		}
		Json::GetPrefabFromEntity(cellEntityManager, serializedEntity, entity.getEntity(), jsonSerializationHolder);
		mCopiedObjects.push_back(serializedEntity);
		auto [transform] = cellEntityManager.getEntityComponents<TransformComponent>(entity.getEntity());
		if (transform)
		{
			center += transform->getLocation();
		}
	}

	if (mCopiedObjects.size() > 0)
	{
		mCopiedGroupCenter = center / mCopiedObjects.size();
	}
}

void TransformEditorToolbox::onPasteCommand()
{
	const CommandExecutionContext context = mMainWindow->getCommandExecutionContext();
	if (context.world == nullptr)
	{
		return;
	}

	if (mContent == nullptr)
	{
		return;
	}

	if (mCopiedObjects.empty())
	{
		return;
	}

	const Json::ComponentSerializationHolder& serializationHolder = mMainWindow->getComponentSerializationHolder();

	mMainWindow->getCommandStack().executeNewCommand<AddEntityGroupCommand>(context, mCopiedObjects, serializationHolder, mContent->deprojectAbsolute(getWidgetCenter()) - mCopiedGroupCenter);

	mContent->repaint();
}

void TransformEditorToolbox::onDeleteCommand()
{
	const CommandExecutionContext commandExecutionContext = mMainWindow->getCommandExecutionContext();
	if (commandExecutionContext.world == nullptr)
	{
		return;
	}

	if (mContent == nullptr)
	{
		return;
	}

	// clear copied objects because the deleted objects could be part of them
	mCopiedObjects.clear();

	mMainWindow->getCommandStack().executeNewCommand<RemoveEntitiesCommand>(
		commandExecutionContext,
		mContent->mSelectedEntities,
		mMainWindow->getComponentSerializationHolder()
	);

	mContent->repaint();
}

void TransformEditorToolbox::unselectNonExistingEntities()
{
	World* world = mContent->mWorld;

	if (world == nullptr)
	{
		return;
	}

	auto [begin, end] = std::ranges::remove_if(mContent->mSelectedEntities, [world](const EditorEntityReference& entityRef) {
		if (!entityRef.cellPos.has_value())
		{
			return true;
		}

		WorldCell* cell = world->getSpatialData().getCell(*entityRef.cellPos);

		if (cell == nullptr)
		{
			return true;
		}

		const OptionalEntity entity = Utils::GetEntityFromId(entityRef.editorUniqueId, cell->getEntityManager());
		if (!entity.isValid())
		{
			return true;
		}

		return !cell->getEntityManager().hasEntity(entity.getEntity());
	});
	mContent->mSelectedEntities.erase(begin, end);

	mContent->setGroupCenter(GetEntityGroupPosition(mContent->mSelectedEntities, mContent->mWorld));
}

QVector2D TransformEditorToolbox::getWidgetCenter() const
{
	const QSize size = mContent->size() / 2;
	return QVector2D(size.width(), size.height());
}

std::pair<CellPos, Vector2D> TransformEditorToolbox::getWidgetCenterWorldPosition() const
{
	return mContent->deproject(getWidgetCenter());
}

TransformEditorWidget::TransformEditorWidget(MainWindow* mainWindow)
	: mMainWindow(mainWindow)
{
	mWorld = mainWindow->getCurrentWorld();
}

void TransformEditorWidget::mousePressEvent(QMouseEvent* event)
{
	mLastMousePos = QVector2D(event->pos());
	mMoveShift = ZERO_VECTOR;
	mPressMousePos = mLastMousePos;
	mIsMoved = false;
	mHasCaughtSelectedEntity = false;
	mIsRectangleSelection = false;

	if (EditorEntityReference entityUnderCursor = getEntityUnderPoint(event->pos()); entityUnderCursor.cellPos.has_value())
	{
		if (std::find(mSelectedEntities.begin(), mSelectedEntities.end(), entityUnderCursor) != mSelectedEntities.end())
		{
			mHasCaughtSelectedEntity = true;
		}
		else if (mFreeMove && !IsCtrlPressed())
		{
			mSelectedEntities.clear();
			mSelectedEntities.emplace_back(entityUnderCursor);
			mHasCaughtSelectedEntity = true;
			setGroupCenter(GetEntityPosition(entityUnderCursor, mWorld));
		}
	}
	else
	{
		if (IsCtrlPressed())
		{
			mIsRectangleSelection = true;
		}
	}
}

void TransformEditorWidget::mouseMoveEvent(QMouseEvent* event)
{
	const QVector2D pos = QVector2D(event->pos());
	if (!mIsMoved && (mPressMousePos - pos).lengthSquared() > 10.0f)
	{
		mIsMoved = true;
	}

	if (mIsMoved && mHasCaughtSelectedEntity)
	{
		mMoveShift = deprojectAbsolute(pos) - deprojectAbsolute(mPressMousePos);
	}

	if (!mHasCaughtSelectedEntity && !mIsRectangleSelection)
	{
		mPosShift -= QVector2D(mLastMousePos - pos) / mScale;
	}
	mLastMousePos = pos;
	repaint();
}

void TransformEditorWidget::mouseReleaseEvent(QMouseEvent* event)
{
	bool needRepaint = true;
	if (mIsMoved)
	{
		if (mHasCaughtSelectedEntity && !mSelectedEntities.empty())
		{
			Vector2D cachedMoveShift = mMoveShift;
			// need to reset the value before calling OnEntitiesMoved to correctly update visuals
			mMoveShift = ZERO_VECTOR;
			OnEntitiesMoved.callSafe(mSelectedEntities, cachedMoveShift);
			setGroupCenter(mSelectedGroupCenter + cachedMoveShift);
			// we've just triggered a repaint, no need to do it again
			needRepaint = false;
		}
		else if (mIsRectangleSelection)
		{
			addEntitiesInRectToSelection(deprojectAbsolute(mPressMousePos), deprojectAbsolute(QVector2D(event->pos())));
			setGroupCenter(GetEntityGroupPosition(mSelectedEntities, mWorld));
		}
	}
	else
	{
		onClick(event->pos());
	}

	mIsRectangleSelection = false;
	if (needRepaint)
	{
		repaint();
	}
}

void TransformEditorWidget::paintEvent(QPaintEvent*)
{
	if (mWorld == nullptr)
	{
		return;
	}

	QPainter painter(this);

	int crossSize = static_cast<int>(5.0f * mScale);

	SpatialEntityManager::RecordsVector cells = getCellsOnScreen();
	for (SpatialEntityManager::Record& record : cells)
	{
		WorldCell& cell = record.extraData;
		cell.getEntityManager().forEachComponentSetWithEntity<const TransformComponent>(
			[&painter, &cell, crossSize, this](Entity entity, const TransformComponent* transform) {
				const CellPos cellPos = cell.getPos();
				Vector2D location = transform->getLocation();

				auto [collision] = cell.getEntityManager().getEntityComponents<const CollisionComponent>(entity);

				const size_t editorId = Utils::GetOrCreateEditorIdFromEntity(entity, cell.getEntityManager(), mMainWindow->getEditorIdGenerator());

				if (std::find(mSelectedEntities.begin(), mSelectedEntities.end(), EditorEntityReference(editorId, cellPos)) != mSelectedEntities.end())
				{
					// preview the movement
					location += mMoveShift;

					// calc selected entity border
					QVector2D selectionLtShift;
					QVector2D selectionSize;
					if (collision)
					{
						Hull geometry = collision->getGeometry();
						if (geometry.type == HullType::Angular)
						{
							for (const Vector2D& point : geometry.points)
							{
								if (point.x < selectionLtShift.x())
								{
									selectionLtShift.setX(point.x);
								}
								if (point.y < selectionLtShift.y())
								{
									selectionLtShift.setY(point.y);
								}
								if (point.x > selectionSize.x())
								{
									selectionSize.setX(point.x);
								}
								if (point.y > selectionSize.y())
								{
									selectionSize.setY(point.y);
								}
							}

							selectionSize -= selectionLtShift;
						}
						else
						{
							const float radius = geometry.getRadius();
							selectionLtShift = QVector2D(-radius, -radius);
							selectionSize = QVector2D(radius * 2.0f, radius * 2.0f);
						}
					}

					// draw selected entity border
					selectionLtShift *= mScale;
					selectionLtShift -= QVector2D(5.0f, 5.0f);
					selectionSize *= mScale;
					selectionSize += QVector2D(10.0f, 10.0f);
					const QRectF rectangle((projectAbsolute(location) + selectionLtShift).toPoint(), QSize(static_cast<int>(selectionSize.x()), static_cast<int>(selectionSize.y())));
					QBrush brush = painter.brush();
					brush.setColor(Qt::GlobalColor::blue);
					painter.setBrush(brush);
					painter.drawRect(rectangle);
				}

				// draw collision
				if (collision)
				{
					Hull geometry = collision->getGeometry();
					if (geometry.type == HullType::Angular)
					{
						QPolygonF polygon;
						for (const Vector2D& point : geometry.points)
						{
							polygon.append(projectAbsolute(location + point).toPointF());
						}
						painter.drawPolygon(polygon);
					}
					else
					{
						const float radius = geometry.getRadius();
						const float halfWorldSize = radius * mScale;
						const int worldSizeInt = static_cast<int>(halfWorldSize * 2.0f);
						const QRectF rectangle((projectAbsolute(location) - QVector2D(halfWorldSize, halfWorldSize)).toPoint(), QSize(worldSizeInt, worldSizeInt));
						painter.drawEllipse(rectangle);
					}
				}

				// draw entity location cross
				const QVector2D screenLocation = projectAbsolute(location);
				const QPoint screenPoint(static_cast<int>(screenLocation.x()), static_cast<int>(screenLocation.y()));
				painter.drawLine(QPoint(screenPoint.x() - crossSize, screenPoint.y()), QPoint(screenPoint.x() + crossSize, screenPoint.y()));
				painter.drawLine(QPoint(screenPoint.x(), screenPoint.y() - crossSize), QPoint(screenPoint.x(), screenPoint.y() + crossSize));
			}
		);
	}

	if (mIsRectangleSelection)
	{
		const QVector2D size = QVector2D(mLastMousePos) - mPressMousePos;
		const QRect rect(mPressMousePos.toPoint(), QSize(static_cast<int>(size.x()), static_cast<int>(size.y())));
		painter.drawRect(rect);
	}
}

void TransformEditorWidget::onClick(const QPoint& pos)
{
	EditorEntityReference findResult = getEntityUnderPoint(pos);

	if (IsCtrlPressed())
	{
		if (findResult.cellPos.has_value())
		{
			const auto it = std::find(mSelectedEntities.begin(), mSelectedEntities.end(), findResult);
			if (it != mSelectedEntities.end())
			{
				mSelectedEntities.erase(it);
			}
			else
			{
				mSelectedEntities.push_back(findResult);
			}
		}
	}
	else
	{
		mSelectedEntities.clear();

		if (findResult.cellPos.has_value())
		{
			mSelectedEntities.push_back(findResult);
			mMainWindow->OnSelectedEntityChanged.broadcast(EditorEntityReference(findResult));
		}
	}
	updateSelectedEntitiesPosition();
}

SpatialEntityManager::RecordsVector TransformEditorWidget::getCellsOnScreen()
{
	const Vector2D screenSizeInWorld = Vector2D(size().width(), size().height()) / mScale;
	const Vector2D screenCenterPos = screenSizeInWorld * 0.5f - Vector2D(mPosShift.x(), mPosShift.y());
	return mWorld->getSpatialData().getCellsAround(screenCenterPos, screenSizeInWorld);
}

EditorEntityReference TransformEditorWidget::getEntityUnderPoint(const QPoint& pos)
{
	Vector2D worldPos = deprojectAbsolute(QVector2D(pos));

	SpatialEntity findResult;

	if (mWorld)
	{
		SpatialEntityManager::RecordsVector cells = getCellsOnScreen();
		for (SpatialEntityManager::Record& record : cells)
		{
			WorldCell& cell = record.extraData;
			cell.getEntityManager().forEachComponentSetWithEntity<const TransformComponent>(
				[worldPos, cellPos = cell.getPos(), &findResult](Entity entity, const TransformComponent* transform) {
					const Vector2D location = transform->getLocation();
					if (location.x - 10 < worldPos.x && location.x + 10 > worldPos.x
						&& location.y - 10 < worldPos.y && location.y + 10 > worldPos.y)
					{
						findResult.entity = entity;
						findResult.cell = cellPos;
					}
				}
			);
		}
	}

	if (!findResult.isValid())
	{
		return EditorEntityReference{ 0 };
	}

	const size_t editorId = Utils::GetOrCreateEditorIdFromEntity(findResult.entity.getEntity(), mWorld->getSpatialData().getOrCreateCell(findResult.cell).getEntityManager(), mMainWindow->getEditorIdGenerator());

	return EditorEntityReference{ editorId, findResult.cell };
}

void TransformEditorWidget::onCoordinateXChanged(const QString& newValueStr)
{
	const CommandExecutionContext context = mMainWindow->getCommandExecutionContext();
	if (context.world == nullptr)
	{
		return;
	}

	bool conversionSuccessful = false;
	const float newValue = newValueStr.toFloat(&conversionSuccessful);

	if (!conversionSuccessful)
	{
		return;
	}

	const Vector2D shift(newValue - mSelectedGroupCenter.x, 0.0f);

	mMainWindow->getCommandStack().executeNewCommand<ChangeEntityGroupLocationCommand>(context, mSelectedEntities, shift);
	mSelectedGroupCenter.x = newValue;
}

void TransformEditorWidget::onCoordinateYChanged(const QString& newValueStr)
{
	const CommandExecutionContext context = mMainWindow->getCommandExecutionContext();
	if (context.world == nullptr)
	{
		return;
	}

	bool conversionSuccessful = false;
	const float newValue = newValueStr.toFloat(&conversionSuccessful);

	if (!conversionSuccessful)
	{
		return;
	}

	const Vector2D shift(0.0f, newValue - mSelectedGroupCenter.y);

	mMainWindow->getCommandStack().executeNewCommand<ChangeEntityGroupLocationCommand>(context, mSelectedEntities, shift);
	mSelectedGroupCenter.y = newValue;
}

void TransformEditorWidget::addEntitiesInRectToSelection(const Vector2D& start, const Vector2D& end)
{
	Vector2D lt;
	Vector2D rd;

	if (start.x > end.x)
	{
		lt.x = end.x;
		rd.x = start.x;
	}
	else
	{
		lt.x = start.x;
		rd.x = end.x;
	}

	if (start.y > end.y)
	{
		lt.y = end.y;
		rd.y = start.y;
	}
	else
	{
		lt.y = start.y;
		rd.y = end.y;
	}

	if (mWorld)
	{
		SpatialEntityManager::RecordsVector cells = getCellsOnScreen();
		for (SpatialEntityManager::Record& record : cells)
		{
			WorldCell& cell = record.extraData;
			cell.getEntityManager().forEachComponentSetWithEntity<const TransformComponent>(
				[this, lt, rd, &cell](Entity entity, const TransformComponent* transform) {
					const Vector2D location = transform->getLocation();
					if (lt.x < location.x && location.x < rd.x && lt.y < location.y && location.y < rd.y)
					{
						const size_t editorId = Utils::GetOrCreateEditorIdFromEntity(entity, cell.getEntityManager(), mMainWindow->getEditorIdGenerator());
						const auto it = std::find(mSelectedEntities.begin(), mSelectedEntities.end(), EditorEntityReference(editorId, cell.getPos()));
						if (it == mSelectedEntities.end())
						{
							mSelectedEntities.emplace_back(editorId, cell.getPos());
						}
					}
				}
			);
		}
	}
}

template<typename Func>
void UpdateFieldIfChanged(QLineEdit* field, float newValue, TransformEditorWidget* receiver, Func method)
{
	bool conversionSuccessful = false;
	const float oldValue = field->text().toFloat(&conversionSuccessful);
	if (!conversionSuccessful || !Math::AreEqualWithEpsilon(oldValue, newValue, 0.0101f))
	{
		QObject::disconnect(field, &QLineEdit::textChanged, receiver, method);
		field->setText(QString::asprintf("%.2f", newValue));
		QObject::connect(field, &QLineEdit::textChanged, receiver, method);
	}
}

void TransformEditorWidget::setGroupCenter(Vector2D newCenter)
{
	mSelectedGroupCenter = newCenter;

	UpdateFieldIfChanged(mCoordinateXEdit, newCenter.x, this, &TransformEditorWidget::onCoordinateXChanged);
	UpdateFieldIfChanged(mCoordinateYEdit, newCenter.y, this, &TransformEditorWidget::onCoordinateXChanged);
}

void TransformEditorWidget::clearGroupCenter()
{
	QObject::disconnect(mCoordinateXEdit, &QLineEdit::textChanged, this, &TransformEditorWidget::onCoordinateXChanged);
	QObject::disconnect(mCoordinateYEdit, &QLineEdit::textChanged, this, &TransformEditorWidget::onCoordinateYChanged);

	mCoordinateXEdit->setText("");
	mCoordinateYEdit->setText("");
}

void TransformEditorWidget::updateSelectedEntitiesPosition()
{
	if (!mSelectedEntities.empty())
	{
		setGroupCenter(GetEntityGroupPosition(mSelectedEntities, mWorld));
	}
	else
	{
		clearGroupCenter();
	}
}

QVector2D TransformEditorWidget::projectAbsolute(const Vector2D& worldPos) const
{
	return mScale * (QVector2D(worldPos.x, worldPos.y) + mPosShift);
}

Vector2D TransformEditorWidget::deprojectAbsolute(const QVector2D& screenPos) const
{
	const QVector2D worldPos = screenPos / mScale - mPosShift;
	return Vector2D(worldPos.x(), worldPos.y());
}

QVector2D TransformEditorWidget::project(const Vector2D& pos) const
{
	const Vector2D absoluteWorldPos = pos;
	return mScale * (QVector2D(absoluteWorldPos.x, absoluteWorldPos.y) + mPosShift);
}

std::pair<CellPos, Vector2D> TransformEditorWidget::deproject(const QVector2D& screenPos) const
{
	const QVector2D worldPosQt = screenPos / mScale - mPosShift;
	Vector2D worldPos(worldPosQt.x(), worldPosQt.y());
	return std::make_pair(SpatialWorldData::GetCellForPos(worldPos), worldPos);
}
