#pragma once

#include <algorithm>

#include "Object.h"
#include "PerlinNoise.h"

class WorldManager {
public:
	class Chunk {
	public:
		Chunk(DirectX::XMFLOAT3 pos);
		//~Chunk() = default;
		void Init(DirectX::XMFLOAT3 pos);

		Chunk& operator=(Chunk& c);

		bool GetAcitve() { return mActive; };
		DirectX::XMFLOAT3 GetPos() { return mPosition; };
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> GetBlocks() { return mBlocks; };
		std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> GetRItems() { return mRItems; };
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> GetActiveBlocks() { return mActiveBlocks; };

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
		DirectX::XMFLOAT3 mPosition; //Bottom left front corner. (-x, -y, -z)
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

	void LoadChunk(int x, int y, int z);
	void UnloadChunk(int x, int y, int z);
	//Chunk 1 is currently active, to be swapped with chunk 2
	void SwapChunk(int x1, int y1, int z1, int x2, int y2, int z2);

	int GetChunkSize() { return sChunkDimension; };
	int GetNumberOfChunksToLoad() { return mChunksToLoad; };

private:
	static void CreateCube(std::string materialName, XMFLOAT3 pos, std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> ris);
	std::shared_ptr<Chunk> GetChunk(int x, int y, int z);

private:

	static PerlinNoise sNoise;

	static std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> sGeometries;
	static std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Material>>> sMaterials;
	std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> mRitemLayer[(int)GameData::RenderLayer::Count];

	static const int sChunkDimension = 8;
	static const int sChunkSize = sChunkDimension * sChunkDimension;// *sChunkDimension;
	static int sChunkMaxID;

	const int mMaxHeight = 1;
	const int mMaxLength = 3;// 6;

	int mLoadedChunksAroundCurrentChunk = 0; //If 0, 1 chunk is loaded. if 1, 9 chunks are loaded, if 2, 25.
	int mChunksToLoad = 9;
	std::vector<std::shared_ptr<Chunk>> mChunks;
};