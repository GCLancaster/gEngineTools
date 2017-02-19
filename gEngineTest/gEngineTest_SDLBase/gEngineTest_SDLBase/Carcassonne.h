
#include "gEngine\System\Logging\gLogger.h"
#include "gEngine\Useful.h"
#include <xutility>

namespace Carcassonne
{
	enum PlayerID
	{
		NoPlayer,
		Player_1,
		Player_2,
		Player_3
	};
	enum EdgeType
	{
		ctFarm,
		ctCity,
		ctRoad,

		ctMonk, // Centre use only

		ctCOUNT,
		ctNULL
	};
	static const char * gEdgeTypeNamess[] =
	{
		"Farm",
		"City",
		"Road",
		"Monk"
	};
	enum TileEdge
	{
		csTop,
		csBottom,
		csLeft,
		csRight,

		csCOUNT,
		csNULL
	};
	static const char * gTilePositionNames[] =
	{
		"Top",
		"Bottom",
		"Left",
		"Right"
	};
	static TileEdge SGetInverseTilePosition(const TileEdge & pos)
	{
		switch (pos)
		{
		case csTop: return csBottom;
		case csBottom: return csTop;
		case csLeft: return csRight;
		case csRight: return csLeft;
		case csCOUNT:
		case csNULL:
		default:
			return csNULL;
		}
	}

	union TilePosition
	{
		int m_pos[2];
		struct { int X; int Y; };
		struct { int x; int y; };
		TilePosition() : X(0), Y(0) {};
		TilePosition(const int & x, const int & y) : X(x), Y(y) {};
	};
	static std::array<TilePosition, 4> SGetAdjacentTilePositions(const TilePosition & pos)
	{
		const std::array<TilePosition, 4> arr =
		{
			// csTop
			// csBottom
			// csLeft
			// csRight
			TilePosition(pos.X,		pos.Y - 1),
			TilePosition(pos.X,		pos.Y + 1),
			TilePosition(pos.X - 1, pos.Y	 ),
			TilePosition(pos.X + 1, pos.Y	 )
		};
		return arr;
	}

	class Tile
	{
	public:
		bool m_bPlaced = false;
		int m_cardNumber = -1;
		TilePosition m_pos;
		bool m_road[csCOUNT];
		bool m_roadGate[csCOUNT];

		Tile(){};
		Tile(const int & id) : m_cardNumber(id) {};

		void SetPos(const TilePosition & pos) { m_pos = pos; }
		TilePosition GetPos() const { return m_pos; }

		void IncrementPosition(const int & x, const int & y) { m_pos.X += x; m_pos.Y += y; };

		std::array<std::string, 7> Carcassonne::Tile::Render(const bool & bDebugDisplay = false) const;
		void GenerateTile();
	};

	class Board
	{
		static const size_t NUM_TILES = 50;
		static const size_t BOARD_WIDTH = 10;
		static const size_t BOARD_HEIGHT = 10;

		size_t m_poolPos = NUM_TILES - 2;
		std::array<Tile, NUM_TILES> m_tilePool;
		std::vector<int> m_placedTiles;
		std::vector<int> m_skippedTiles;

		struct TileMap  
		{
			std::array<std::array<Tile *, BOARD_WIDTH>, BOARD_HEIGHT> map;

			bool CheckPosition(const TilePosition & pos)
			{
				const int x = pos.x + (BOARD_WIDTH / 2);
				const int y = pos.y + (BOARD_HEIGHT / 2);
				if ((x < 0 || x >= (BOARD_WIDTH)) || (y < 0 || y >= (BOARD_HEIGHT)))
					return false;
				return true;
			};
			bool SetTile(const TilePosition & pos, Tile * pTile) 
			{
				if (!CheckPosition(pos))
					return false;
				const int x = pos.x + (BOARD_WIDTH / 2);
				const int y = pos.y + (BOARD_HEIGHT / 2);
				map[x][y] = pTile; 
				return true;
			};
			Tile * GetTile(const TilePosition & pos) 
			{
				if (!CheckPosition(pos))
					return nullptr;
				const int x = pos.x + (BOARD_WIDTH / 2);
				const int y = pos.y + (BOARD_HEIGHT / 2);
				return map[x][y]; 
			};

			TileMap()
			{
				for (int y = 0; y < map.size(); y++)
					map[y].assign(nullptr);
			};
		};
		TileMap m_tileMap;
		
	public:
		Board();

		bool GetIsFinished() const;
		Tile * GetTopOfStack();
		Tile * GetBottomOfStack();
		Tile * GetStartTile();

		Tile * GetTile(const int & position)
		{
			if (position < 0 || position >= NUM_TILES)
			{
				LOG("Attempting to access TilePool with invalid index: " << std::to_string(position) << ".");
				return nullptr;
			}
			return &m_tilePool[position];
		}
		std::vector<int> GetPlacedTiles() const { return m_placedTiles; }
		std::vector<int> GetSkippedTiles() const { return m_skippedTiles; }
				
		void Render();
		
		bool _CheckTilePlacementIsValid_Road(const TilePosition & placementPosition, Tile * pTile);

		bool PlaceTile(const TilePosition & placementPosition, Tile * pTile);
		bool AttemptToPlaceTileAroundPosition(const TilePosition & placementPosition, Tile * pTile);

		std::array<Tile*, 4> SGetAdjacentTiles(const TilePosition & pos)
		{
			const auto adj = SGetAdjacentTilePositions(pos);
			return
			{
				// csTop
				// csBottom
				// csLeft
				// csRight
				m_tileMap.GetTile(adj[0]),
				m_tileMap.GetTile(adj[1]),
				m_tileMap.GetTile(adj[2]),
				m_tileMap.GetTile(adj[3])
			};
		};
		TilePosition GetRandomPosition()
		{
			return{ ((std::rand() % NUM_TILES * 2) - NUM_TILES), ((std::rand() % NUM_TILES * 2) - NUM_TILES) };
		}
		size_t GetTileMapPosition(Tile * pTile) const { return pTile - m_tilePool.data(); };
	};
}

