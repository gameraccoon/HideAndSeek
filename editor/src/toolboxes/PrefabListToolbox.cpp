#include "PrefabListToolbox.h"

#include <fstream>
#include <iomanip>

#include "src/mainwindow.h"

#include "DockManager.h"
#include "DockWidget.h"
#include "DockAreaWidget.h"

#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>

#include <raccoon-ecs/component_factory.h>

#include "GameData/Serialization/Json/EntityManager.h"
#include "GameData/World.h"

#include "src/editorcommands/addcomponentcommand.h"

const QString PrefabListToolbox::WidgetName = "Prefabs";
const QString PrefabListToolbox::ToolboxName = PrefabListToolbox::WidgetName + "Toolbox";
const QString PrefabListToolbox::ContainerName = PrefabListToolbox::WidgetName + "Container";
const QString PrefabListToolbox::ContainerContentName = PrefabListToolbox::ContainerName + "Content";
const QString PrefabListToolbox::ListName = "PrefabList";

PrefabListToolbox::PrefabListToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager)
	: mMainWindow(mainWindow)
	, mDockManager(dockManager)
{
}

PrefabListToolbox::~PrefabListToolbox()
{
}

void PrefabListToolbox::show()
{
	if (ads::CDockWidget* dockWidget = mDockManager->findDockWidget(ToolboxName))
	{
		if (dockWidget->isVisible())
		{
			return;
		}
		else
		{
			mDockManager->layout()->removeWidget(dockWidget);
		}
	}

	QWidget* containerWidget = HS_NEW QWidget();
	ads::CDockWidget* dockWidget = HS_NEW ads::CDockWidget(QString("Prefabs Library"));
	dockWidget->setObjectName(ToolboxName);
	dockWidget->setWidget(containerWidget);
	dockWidget->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
	dockWidget->setIcon(mMainWindow->style()->standardIcon(QStyle::SP_DialogOpenButton));
	mDockManager->addDockWidget(ads::RightDockWidgetArea, dockWidget);

	containerWidget->setObjectName(ContainerName);

	QVBoxLayout* layout = HS_NEW QVBoxLayout();
	containerWidget->setLayout(layout);
	QListWidget* entityList = HS_NEW QListWidget();
	layout->addWidget(entityList);
	entityList->setObjectName(ListName);
	entityList->setContextMenuPolicy(Qt::CustomContextMenu);

	QObject::connect(entityList, &QListWidget::currentItemChanged, this, &PrefabListToolbox::onCurrentItemChanged);
	QObject::connect(entityList, &QListWidget::customContextMenuRequested, this, &PrefabListToolbox::showContextMenu);

	updateContent();
}

void PrefabListToolbox::clear()
{
	mPrefabs.clear();
	updateContent();
}

void PrefabListToolbox::loadFromFile(const QString& filePath)
{
	try
	{
		std::ifstream mapFile(filePath.toStdString());
		nlohmann::json prefabJson;
		mapFile >> prefabJson;

		mPrefabs.clear();
		prefabJson.at("prefabs").get_to(mPrefabs);
	}
	catch(const nlohmann::detail::exception& e)
	{
		LogError("Can't parse '%s': %s", filePath.toStdString().c_str(), e.what());
	}
	catch(const std::exception& e)
	{
		LogError("Can't open '%s': %s", filePath.toStdString().c_str(), e.what());
	}

	mLastOpenFilePath = filePath;
	updateContent();
}

void PrefabListToolbox::saveToFile(QString filePath)
{
	if (filePath.isEmpty())
	{
		filePath = mLastOpenFilePath;
	}

	try
	{
		std::ofstream mapFile(filePath.toStdString());

		nlohmann::json content{
			{"prefabs", mPrefabs}
		};
		mapFile << std::setw(4) << content << std::endl;
	}
	catch (const std::exception& e)
	{
		LogError("Can't save level to file '%s': %s", filePath.toStdString().c_str(), e.what());
	}
}

void PrefabListToolbox::createPrefabFromEntity(const QString& prefabName, Entity entity)
{
	auto it = std::find_if(mPrefabs.begin(), mPrefabs.end(), [prefabName](const PrefabData& prefabData)
	{
		return prefabData.name == prefabName;
	});

	if (it != mPrefabs.end())
	{
		QMessageBox messageBox;
		messageBox.warning(this, "Prefab can't be created", "There are already a prefab with this name!");
		return;
	}

	World* currentWorld = mMainWindow->getCurrentWorld();
	if (currentWorld == nullptr)
	{
		return;
	}

	const auto& jsonSerializationHolder = mMainWindow->getComponentSerializationHolder();

	PrefabData prefabData;
	prefabData.name = prefabName;
	EntityManager& worldEntityManager = currentWorld->getEntityManager();
	Json::GetPrefabFromEntity(worldEntityManager, prefabData.data, entity, jsonSerializationHolder);
	mPrefabs.push_back(prefabData);
	updateContent();
}

void PrefabListToolbox::updateContent()
{
	World* currentWorld = mMainWindow->getCurrentWorld();
	if (currentWorld == nullptr)
	{
		return;
	}

	QStringList entitiesStringList;
	for (const auto& [componentTypeName, _] : mPrefabs)
	{
		entitiesStringList.append(componentTypeName);
	}

	if (QListWidget* entitiesList = mDockManager->findChild<QListWidget*>(ListName))
	{
		entitiesList->clear();
		entitiesList->addItems(entitiesStringList);
	}
}

void PrefabListToolbox::onCurrentItemChanged(QListWidgetItem* /*current*/, QListWidgetItem* /*previous*/)
{
}

void PrefabListToolbox::showContextMenu(const QPoint& pos)
{
	QListWidget* entitiesList = mDockManager->findChild<QListWidget*>(ListName);
	if (entitiesList == nullptr)
	{
		return;
	}

	if (!entitiesList->currentItem())
	{
		return;
	}

	QMenu contextMenu(tr("Context menu"), this);

	QAction actionCreateInstance("Create instance", this);
	connect(&actionCreateInstance, &QAction::triggered, this, &PrefabListToolbox::createInstance);
	contextMenu.addAction(&actionCreateInstance);

	QAction actionRemove("Remove Prefab", this);
	connect(&actionRemove, &QAction::triggered, this, &PrefabListToolbox::removeSelectedPreab);
	contextMenu.addAction(&actionRemove);

	contextMenu.exec(entitiesList->mapToGlobal(pos));
}

void PrefabListToolbox::removeSelectedPreab()
{
	QListWidget* prefabList = mDockManager->findChild<QListWidget*>(ListName);
	if (prefabList == nullptr)
	{
		return;
	}

	QListWidgetItem* currentItem = prefabList->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	World* currentWorld = mMainWindow->getCurrentWorld();
	if (currentWorld == nullptr)
	{
		return;
	}

	mPrefabs.erase(std::find_if(mPrefabs.begin(), mPrefabs.end(), [prefabName = currentItem->text()](const PrefabData& prefabData)
	{
		return prefabData.name == prefabName;
	}));

	updateContent();
}

void PrefabListToolbox::createInstance()
{
	QListWidget* prefabList = mDockManager->findChild<QListWidget*>(ListName);
	if (prefabList == nullptr)
	{
		return;
	}

	QListWidgetItem* currentItem = prefabList->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	World* currentWorld = mMainWindow->getCurrentWorld();
	if (currentWorld == nullptr)
	{
		return;
	}

	auto it = std::find_if(mPrefabs.begin(), mPrefabs.end(), [prefabName = currentItem->text()](const PrefabData& prefabData)
	{
		return prefabData.name == prefabName;
	});

	if (it == mPrefabs.end())
	{
		return;
	}

	EntityManager& worldEntityManager = currentWorld->getEntityManager();

	Json::CreatePrefabInstance(worldEntityManager, it->data, mMainWindow->getComponentSerializationHolder());
}

void to_json(nlohmann::json& outJson, const PrefabListToolbox::PrefabData& data)
{
	outJson["data"] = data.data;
	outJson["name"] = data.name.toStdString();
}

void from_json(const nlohmann::json& json, PrefabListToolbox::PrefabData& outData)
{
	outData.data = json.at("data");

	std::string nameStr;
	json.at("name").get_to(nameStr);
	outData.name = QString::fromStdString(nameStr);
}
