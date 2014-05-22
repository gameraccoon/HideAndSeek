#include "LuaAiState.h"

#include "LuaType.h"
#include "LuaTypeLog.h"
#include "LuaFunction.h"


LuaAiState::LuaAiState(World *world, IBody *body, Role *role) : AiState(world, body, role)
{
	scriptUpdateInterval = 0.01f;
	lastExecutionTime = 0.f;

	this->script = new LuaInstance();

	LuaType::registerLog(this->script, "Log");

	// dummies for events
	this->script->execScript("function OnTakeDamage(instigator, damageValue) end\n"
		"function OnSeeEnemy(enemy) end\n"
		"function OnHearNoise(sound) end\n");

	this->script->execScriptFromFile("test.lua");
}

LuaAiState::~LuaAiState(void)
{
	delete this->script;
}

void LuaAiState::onTakeDamage(IActor* instigator, float damageValue, Vector2D impulse)
{
	LuaType::registerConstant<IBody>(this->script, "body", this->body);

	LuaFunction luaFunction;
	luaFunction.readyToRunFunction(this->script, "OnTakeDamage");

	this->script->sendToLua<void*>(instigator);
	this->script->sendToLua<double>(damageValue);

	luaFunction.runFunction(2, 0);
	luaFunction.clearAfterFunction();
	
	this->script->removeSymbol("body");
}

void LuaAiState::onSeeEnemy(IActor *enemy)
{
	LuaType::registerConstant<IBody>(this->script, "body", this->body);

	LuaFunction luaFunction;
	luaFunction.readyToRunFunction(this->script, "OnSeeEnemy");

	this->script->sendToLua<void*>(enemy);

	luaFunction.runFunction(1, 0);
	luaFunction.clearAfterFunction();
	
	this->script->removeSymbol("body");
}

void LuaAiState::onHearNoise(SoundVolume *sound)
{
}
