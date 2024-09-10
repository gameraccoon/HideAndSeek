#pragma once

#include "src/componenteditcontent/typeseditconstructor.h"
#include <raccoon-ecs/entity.h>

#include "EngineData/Geometry/Border.h"
#include "EngineData/Geometry/Rotator.h"
#include "EngineData/Geometry/Vector2D.h"

#include "GameData/Geometry/Hull.h"
#include "GameData/Resources/AnimationClipDescription.h"
#include "GameData/Resources/AnimationClipParams.h"
#include "GameData/Resources/SpriteDescription.h"
#include "GameData/Resources/SpriteParams.h"
#include "GameData/Spatial/CellPos.h"
#include "GameData/Spatial/SpatialEntity.h"

namespace TypesEditConstructor
{
	template<>
	Edit<AnimationClipParams>::Ptr FillEdit<AnimationClipParams>::Call(QLayout* layout, const QString& label, const AnimationClipParams& initialValue);

	template<>
	Edit<AnimationClipDescription>::Ptr FillEdit<AnimationClipDescription>::Call(QLayout* layout, const QString& label, const AnimationClipDescription& initialValue);

	template<>
	Edit<CellPos>::Ptr FillEdit<CellPos>::Call(QLayout* layout, const QString& label, const CellPos& initialValue);

	template<>
	Edit<Entity>::Ptr FillEdit<Entity>::Call(QLayout* layout, const QString& label, const Entity& initialValue);

	template<>
	Edit<SimpleBorder>::Ptr FillEdit<SimpleBorder>::Call(QLayout* layout, const QString& label, const SimpleBorder& initialValue);

	template<>
	Edit<Hull>::Ptr FillEdit<Hull>::Call(QLayout* layout, const QString& label, const Hull& initialValue);

	template<>
	Edit<Rotator>::Ptr FillEdit<Rotator>::Call(QLayout* layout, const QString& label, const Rotator& initialValue);

	template<>
	Edit<SpatialEntity>::Ptr FillEdit<SpatialEntity>::Call(QLayout* layout, const QString& label, const SpatialEntity& initialValue);

	template<>
	Edit<SpriteDescription>::Ptr FillEdit<SpriteDescription>::Call(QLayout* layout, const QString& label, const SpriteDescription& initialValue);

	template<>
	Edit<SpriteParams>::Ptr FillEdit<SpriteParams>::Call(QLayout* layout, const QString& label, const SpriteParams& initialValue);

	template<>
	Edit<StringId>::Ptr FillEdit<StringId>::Call(QLayout* layout, const QString& label, const StringId& initialValue);

	template<>
	Edit<Vector2D>::Ptr FillEdit<Vector2D>::Call(QLayout* layout, const QString& label, const Vector2D& initialValue);
} // namespace TypesEditConstructor
