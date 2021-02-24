#pragma once

#include <algorithm>

#include "Object.h"
#include "PerlinNoise.h"

class WorldManager {
public:
	struct Pos {
	public:
		Pos() {
			x = 0; y = 0; z = 0;
		}
		Pos(int nx, int ny, int nz) {
			x = nx; y = ny; z = nz;
		}
		Pos& operator=(Pos& p) {
			x = p.x;
			y = p.y;
			z = p.z;
			return (*this);
		}
		bool operator!=(Pos& p) {
			return (x != p.x || y != p.y || z != p.z);
		}
		int& operator[](int a) {
			switch (a) {
			case(0):
			case(3):
				return x;
			case(1):
			case(4):
				return y;
			case(2):
			case(5):
				return z;
			default:
				assert(true);
				return x;
			}
		}

		int x;
		int y;
		int z;
	};

	class Chunk {
	public:
		//Initializers
		Chunk(Pos pos);
		void Init(Pos pos);
		Chunk& operator=(Chunk& c);

		//Getters
		bool GetAcitve() { return mActive; };
		Pos GetPos() { return mPosition; };
		int GetID() { return mID; };
		size_t GetActiveIndex() { return mActiveIndex; };
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> GetBlocks() { return mBlocks; };
		std::shared_ptr<std::vector<std::shared_ptr<InstanceData>>> GetInstanceDatas() { return mInstanceDatas; };
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> GetActiveBlocks() { return mActiveBlocks; };

		//Setters
		void SetAcitve(bool active) { mActive = active; };
		void SetActiveIndex(size_t ind) { mActiveIndex = ind; };

		//Iterators
		//Get
		size_t GetBlockStartIndex() { return mBlockStartIndex; };
		size_t GetGObjStartIndex() { return mGObjStartIndex; };
		size_t GetInstanceStartIndex() { return mInstanceStartIndex; };
		//Setters
		void SetStartIndexes(size_t block, size_t obj, size_t inst) {
			mBlockStartIndex = block; mGObjStartIndex = obj; mInstanceStartIndex = inst; }
		void SetBlockStartIndex(size_t b) { mBlockStartIndex = b; };
		void SetObjStartIndex(size_t b) { mGObjStartIndex = b; };
		void SetInstanceStartIndex(size_t b) { mInstanceStartIndex = b; };

	private:
		bool mActive = false;
		Pos mPosition; //Bottom left front corner. (-x, -y, -z)
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> mBlocks;
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> mActiveBlocks;
		std::shared_ptr<std::vector<std::shared_ptr<InstanceData>>> mInstanceDatas;

		int mID = -1;
		size_t mActiveIndex = -1;

		size_t mBlockStartIndex = -1;
		size_t mGObjStartIndex = -1;
		size_t mInstanceStartIndex = -1;
	};

	WorldManager();
	~WorldManager();

	void Init(std::shared_ptr<std::unordered_map<std::string, int>> materialIndexes);
	void CreateWorld();

	void UpdatePlayerPosition(DirectX::XMFLOAT3 pos);

	bool IsChunkCoordValid(int x, int y, int z);
	void LoadChunk(int x, int y, int z);
	void UnloadChunk(int x, int y, int z);
	void SwapChunk(Pos old, Pos neew);
	void UnloadAllChunks();

	int GetChunkSize() { return sChunkSize; };
	int GetNumberOfChunksToLoad() { return mChunksToLoad; };

	int GetTotalAmountOfBlocks();
	int GetPlayerChunkIndex(DirectX::XMFLOAT3 pos);
	Pos GetPlayerChunkCoords(DirectX::XMFLOAT3 pos);
	std::shared_ptr<Chunk> GetPlayerChunk(DirectX::XMFLOAT3 pos);
	void LoadFirstChunks(DirectX::XMFLOAT3 pos);

	void RelocatePlayer(DirectX::XMFLOAT3 newPos);

	//Returns an index (in Block::sAllBlocks) used by a given block 
	std::shared_ptr<Block> GetBlock(DirectX::XMFLOAT3 pos);

	//DEBUG
	void PrintChunkOrder();

private:
	static void CreateCube(std::string materialName, XMFLOAT3 pos, bool active, std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, std::shared_ptr<std::vector<std::shared_ptr<InstanceData>>> blockInstances);


	//Get the chunk from its chunk-coordiantes
	std::shared_ptr<Chunk> GetChunk(int x, int y, int z);
	void SetChunkActive(std::shared_ptr<Chunk> chunk, bool active);

private:
	static std::shared_ptr<std::unordered_map<std::string, int>> sMaterialIndexes;

	static PerlinNoise sNoise;
	std::vector<std::shared_ptr<Chunk>> mChunks;
	std::vector<std::shared_ptr<Chunk>> mActiveChunks;


	//The length, depth and height of a chunk
	static const int sChunkDimension = 8;
	//The number of cubes in a chunk
	static const int sChunkSize = sChunkDimension * sChunkDimension * sChunkDimension;
	//The unqiue ID for chunks
	static int sChunkMaxID;
	//The height of the world (in chunks)
	const int mMaxHeight = 5;
	//The length and depth of the world (in chunks)
	const int mMaxLength = 6;

	int mLoadedChunksAroundCurrentChunk = 1; //If 0, 1 chunk is loaded. if 1, 9 chunks are loaded, if 2, 25.
	int mChunksToLoad = -1;
	int mChunkRowsToLoad = -1;

	bool mCreatedWorld = false;
	//If a variable is +/- 1 then player is at the +/- edge of the map by that axis
	Pos mPlayerAtEdge; 
	Pos mWorldSizes;
	//Player variables

	Pos mPlayerPos;
	Pos mChangeInPlayerPos;

	std::vector<size_t> mAvailableActiveChunkIndexes;
	std::vector<size_t> mAvailableBlockStartIndexes;
	std::vector<size_t> mAvailableGObjStartIndexes;
	std::vector<size_t> mAvailableBlockInstanceStartIndexes;


	//DEBUG
	std::string mChunkOrder = "";
};