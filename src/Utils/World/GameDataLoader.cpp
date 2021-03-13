#include "Base/precomp.h"

#include "Utils/World/GameDataLoader.h"

#include <fstream>
#include <map>
#include <iomanip>

#include <filesystem>
#include <nlohmann/json.hpp>

#include "GameData/World.h"
#include "GameData/GameData.h"
#include "GameData/Components/LightBlockingGeometryComponent.generated.h"
#include "GameData/Components/PathBlockingGeometryComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Spatial/SpatialWorldData.h"

#include "Utils/Geometry/LightBlockingGeometry.h"
#include "Utils/AI/PathBlockingGeometry.h"

namespace GameDataLoader
{
	static const std::filesystem::path MAPS_PATH = "./resources/maps";
	static const std::filesystem::path GAME_DATA_PATH = "./resources/game";

	static void SaveLightBlockingGeometry(const World& world, const std::filesystem::path& levelPath)
	{
		namespace fs = std::filesystem;

		fs::path geometryPath(levelPath);
		geometryPath.replace_extension(".lbg.json");

		std::ofstream geometryFile(geometryPath);
		nlohmann::json geometryJson;

		const std::unordered_map<CellPos, WorldCell>& cells = world.getSpatialData().getAllCells();
		for (auto& [cellPos, cell] : cells)
		{
			const auto [lightBlockingGeometry] = cell.getCellComponents().getComponents<LightBlockingGeometryComponent>();
			if (lightBlockingGeometry)
			{
				std::string key = std::to_string(cellPos.x) + "," + std::to_string(cellPos.y);
				geometryJson[key] = lightBlockingGeometry->getBorders();
			}
		}

		geometryFile << std::setw(4) << geometryJson << std::endl;
	}

	static void GenerateLightBlockingGeometry(World& world)
	{
		TupleVector<WorldCell*, CollisionComponent*, TransformComponent*> components;
		world.getSpatialData().getAllCellManagers().getSpatialComponents<CollisionComponent, TransformComponent>(components);
		std::unordered_map<CellPos, std::vector<SimpleBorder>> lightBlockingGeometryPieces;
		LightBlockingGeometry::CalculateLightGeometry(lightBlockingGeometryPieces, components);

		for (auto& [cellPos, borders] : lightBlockingGeometryPieces)
		{
			WorldCell& cell = world.getSpatialData().getOrCreateCell(cellPos);
			ComponentSetHolder& cellComponents = cell.getCellComponents();
			LightBlockingGeometryComponent* lightBlockingGeometry = cellComponents.getOrAddComponent<LightBlockingGeometryComponent>();
			lightBlockingGeometry->setBorders(std::move(borders));
		}
	}

	static void LoadLightBlockingGeometry(World& world, const std::filesystem::path& levelPath)
	{
		namespace fs = std::filesystem;

		fs::path geometryPath(levelPath);
		geometryPath.replace_extension(".lbg.json");

		if (!fs::exists(geometryPath))
		{
			GenerateLightBlockingGeometry(world);
			return;
		}

		std::ifstream geometryFile(geometryPath);
		nlohmann::json geometryJson;
		geometryFile >> geometryJson;

		SpatialWorldData& spatialData = world.getSpatialData();

		for (auto& [key, cellData] : geometryJson.items())
		{
			const int delimeterPos = key.find(',');
			const CellPos pos{std::atoi(key.substr(0, delimeterPos).c_str()), std::atoi(key.substr(delimeterPos + 1).c_str())};
			LightBlockingGeometryComponent* lightBlockingCompontnt = spatialData.getCell(pos)->getCellComponents().getOrAddComponent<LightBlockingGeometryComponent>();
			cellData.get_to(lightBlockingCompontnt->getBordersRef());
		}
	}

	static void SavePathBlockingGeometry(const World& world, const std::filesystem::path& levelPath)
	{
		namespace fs = std::filesystem;

		fs::path geometryPath(levelPath);
		geometryPath.replace_extension(".pbg.json");

		std::ofstream geometryFile(geometryPath);
		nlohmann::json geometryJson;

		auto [pathBlockingGeometry] = world.getWorldComponents().getComponents<PathBlockingGeometryComponent>();

		if (pathBlockingGeometry)
		{
			geometryJson = pathBlockingGeometry->getPolygons();
			geometryFile << std::setw(4) << geometryJson << std::endl;
		}
	}

	static void GeneratePathBlockingGeometry(World& world)
	{
		TupleVector<CollisionComponent*, TransformComponent*> components;
		world.getSpatialData().getAllCellManagers().getComponents<CollisionComponent, TransformComponent>(components);

		PathBlockingGeometryComponent* pathBlockingGeometry = world.getWorldComponents().getOrAddComponent<PathBlockingGeometryComponent>();

		PathBlockingGeometry::CalculatePathBlockingGeometry(pathBlockingGeometry->getPolygonsRef(), components);
	}

	static void LoadPathBlockingGeometry(World& world, const std::filesystem::path& levelPath)
	{
		namespace fs = std::filesystem;

		fs::path geometryPath(levelPath);
		geometryPath.replace_extension(".pbg.json");

		if (!fs::exists(geometryPath))
		{
			GeneratePathBlockingGeometry(world);
			return;
		}

		std::ifstream geometryFile(geometryPath);
		nlohmann::json geometryJson;
		geometryFile >> geometryJson;

		PathBlockingGeometryComponent* pathBlockingCompontnt = world.getWorldComponents().getOrAddComponent<PathBlockingGeometryComponent>();
		geometryJson.get_to(pathBlockingCompontnt->getPolygonsRef());
	}

	void SaveWorld(const World& world, const std::string& levelName, const ComponentSerializersHolder& componentSerializers)
	{
		namespace fs = std::filesystem;
		fs::path levelPath(levelName);

		// if it's name, we save to maps folder
		if (levelName.find_first_of("/\\.") == std::string::npos)
		{
			levelPath = MAPS_PATH / (levelName + ".json");
		}
		else
		{
			if (!levelPath.has_extension())
			{
				levelPath = levelPath.string() + ".json";
			}
		}

		try
		{
			std::ofstream mapFile(levelPath);
			nlohmann::json mapJson({{"world", world.toJson(componentSerializers)}});

			mapFile << std::setw(4) << mapJson << std::endl;

			SaveLightBlockingGeometry(world, levelPath);
			SavePathBlockingGeometry(world, levelPath);
		}
		catch (const std::exception& e)
		{
			LogError("Can't save world to file '%s': %s", levelPath.c_str(), e.what());
		}
	}

	void LoadWorld(World& world, const std::string& levelName, const ComponentSerializersHolder& componentSerializers)
	{
		namespace fs = std::filesystem;
		fs::path levelPath(levelName);

		// if it's name, we search the map in maps folder
		if (levelName.find_first_of("/\\.") == std::string::npos)
		{
			levelPath = MAPS_PATH / (levelName + ".json");
		}

		try
		{
			std::ifstream mapFile(levelPath);
			nlohmann::json mapJson;
			mapFile >> mapJson;

			if (const auto& worldObject = mapJson.at("world"); worldObject.is_object())
			{
				world.fromJson(worldObject, componentSerializers);
			}
			LoadLightBlockingGeometry(world, levelPath);
			LoadPathBlockingGeometry(world, levelPath);
		}
		catch(const nlohmann::detail::exception& e)
		{
			LogError("Can't parse world '%s': %s", levelPath.c_str(), e.what());
		}
		catch(const std::exception& e)
		{
			LogError("Can't open world '%s': %s", levelPath.c_str(), e.what());
		}
	}

	void SaveGameData(const GameData& gameData, const std::string& gameDataName, const ComponentSerializersHolder& componentSerializers)
	{
		namespace fs = std::filesystem;
		fs::path gameDataPath(gameDataName);

		// if it's name, we save to maps folder
		if (gameDataName.find_first_of("/\\.") == std::string::npos)
		{
			gameDataPath = GAME_DATA_PATH / (gameDataName + ".json");
		}
		else
		{
			if (!gameDataPath.has_extension())
			{
				gameDataPath = gameDataPath.string() + ".json";
			}
		}

		try
		{
			std::ofstream mapFile(gameDataPath);
			nlohmann::json mapJson({{"gameData", gameData.toJson(componentSerializers)}});

			mapFile << std::setw(4) << mapJson << std::endl;
		}
		catch (const std::exception& e)
		{
			LogError("Can't save gameData to file '%s': %s", gameDataPath.c_str(), e.what());
		}
	}

	void LoadGameData(GameData& gameData, const std::string& gameDataName, const ComponentSerializersHolder& componentSerializers)
	{
		namespace fs = std::filesystem;
		fs::path gameDataPath(gameDataName);

		// if it's name, we search the map in maps folder
		if (gameDataName.find_first_of("/\\.") == std::string::npos)
		{
			gameDataPath = GAME_DATA_PATH / (gameDataName + ".json");
		}

		try
		{
			std::ifstream mapFile(gameDataPath);
			nlohmann::json mapJson;
			mapFile >> mapJson;

			if (const auto& worldObject = mapJson.at("gameData"); worldObject.is_object())
			{
				gameData.fromJson(worldObject, componentSerializers);
			}
		}
		catch(const nlohmann::detail::exception& e)
		{
			LogError("Can't parse gameData '%s': %s", gameDataPath.c_str(), e.what());
		}
		catch(const std::exception& e)
		{
			LogError("Can't open gameData '%s': %s", gameDataPath.c_str(), e.what());
		}
	}
}
