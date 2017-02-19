
#include "Carcassonne.h"

std::array<std::string, 7> Carcassonne::Tile::RenderGrid(const bool & bDebugDisplay /*= false*/) const
{
	std::array<std::string, 7> row;
	row[0] = "|'''''''''''|";
	row[1] = "|           |";
	row[2] = "|           |";
	row[3] = "|     +     |";
	row[4] = "|           |";
	row[5] = "|           |";
	row[6] = "|___________|";
	return row;
}

std::array<std::string, 7> Carcassonne::Tile::RenderRoad(std::array<std::string, 7> & row, const bool & bDebugDisplay /*= false*/) const
{
	const auto top = std::string(m_road[csTop] ? (m_roadGate[csTop] ? "#" : "R") : "");
	const auto tmi = std::string(m_road[csTop] ? "R" : "");
	const auto bot = std::string(m_road[csBottom] ? (m_roadGate[csBottom] ? "#" : "R") : "");
	const auto bmi = std::string(m_road[csBottom] ? "R" : "");
	const auto lmi = std::string(m_road[csLeft] ? "RR" : "");
	const auto left = lef + lmi + lmi;
	const auto rmi = std::string(m_road[csRight] ? "RR" : "");
	const auto right = rmi + rmi + rig;

	static const size_t rowSize = row[0].size();

	if(m_road[csTop]) row[0].replace(row[0].begin() + (rowSize / 2), row[0].end() - (rowSize / 2), top);
	if(m_road[csTop]) row[1].replace(row[1].begin() + (rowSize / 2), row[1].end() - (rowSize / 2), tmi);
	if(m_road[csTop]) row[2].replace(row[2].begin() + (rowSize / 2), row[2].end() - (rowSize / 2), tmi);
	if(m_road[csLeft]) row[3].replace(row[3].begin(), row[3].begin() + (rowSize / 2), left);
	if(m_road[csRight]) row[3].replace(row[3].end() - (rowSize / 2), row[3].end(), right);
	if(m_road[csBottom]) row[4].replace(row[4].begin() + (rowSize / 2), row[4].end() - (rowSize / 2), bmi);
	if(m_road[csBottom]) row[5].replace(row[5].begin() + (rowSize / 2), row[5].end() - (rowSize / 2), bmi);
	if(m_road[csBottom]) row[6].replace(row[6].begin() + (rowSize / 2), row[6].end() - (rowSize / 2), bot);

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

std::array<std::string, 7> Carcassonne::Tile::RenderCity(std::array<std::string, 7> & row, const bool & bDebugDisplay /*= false*/) const
{
	static const std::string cornerLeft_0 = "|######";
	static const std::string cornerLeft_1 = "|##";
	std::string cornerRight_0(cornerLeft_0); std::reverse(std::begin(cornerRight_0), std::end(cornerRight_0));
	std::string cornerRight_1(cornerLeft_1); std::reverse(std::begin(cornerRight_1), std::end(cornerRight_1));
	static const std::string centre_0 = "#######";
	static const std::string centre_1 = "#####";
	static const std::string centre_2 = "###";
	static const size_t rowSize = row[0].size();

	auto AdjustRowsToCityEdge = [&](const CityEdgePoints & ceEdge)
	{
		switch (ceEdge)
		{
		case Carcassonne::ceTopLeft:
			row[0].replace(row[0].begin(), row[0].begin() + cornerLeft_0.size(), cornerLeft_0);
			row[1].replace(row[1].begin(), row[1].begin() + cornerLeft_1.size(), cornerLeft_1);
			break;
		case Carcassonne::ceTop:
			row[0].replace(row[0].begin() + ((rowSize - centre_0.size()) / 2), row[0].end() - ((rowSize - centre_0.size()) / 2), centre_0);
			row[1].replace(row[1].begin() + ((rowSize - centre_1.size()) / 2), row[1].end() - ((rowSize - centre_1.size()) / 2), centre_1);
			row[2].replace(row[2].begin() + ((rowSize - centre_2.size()) / 2), row[2].end() - ((rowSize - centre_2.size()) / 2), centre_2);
			break;
		case Carcassonne::ceTopRight:
			row[0].replace(row[0].end() - cornerRight_0.size(), row[0].end(), cornerRight_0);
			row[1].replace(row[1].end() - cornerRight_1.size(), row[1].end(), cornerRight_1);
			break;
		case Carcassonne::ceBottomLeft:
			row[5].replace(row[5].begin(), row[5].begin() + cornerLeft_1.size(), cornerLeft_1);
			row[6].replace(row[6].begin(), row[6].begin() + cornerLeft_0.size(), cornerLeft_0);
			break;
		case Carcassonne::ceBottom:
			row[4].replace(row[4].begin() + ((rowSize - centre_2.size()) / 2), row[4].end() - ((rowSize - centre_2.size()) / 2), centre_2);
			row[5].replace(row[5].begin() + ((rowSize - centre_1.size()) / 2), row[5].end() - ((rowSize - centre_1.size()) / 2), centre_1);
			row[6].replace(row[6].begin() + ((rowSize - centre_0.size()) / 2), row[6].end() - ((rowSize - centre_0.size()) / 2), centre_0);
			break;
		case Carcassonne::ceBottomRight:
			row[5].replace(row[5].end() - cornerRight_1.size(), row[5].end(), cornerRight_1);
			row[6].replace(row[6].end() - cornerRight_0.size(), row[6].end(), cornerRight_0);
			break;
		case Carcassonne::ceLeft:
			row[0].replace(row[0].begin(), row[0].begin() + 3, "|##");
			row[1].replace(row[1].begin(), row[1].begin() + 4, "|###");
			row[2].replace(row[2].begin(), row[2].begin() + 5, "|####");
			row[3].replace(row[3].begin(), row[3].begin() + 6, "|#####");
			row[4].replace(row[4].begin(), row[4].begin() + 5, "|####");
			row[5].replace(row[5].begin(), row[5].begin() + 4, "|###");
			row[6].replace(row[6].begin(), row[6].begin() + 3, "|##");
			break;
		case Carcassonne::ceRight:
			row[0].replace(row[0].end() - 3, row[0].end(), "##|");
			row[1].replace(row[1].end() - 4, row[1].end(), "###|");
			row[2].replace(row[2].end() - 5, row[2].end(), "####|");
			row[3].replace(row[3].end() - 6, row[3].end(), "#####|");
			row[4].replace(row[4].end() - 5, row[4].end(), "####|");
			row[5].replace(row[5].end() - 4, row[5].end(), "###|");
			row[6].replace(row[6].end() - 3, row[6].end(), "##|");
			break;
		case Carcassonne::ceCOUNT:
		case Carcassonne::ceNULL:
		default:
			break;
		}
	};
	
	AdjustRowsToCityEdge(m_cityPoints[0][0]);
	AdjustRowsToCityEdge(m_cityPoints[0][1]);

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

std::array<std::string, 7> Carcassonne::Tile::RenderData(std::array<std::string, 7> & row, const bool & bDebugDisplay /*= false*/) const
{
	const auto cen = GENG::fixedLength<std::string>(m_cardNumber, 3);
	const auto xpo = GENG::fixedLength<std::string>(m_pos.x, 3);
	const auto ypo = GENG::fixedLength<std::string>(m_pos.y, 3);

	row[4].replace(row[4].begin() + 2, row[4].begin() + 2 + cen.size(), cen);
	row[4].replace(row[4].end() - 2 - xpo.size(), row[4].end() - 2, xpo);
	row[5].replace(row[5].end() - 2 - ypo.size(), row[5].end() - 2, ypo);

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
	for (int i = 0; i < 2 ; i++)
	{
		m_cityPoints[i][0] = ceNULL;
		m_cityPoints[i][1] = ceNULL;
	}

	//////////////////////////////////////////////////////////////////////////
	// Randomise
	int type[csCOUNT];
	type[csTop] = (std::rand() % 3);
	type[csBottom] = (std::rand() % 3);
	type[csLeft] = (std::rand() % 3);
	type[csRight] = (std::rand() % 3);

	// Set road
	for (int i = 0; i < csCOUNT; i++)
		m_road[i] = (type[i] == 1);

	// Set city
	std::array<std::vector<CityEdgePoints>, 4> pointsAvailable;
	std::array<std::vector<CityEdgePoints>, 4> alternateEdges;
	std::array<bool, 4> edgeHasCity = { false, false, false, false };
	pointsAvailable[csTop] = { ceTopLeft, ceTop, ceTopRight };
	pointsAvailable[csBottom] = { ceBottomLeft, ceBottom, ceBottomRight };
	pointsAvailable[csLeft] = { ceTopLeft, ceLeft, ceBottomLeft };
	pointsAvailable[csRight] = { ceTopRight, ceRight, ceBottomRight };
	alternateEdges[csTop] = { ceBottomLeft, ceBottom, ceBottomRight, ceLeft, ceRight };
	alternateEdges[csBottom] = { ceTopLeft, ceTop, ceTopRight, ceLeft, ceRight };
	alternateEdges[csLeft] = { ceTop, ceTopRight, ceBottom, ceBottomRight, ceRight };
	alternateEdges[csRight] = { ceTopLeft, ceTop, ceBottom, ceBottomLeft, ceLeft };
	int numBlocks = 0;
	for (int i = 0; i < csCOUNT; i++)
	{
		if (type[i] == 2 && !edgeHasCity[i] && numBlocks < 2 && ((std::rand() % 2) == 1))
		{
			int block = numBlocks++;
			m_cityPoints[block][0] = pointsAvailable[i][(std::rand() % pointsAvailable[i].size())];
			m_cityPoints[block][1] = alternateEdges[i][(std::rand() % alternateEdges[i].size())];
			const auto edgePair = STileEdgeFromCityEdge(m_cityPoints[block][1]);
			edgeHasCity[i] = true;
			edgeHasCity[edgePair.first] = true;
			edgeHasCity[edgePair.second] = true;
		}
	}

	auto row = RenderGrid();
	RenderCity(row);
	RenderRoad(row);
	RenderData(row, true);
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
	m_placedTiles.push_back(NUM_TILES - 1);
}

bool Carcassonne::Board::GetIsFinished() const
{
	const auto numUsed = (m_placedTiles.size() + m_skippedTiles.size());
	return numUsed == NUM_TILES;
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
				auto tileLines = pTile->RenderGrid(); 
				pTile->RenderRoad(tileLines);
				pTile->RenderCity(tileLines);
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
						lines[i].append("|-----------|");
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
