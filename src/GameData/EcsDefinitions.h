#pragma once

#include "Base/Types/String/StringId.h"

#include "ECS/EntityManager.h"
#include "ECS/EntityView.h"
#include "ECS/ComponentSetHolder.h"
#include "ECS/ComponentFactory.h"
#include "ECS/TypedComponent.h"
#include "ECS/Entity.h"

using EntityManager = Ecs::EntityManagerImpl<StringId>;
using EntityView = Ecs::EntityViewImpl<StringId>;
using ComponentSetHolder = Ecs::ComponentSetHolderImpl<StringId>;
using ComponentFactory = Ecs::ComponentFactoryImpl<StringId>;
using TypedComponent = Ecs::TypedComponentImpl<StringId>;
using ConstTypedComponent = Ecs::ConstTypedComponentImpl<StringId>;
using Entity = Ecs::Entity;
using OptionalEntity = Ecs::OptionalEntity;
