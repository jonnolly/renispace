////////////////////////////////////////////////////////////////////////////////////////////////////
// CMap.h
// Created: 15/11/15
// Author: Hannah Howell
//

#ifndef SRC_CMAP_H_
#define SRC_CMAP_H_



//~~~ INCLUDES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "EnumsHeader.h"
#include "Instructions.h"
#include<vector>
#include<string>

//~~~ CLASS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CMap
{
	// === Constructors and Destructors =============================================================
public:
	CMap(std::string filepath);
	CMap(int room_height, int room_width);

	// === Member Variables =========================================================================
private:
	std::vector<std::vector<int>> m_cellMap;
	std::vector<std::vector<ERoom>> m_roomMap;
	int m_cellheight;
	int m_cellwidth;
	int m_firstRoom;
	int m_exitRoom;
	std::vector<int> m_entranceCell;
	std::vector<int> m_exitCell;

	// === Location Tracking ==========================================================================

	int m_currentRoom;
	int m_currentVertex;
	EOrientation m_currentOrientation;


	// === Accessor Functions =========================================================================
public:
	std::vector<std::vector<ERoom>>	GetRoomMap();
	ERoom GetRoomType(int room_index);
	std::vector<std::vector<int>> GetCellMap();
	int GetEntranceRoom();
	int GetExitRoom();
	std::vector<int> GetEntranceCell();
	std::vector<int> GetExitCell();
	static std::vector<int> GetRoomVertices(ERoom room_type);
	int GetEntranceVertex();
	int GetExitVertex();

	int GetCurrentVertex();
	int GetCurrentRoom();

	void SetCurrentRoom(int new_room);
	void SetCurrentVertex(int new_vertex);



	// === Public Functions =========================================================================

	void UpdateRoomMap();
	void UpdateCellMap();
	void CalculateBlockRooms(std::vector<int> *pBlockRooms);
	std::vector<int> CalculateRoomVertices(int room_index);
	std::vector<int> CalculateRoomVertices(int row, int col);
	void FollowInstructions(CInstructions &inputInstructions);



	// === Private Functions ========================================================================
private:
	void CreateRoomMap();
	void ComputeCellMapSize();
	std::vector<int> CalculateRoomVertices(std::vector<int> coord);
	std::vector<int> RoomIndextoCoord(int room_index);
	
};


#endif SRC_CMap_H_
