#pragma once

#include <map>
#include <memory>

#include "abstracteditfactory.h"

namespace ComponentRegistration
{
	void RegisterToEditFactory(std::map<StringId, std::unique_ptr<AbstractEditFactory>>& factories);
}
