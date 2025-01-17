#pragma once

#include <QString>
#include <QWidget>

#include <nlohmann/json.hpp>

#include "GameData/EcsDefinitions.h"

class MainWindow;

namespace ads
{
	class CDockManager;
}

class QListWidgetItem;
class QMenu;
class QPoint;

class PrefabListToolbox final : public QWidget
{
public:
	struct PrefabData
	{
		QString name;
		nlohmann::json data;

		friend void to_json(nlohmann::json& outJson, const PrefabData& data);
		friend void from_json(const nlohmann::json& json, PrefabData& outData);
	};

public:
	PrefabListToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager);
	~PrefabListToolbox() override;
	void show();

	void clear();
	void loadFromFile(const QString& filePath);
	void saveToFile(QString filePath = "");
	void createPrefabFromEntity(const QString& prefabName, Entity entity);

	static const QString WidgetName;
	static const QString ToolboxName;
	static const QString ContainerName;
	static const QString ContainerContentName;
	static const QString ListName;

private:
	void updateContent();
	void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void showContextMenu(const QPoint& pos);
	void removeSelectedPrefab();
	void createInstance();

private:
	std::vector<PrefabData> mPrefabs;
	MainWindow* mMainWindow;
	ads::CDockManager* mDockManager;
	QString mLastOpenFilePath;
};
