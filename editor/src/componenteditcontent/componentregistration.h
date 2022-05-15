#ifndef COMPONENTREGISTRATION_H
#define COMPONENTREGISTRATION_H

#include <map>
#include <memory>
#include <string>

#include "abstracteditfactory.h"

namespace ComponentRegistration
{
	void RegisterToEditFactory(std::map<StringId, std::unique_ptr<AbstractEditFactory>>& factories);
}

#endif // COMPONENTREGISTRATION_H
