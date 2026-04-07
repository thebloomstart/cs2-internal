#pragma once
#include <string>

class Spec
{
public:
	void spectators();
};

inline const auto g_Spectators = std::make_unique<Spec>();
