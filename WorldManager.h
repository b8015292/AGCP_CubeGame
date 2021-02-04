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
		Chunk(Pos pos);
		//~Chunk() = default;
		void Init(Pos pos);

		Chunk& operator=(Chunk& c);

		bool GetAcitve() { return mActive; };
		Pos GetPos() { return mPosition; };
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> GetBlocks() { return mBlocks; };
		std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> GetRItems() { return mRItems; };
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> GetActiveBlocks() { return mActiveBlocks; };
		int GetID() { return mID; };

		//Iterators
		size_t GetBlockStartIndex() { return mBlockStartIndex; };
		size_t GetGObjStartIndex() { return mGObjStartIndex; };
		size_t GetRIStartIndex() { return mRIStartIndex; };

		void SetStartIndexes(size_t block, size_t obj, size_t ri) {
			mBlockStartIndex = block; mGObjStartIndex = obj; mRIStartIndex = ri; }
		void SetBlockStartIndex(size_t b) { mBlockStartIndex = b; };
		void SetObjStartIndex(size_t b) { mGObjStartIndex = b; };
		void SetRIStartIndex(size_t b) { mRIStartIndex = b; };


		void SetAcitve(bool active) { mActive = active; };


	private:
		bool mActive = false;
		Pos mPosition; //Bottom left front corner. (-x, -y, -z)
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> mBlocks;
		std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> mRItems;
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> mActiveBlocks;

		int mID = -1;

		size_t mBlockStartIndex = -1;
		size_t mGObjStartIndex = -1;
		size_t mRIStartIndex = -1;
	};

	WorldManager();
	~WorldManager();

	void Init(std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> geos,
		std::shared_ptr < std::unordered_map<std::string, std::shared_ptr<Material>>> mats,
		std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> mRitemLayer[]);
	void CreateWorld();

	void UpdatePlayerPosition(DirectX::XMFLOAT3 pos);

	bool IsChunkCoordValid(int x, int y, int z);
	void LoadChunk(int x, int y, int z);
	void UnloadChunk(int x, int y, int z);
	//Chunk 1 is currently active, to be swapped with chunk 2
	void SwapChunk(Pos old, Pos neew);

	int GetChunkSize() { return sChunkSize; };
	int GetNumberOfChunksToLoad() { return mChunksToLoad; };

	int GetPlayerChunkIndex(DirectX::XMFLOAT3 pos);
	Pos GetPlayerChunkCoords(DirectX::XMFLOAT3 pos);
	std::shared_ptr<Chunk> GetPlayerChunk(DirectX::XMFLOAT3 pos);
	void LoadFirstChunks(float playerX, float playerZ);

	//Returns an index (in Block::sAllBlocks) used by a given block 
	std::shared_ptr<Block> GetBlock(DirectX::XMFLOAT3 pos);

	//DEBUG
	void PrintChunkOrder();

private:
	static void CreateCube(std::string materialName, XMFLOAT3 pos, bool active, std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> ris);
	//IsChunkCoordValid should be called before this.
	std::shared_ptr<Chunk> GetChunk(int x, int y, int z);
	UINT GetRenderItemCount();

private:
	static std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> sGeometries;
	static std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Material>>> sMaterials;
	std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> mRitemLayer[(int)GameData::RenderLayer::Count];

	static PerlinNoise sNoise;
	std::vector<std::shared_ptr<Chunk>> mChunks;

	//The length, depth and height of a chunk
	static const int sChunkDimension = 8;
	//The number of cubes in a chunk
	static const int sChunkSize = sChunkDimension * sChunkDimension * sChunkDimension;
	//The unqiue ID for chunks
	static int sChunkMaxID;
	//The height of the world (in chunks)
	const int mMaxHeight = 3;
	//The length and depth of the world (in chunks)
	const int mMaxLength = 6;

	int mLoadedChunksAroundCurrentChunk = 1; //If 0, 1 chunk is loaded. if 1, 9 chunks are loaded, if 2, 25.
	int mChunksToLoad;
	int mChunkRowsToLoad;

	bool mCreatedWorld = false;
	//If a variable is +/- 1 then player is at the +/- edge of the map by that axis
	Pos mPlayerAtEdge; 

	//Player variables

	Pos mPlayerPos;
	Pos mChangeInPlayerPos;


	//DEBUG
	std::string mChunkOrder = "";
};