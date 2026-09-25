#include "TileType.h"

namespace
{
	constexpr core::DirectionMask NORTH{ core::toMask(core::Direction::North) };
	constexpr core::DirectionMask EAST{ core::toMask(core::Direction::East) };
	constexpr core::DirectionMask SOUTH{ core::toMask(core::Direction::South) };
	constexpr core::DirectionMask WEST{ core::toMask(core::Direction::West) };
} // namespace

namespace game::board
{
	core::DirectionMask baseConnections(TileType type)
	{
		switch (type)
		{
		case TileType::Straight: return NORTH | SOUTH;
		case TileType::Corner: return NORTH | EAST;
		case TileType::Tee: return NORTH | EAST | WEST;
		case TileType::Cross: return core::DIRECTION_ALL;
		case TileType::Source: return SOUTH;
		case TileType::Goal: return NORTH;
		case TileType::Locked: return NORTH | SOUTH;
		case TileType::Blank: return 0;
		}
		return 0;
	}

	bool isRotatable(TileType type)
	{
		switch (type)
		{
		case TileType::Straight:
		case TileType::Corner:
		case TileType::Tee:
		case TileType::Cross: return true;
		case TileType::Source:
		case TileType::Goal:
		case TileType::Locked:
		case TileType::Blank: return false;
		}
		return false;
	}

	std::optional<TileType> tileTypeFromSymbol(char symbol)
	{
		switch (symbol)
		{
		case 'S': return TileType::Straight;
		case 'C': return TileType::Corner;
		case 'T': return TileType::Tee;
		case 'X': return TileType::Cross;
		case 'P': return TileType::Source;
		case 'G': return TileType::Goal;
		case 'L': return TileType::Locked;
		case 'B': return TileType::Blank;
		default: return std::nullopt;
		}
	}
} // namespace game::board
