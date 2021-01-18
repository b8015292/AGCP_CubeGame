#pragma once

#include "Object.h"
#include "PerlinNoise.h"

class WorldManager {
public:
	class Chunk {
	public:
		Chunk(DirectX::XMFLOAT3 pos);
		//~Chunk() = default;
		void Init(DirectX::XMFLOAT3 pos);

		bool GetAcitve() { return mActive; };
		DirectX::XMFLOAT3 GetPos() { return mPosition; };
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> GetBlocks() { return mBlocks; };
		std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> GetRItems() { return mRItems; };
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> GetActiveBlocks() { return mActiveBlocks; };

		void SetAcitve(bool active) { mActive = active; };


	private:
		bool mActive = false;
		DirectX::XMFLOAT3 mPosition; //Bottom left front corner. (-x, -y, -z)
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> mBlocks;
		std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> mRItems;
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> mActiveBlocks;

		int mID = -1;
	};

	WorldManager();
	~WorldManager();

	void Init(std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> geos,
		std::shared_ptr < std::unordered_map<std::string, std::shared_ptr<Material>>> mats,
		std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> sAllGObjs,
		std::shared_ptr<std::vector<std::shared_ptr<Block>>> mAllBlocks,
		std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> mRitemLayer[]);
	void CreateWorld();

	void LoadChunk();

	int GetChunkSize() { return sChunkDimension; };

private:
	static void CreateCube(std::string materialName, XMFLOAT3 pos, std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> ris);
	Chunk& GetChunk(int x, int y, int z);

private:

	static PerlinNoise sNoise;

	static std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> sGeometries;
	static std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Material>>> sMaterials;
	static std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> sAllGObjs;

	std::shared_ptr<std::vector<std::shared_ptr<Block>>> mAllBlocks;
	std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> mRitemLayer[(int)GameData::RenderLayer::Count];

	static const int sChunkDimension = 8;
	static const int sChunkSize = sChunkDimension * sChunkDimension;// *sChunkDimension;
	static int sChunkMaxID;

	const int mMaxHeight = 1;
	const int mMaxLength = 1;// 6;

	int mLoadedChunksAroundCurrentChunk = 1; //If 0, 1 chunk is loaded. if 1, 9 chunks are loaded, if 2, 25.
	//int mMaxChunks = 1 + 
	std::vector<Chunk> mChunks;
};