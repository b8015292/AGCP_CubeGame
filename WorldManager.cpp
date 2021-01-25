#include "WorldManager.h"

PerlinNoise WorldManager::sNoise = PerlinNoise();
std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> WorldManager::sGeometries = nullptr;
std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Material>>> WorldManager::sMaterials = nullptr;
int WorldManager::sChunkMaxID = 0;

//************************************************************************
//						Chunk
//************************************************************************

WorldManager::Chunk::Chunk(DirectX::XMFLOAT3 pos) {
	mBlocks = std::make_shared<std::vector<std::shared_ptr<Block>>>();// (size_t)WorldManager::sChunkSize, std::make_shared<Block>());
	mActiveBlocks = std::make_shared<std::vector<std::shared_ptr<Block>>>();
	mRItems = std::make_shared<std::vector<std::shared_ptr<RenderItem>>>();
	mID = WorldManager::sChunkMaxID++;

	Init(pos);
}

void WorldManager::Chunk::Init(DirectX::XMFLOAT3 pos) {
	mPosition = pos;

	for (float worldZ = 0; worldZ < (float)WorldManager::sChunkDimension; ++worldZ)
	{
		for (float worldX = 0; worldX < (float)WorldManager::sChunkDimension; ++worldX)
		{

			//float noise = roundf(10.0f * (float)WorldManager::sNoise.noise((double)worldX / ((double)WorldManager::sChunkDimension),
			//		(double)worldZ / ((double)WorldManager::sChunkDimension), 0.8));
			float noise = roundf(10.0f * 
				(float)WorldManager::sNoise.noise((double)worldX + pos.x, (double)worldZ + pos.z, 0.8));

			for (float worldY = 0; worldY < (float)WorldManager::sChunkDimension; ++worldY) {
				if (worldY < noise) {
					CreateCube("mat_dirt",
						{ pos.x * (float)WorldManager::sChunkDimension + (float)worldX,
						pos.y * (float)WorldManager::sChunkDimension + (float)worldY,
						pos.z * (float)WorldManager::sChunkDimension + 1.0f * (float)worldZ }, true, mBlocks, mRItems);
				}
				else if(worldY == noise){
					CreateCube("mat_grass",
						{ pos.x * (float)WorldManager::sChunkDimension + (float)worldX,
						pos.y * (float)WorldManager::sChunkDimension + (float)worldY,
						pos.z * (float)WorldManager::sChunkDimension + 1.0f * (float)worldZ }, true, mBlocks, mRItems);
				}
				else {
					CreateCube("mat_grass",
						{ pos.x * (float)WorldManager::sChunkDimension + (float)worldX,
						pos.y * (float)WorldManager::sChunkDimension + (float)worldY,
						pos.z * (float)WorldManager::sChunkDimension + 1.0f * (float)worldZ }, false, mBlocks, mRItems);
				}
			}
		}
	}



	//for (float worldX = 0; worldX < (float)WorldManager::sChunkDimension; ++worldX)
	//{
	//	for (float worldZ = 0; worldZ < (float)WorldManager::sChunkDimension; ++worldZ)
	//	{
	//		float noise = -20.f 
	//			+ roundf(10.0f * (float)WorldManager::sNoise.noise((double)worldX / ((double)WorldManager::sChunkDimension), 
	//			(double)worldZ / ((double)WorldManager::sChunkDimension), 
	//			0.8));
	//		
	//		CreateCube("mat_grass",
	//			{ pos.x * (float)WorldManager::sChunkDimension + (float)worldX,
	//			pos.y * (float)WorldManager::sChunkDimension + noise,
	//			pos.z * (float)WorldManager::sChunkDimension + 1.0f * (float)worldZ }, mBlocks, mRItems);

	//	}
	//}
}

WorldManager::Chunk& WorldManager::Chunk::operator=(Chunk& c) {
	mActive = c.mActive;
	mID = c.mID;
	mBlocks = c.mBlocks;
	mActiveBlocks = c.mActiveBlocks;
	mRItems = c.mRItems;
	return (*this);
}


//************************************************************************
//						World Manager
//************************************************************************


WorldManager::WorldManager(){
	//Seed the noise
	srand(time_t(NULL));
	unsigned int seed = rand() % 10000;//237;
	sNoise = PerlinNoise(seed);
}

WorldManager::~WorldManager() {
	sMaterials.reset();
	sGeometries.reset();
	mChunks.clear();

	for (int i = 0; i < (int)GameData::RenderLayer::Count; i++){
		mRitemLayer[i].reset();
	}
}

void WorldManager::Init(std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> geos,
						std::shared_ptr < std::unordered_map<std::string, std::shared_ptr<Material>>> mats,
						std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> riLayers[]) {
	WorldManager::sGeometries = geos;
	WorldManager::sMaterials = mats;

	for (int i = 0; i < (int)GameData::RenderLayer::Count; i++)
		mRitemLayer[i] = riLayers[i];
}

void WorldManager::CreateWorld() {
	for (int i = 0; i < mMaxLength; i++) {
		for (int j = 0; j < mMaxHeight; j++) {
			for (int k = 0; k < mMaxLength; k++) {
				//mChunks.push_back(std::make_unique<Chunk>(DirectX::XMFLOAT3( (float)i, (float)j, (float)k )));
				mChunks.push_back(std::make_shared<Chunk>(DirectX::XMFLOAT3( (float)i, (float)j, (float)k )));

			}
		}
	}
}

void WorldManager::CreateCube(std::string materialName, XMFLOAT3 pos, bool active, std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> ris) {
	//Creates a render item, then uses it to create a Block. Then adds it to the needed lists
	ris->push_back(std::make_shared<RenderItem>(sGeometries->at("geo_shape").get(), "mesh_cube", sMaterials->at(materialName).get(), XMMatrixTranslation(pos.x, pos.y, pos.z)));

	//Create the block directly inside the block list
	blocks->push_back(std::make_shared<Block>( ris->at(ris->size() - 1)));

	blocks->at(blocks->size()- 1)->SetActive(active);
}

std::shared_ptr<WorldManager::Chunk> WorldManager::GetChunk(int x, int y, int z) {
	assert(x >= mMaxLength - 1 || y >= mMaxHeight - 1|| z >= mMaxLength - 1);
	return mChunks.at((size_t)x + (y * mMaxLength) + (z * mMaxHeight * mMaxLength));
}

void WorldManager::LoadChunk(int x, int y, int z) {
	std::shared_ptr<Chunk> chunk = GetChunk(x, y, z);

	if (chunk->GetAcitve())
		return;

	chunk->SetAcitve(true);
	chunk->SetStartIndexes(Block::sAllBlocks->size(), GameObject::sAllGObjs->size(), mRitemLayer[(int)GameData::RenderLayer::Main]->size());
	
	//Get the vectors to insert
	std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocksToInsert = chunk->GetBlocks();
	std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> ris = chunk->GetRItems();
	int count = GetRenderItemCount();

	//Insert the chunk
	Block::sAllBlocks->insert(Block::sAllBlocks->begin() + chunk->GetBlockStartIndex(), blocksToInsert->begin(), blocksToInsert->end());
	GameObject::sAllGObjs->insert(GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex(), blocksToInsert->begin(), blocksToInsert->end());
	mRitemLayer[(int)GameData::RenderLayer::Main]->insert(mRitemLayer[(int)GameData::RenderLayer::Main]->begin() + chunk->GetRIStartIndex(), ris->begin(), ris->end());

	//Set the render items object CB index so it can be found and updated by the GPU
	for (int i = chunk->GetRIStartIndex(); i < chunk->GetRIStartIndex() + sChunkSize; i++, count++) {
		mRitemLayer[(int)GameData::RenderLayer::Main]->at(i)->ObjCBIndex = count;
		mRitemLayer[(int)GameData::RenderLayer::Main]->at(i)->NumFramesDirty = gNumFrameResources;
	}
}

void WorldManager::UnloadChunk(int x, int y, int z) {
	std::shared_ptr<Chunk> chunk = GetChunk(x, y, z);
	
	if(!chunk->GetAcitve())
		return;

	chunk->SetAcitve(false);

	Block::sAllBlocks->erase(Block::sAllBlocks->begin() + chunk->GetBlockStartIndex(), Block::sAllBlocks->begin() + chunk->GetBlockStartIndex() + sChunkSize - 1 );
	GameObject::sAllGObjs->erase(GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex(), GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex() + sChunkSize - 1);
	mRitemLayer[(int)GameData::RenderLayer::Main]->erase(mRitemLayer[(int)GameData::RenderLayer::Main]->begin() + chunk->GetRIStartIndex(),
		mRitemLayer[(int)GameData::RenderLayer::Main]->begin() + chunk->GetRIStartIndex() + sChunkSize - 1);

	chunk->SetStartIndexes(-1, -1, -1);
}

void WorldManager::SwapChunk(int x1, int y1, int z1, int x2, int y2, int z2) {
	std::shared_ptr<Chunk> oldChunk = GetChunk(x1, y1, z1);
	std::shared_ptr<Chunk> newChunk = GetChunk(x2, y2, z2);

	if (!oldChunk->GetAcitve() || newChunk->GetAcitve())
		return;

	oldChunk->SetAcitve(false);
	newChunk->SetAcitve(true);
	int count = GetRenderItemCount();

	//Copy the chunk into the main lists, replacing the old chunk
	std::copy(newChunk->GetBlocks()->begin(), newChunk->GetBlocks()->end(), Block::sAllBlocks->begin() + oldChunk->GetBlockStartIndex());
	std::copy(newChunk->GetBlocks()->begin(), newChunk->GetBlocks()->end(), GameObject::sAllGObjs->begin() + oldChunk->GetGObjStartIndex());
	std::copy(newChunk->GetRItems()->begin(), 
		newChunk->GetRItems()->end(), 
		mRitemLayer[(int)GameData::RenderLayer::Main]->begin() + oldChunk->GetRIStartIndex());

	//Set the render items object CB index so it can be found and updated by the GPU
	for (int i = oldChunk->GetRIStartIndex(); i < oldChunk->GetRIStartIndex() + sChunkSize; i++, count++) {
		mRitemLayer[(int)GameData::RenderLayer::Main]->at(i)->ObjCBIndex = count;
		mRitemLayer[(int)GameData::RenderLayer::Main]->at(i)->NumFramesDirty = gNumFrameResources;
	}

	//Set the new iterators
	newChunk->SetStartIndexes(oldChunk->GetBlockStartIndex(), oldChunk->GetGObjStartIndex(), oldChunk->GetRIStartIndex());
	oldChunk->SetStartIndexes(-1, -1, -1);
}

int WorldManager::GetRenderItemCount() {
	int r = 0;
	for (int i = 0; i < (int)GameData::RenderLayer::Count; i++) {
		r += mRitemLayer[i]->size();
	}
	return r;
}

int WorldManager::GetPlayerChunk(DirectX::XMFLOAT3 pos) {
	int x = (int)floorf(pos.x / sChunkDimension);
	int y = (int)floorf(pos.y / sChunkDimension);
	int z = (int)floorf(pos.z / sChunkDimension);
	return z + (z * x);
}

void WorldManager::LoadFirstChunks() {
	LoadChunk(0, 0, 0);
	//LoadChunk(1, 0, 0);
	LoadChunk(2, 0, 0);
	LoadChunk(0, 0, 1);
}