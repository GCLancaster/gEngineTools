
#include "Carcassonne.h"

std::array<std::string, 7> Carcassonne::Tile::Render(const bool & bDebugDisplay /*= false*/) const
{
	const auto top = std::string(m_road[csTop] ? (m_roadGate[csTop] ? "#" : "R") : "\'");
	const auto tmi = std::string(m_road[csTop] ? "R" : " ");
	const auto bot = std::string(m_road[csBottom] ? (m_roadGate[csBottom] ? "#" : "R") : "_");
	const auto bmi = std::string(m_road[csBottom] ? "R" : " ");
	const auto lef = std::string(m_road[csLeft] ? (m_roadGate[csLeft] ? "# " : "R ") : "| ");
	const auto lmi = std::string(m_road[csLeft] ? "R " : "  ");
	const auto rig = std::string(m_road[csRight] ? (m_roadGate[csRight] ? " #" : " R") : " |");
	const auto rmi = std::string(m_road[csRight] ? " R" : "  ");

	const auto cen = GENG::fixedLength<std::string>(m_cardNumber, 3);
	const auto xpo = GENG::fixedLength<std::string>(m_pos.x, 3);
	const auto ypo = GENG::fixedLength<std::string>(m_pos.y, 3);
	std::array<std::string, 7> row;
	row[0] = "|'''''" + top + "'''''|";
	row[1] = "|     " + tmi + "     |";
	row[2] = "|     " + tmi + "     |";
	row[3] = lef + lmi + lmi + '+' + rmi + rmi + rig;
	row[4] = "|     " + bmi + " " + xpo +" |";
	row[5] = "| " + cen + " " + bmi + " " + ypo + " |";
	row[6] = "|_____" + bot + "_____|";

	if (bDebugDisplay)
	{
		DBG(row[0]);
		DBG(row[1]);
		DBG(row[2]);
		DBG(row[3]);
		DBG(row[4]);
		DBG(row[5]);
		DBG(row[6]);
	}

	return row;
}

void Carcassonne::Tile::GenerateTile()
{
	// Reset
	for (int i = 0; i < csCOUNT; i++)
	{
		m_road[i] = false;
		m_roadGate[i] = false;
	}

	// Randomise
	for (int i = 0; i < csCOUNT; i++)
	{
		m_road[i] = (std::rand() % 2) == 1;
	}
}

Carcassonne::Board::Board()
{
	m_tilePool[NUM_TILES - 1] = Tile(0);
	m_tilePool[NUM_TILES - 1].GenerateTile();
	for (int i = NUM_TILES - 2; i >= 0; i--)
	{
		m_tilePool[i] = Tile((NUM_TILES - i - 1));
		m_tilePool[i].GenerateTile();
	}
	PlaceTile({ 0, 0 }, &m_tilePool[NUM_TILES - 1]);
}

bool Carcassonne::Board::GetIsFinished() const
{
	return m_poolPos == 0;
}

Carcassonne::Tile * Carcassonne::Board::GetTopOfStack()
{
	Tile * pNextTile = &m_tilePool[m_poolPos];
	if (m_poolPos != 0)
		m_poolPos--;
	LOG("Retriving tile " << std::to_string(pNextTile->m_cardNumber) << ".");
	return pNextTile;
}

Carcassonne::Tile * Carcassonne::Board::GetBottomOfStack()
{
	return &m_tilePool[0];
}

Carcassonne::Tile * Carcassonne::Board::GetStartTile()
{
	return &m_tilePool[NUM_TILES - 1];
}

void Carcassonne::Board::Render()
{
	//for (int y = m_tileMap.map.size() - 1; y >= 0 ; y--)
	for (int y = 0; y < m_tileMap.map.size() ; y++)
	{
		std::array<std::string, 7> lines;
		//for (int x = m_tileMap.map[y].size() - 1; x >= 0 ; x--)
		for (int x = 0; x < m_tileMap.map[y].size(); x++)
		{
			const auto pTile = m_tileMap.map[x][y];
			if (pTile != nullptr)
			{
				const auto tileLines = pTile->Render();
				for (int i = 0; i < lines.size() ; i++)
					lines[i].append(tileLines[i]);
			}
			else
			{
				for (int i = 0; i < lines.size(); i++)
				{
					if (i == 0)
						lines[i].append("|'''''''''''|");
					else if (i == lines.size() - 1)
						lines[i].append("|___________|");
					else
						lines[i].append("|           |");
				}
			}
		}
		for (int i = 0; i < lines.size(); i++)
		{
			DBG(lines[i]);
		}
	}
}

bool Carcassonne::Board::_CheckTilePlacementIsValid_Road(const TilePosition & placementPosition, Tile * pTile)
{
	// Argument check
	if (pTile == nullptr)
	{
		LOG("Invalid pTile");
		return false;
	}

	if (!m_tileMap.CheckPosition(placementPosition))
	{
		LOG("Invalid placement position");
		return false;
	}

	//LOG("Attempting to place tile at position " << std::to_string(placementPosition.x) << ", " << std::to_string(placementPosition.y) << ".");

	// Check adjacent tiles
	const auto adjacents = SGetAdjacentTiles(placementPosition);
	for (int edgeIDX = 0; edgeIDX < csCOUNT; edgeIDX++)
	{
		const auto pAdjacentTile = adjacents[edgeIDX];
		if (pAdjacentTile == nullptr)
			continue;

		const TileEdge edge = static_cast<TileEdge>(edgeIDX);
		const TileEdge adjEdge = SGetInverseTilePosition(edge);
		
		const bool edgeIsRoad = pTile->m_road[edge];
		const bool adjEdgeIsRoad = pAdjacentTile->m_road[adjEdge];

		if (edgeIsRoad != adjEdgeIsRoad)
		{
			LOG("Cannot place tile. Adjacent tile to the " << gTilePositionNames[edge] << " mismatches.");
			return false;
		}
		else
		{
			//LOG("Can place tile. Adjacent tile to the " << gTilePositionNames[edge] << " matches.");
			pAdjacentTile->Render(true);
			pTile->Render(true);
			pTile->m_roadGate[edge] = true;
			pAdjacentTile->m_roadGate[adjEdge] = true;
		}
	}

	return true;
}

bool Carcassonne::Board::PlaceTile(const TilePosition & placementPosition, Tile * pTile)
{
	// Argument check
	if (!m_tileMap.CheckPosition(placementPosition))
	{
		LOG("Cannot placed tile " << std::to_string(pTile->m_cardNumber) << " at position " << std::to_string(placementPosition.x) << ", " << std::to_string(placementPosition.y) << ". Tile is off the board.");
		return false;
	}
	if (m_tileMap.GetTile(placementPosition) != nullptr)
	{
		LOG("Cannot placed tile " << std::to_string(pTile->m_cardNumber) << " at position " << std::to_string(placementPosition.x) << ", " << std::to_string(placementPosition.y) << ". A tile already exists on this spot.");
		return false;
	}
	if (pTile == nullptr)
		return false;

	// Place tile on valid edge
	if (!_CheckTilePlacementIsValid_Road(placementPosition, pTile))
	{
		LOG("Cannot placed tile " << std::to_string(pTile->m_cardNumber) << " at position " << std::to_string(placementPosition.x) << ", " << std::to_string(placementPosition.y) << ". A adjacent tiles do not allow.");
		return false;
	}

	// Add new placement location
	m_tileMap.SetTile(placementPosition, pTile);
	pTile->SetPos(placementPosition);

	LOG("Placed tile " << std::to_string(pTile->m_cardNumber) << " at position " << std::to_string(placementPosition.x) << ", " << std::to_string(placementPosition.y) << ".");
	
	return true;
}

bool Carcassonne::Board::AttemptToPlaceTileAroundPosition(const TilePosition & placementPosition, Tile * pTile)
{
	const auto adjacents = Carcassonne::SGetAdjacentTilePositions(placementPosition);
	for (int i = 0; i < adjacents.size(); i++)
	{
		if (PlaceTile(adjacents[i], pTile))
		{
			m_placedTiles.push_back(GetTileMapPosition(pTile));
			return true;
		}
	}

	m_skippedTiles.push_back(GetTileMapPosition(pTile));
	return false;
}
