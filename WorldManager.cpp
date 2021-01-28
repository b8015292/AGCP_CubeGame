#include "WorldManager.h"

PerlinNoise WorldManager::sNoise = PerlinNoise();
std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> WorldManager::sGeometries = nullptr;
std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Material>>> WorldManager::sMaterials = nullptr;
int WorldManager::sChunkMaxID = 0;

//************************************************************************
//						Chunk
//************************************************************************

WorldManager::Chunk::Chunk(Pos pos) {
	mBlocks = std::make_shared<std::vector<std::shared_ptr<Block>>>();// (size_t)WorldManager::sChunkSize, std::make_shared<Block>());
	mActiveBlocks = std::make_shared<std::vector<std::shared_ptr<Block>>>();
	mRItems = std::make_shared<std::vector<std::shared_ptr<RenderItem>>>();
	mID = WorldManager::sChunkMaxID++;

	Init(pos);
}

void WorldManager::Chunk::Init(Pos pos) {
	mPosition = pos;

	for (float worldZ = 0; worldZ < (float)WorldManager::sChunkDimension; ++worldZ)
	{
		for (float worldX = 0; worldX < (float)WorldManager::sChunkDimension; ++worldX)
		{

			//float noise = roundf(10.0f * (float)WorldManager::sNoise.noise((double)worldX / ((double)WorldManager::sChunkDimension),
			//		(double)worldZ / ((double)WorldManager::sChunkDimension), 0.8));
			float noise = roundf(10.0f * (float)WorldManager::sNoise.noise(((double)worldX + pos.x) / 10.f, (double)(worldZ + pos.z) / 10.f, 0.8));

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
				else {	//Blocks above the noise wave are not active upon creation
					CreateCube("mat_grass",
						{ pos.x * (float)WorldManager::sChunkDimension + (float)worldX,
						pos.y * (float)WorldManager::sChunkDimension + (float)worldY,
						pos.z * (float)WorldManager::sChunkDimension + 1.0f * (float)worldZ }, false, mBlocks, mRItems);
				}
			}
		}
	}
}

WorldManager::Chunk& WorldManager::Chunk::operator=(Chunk& c) {
	mActive = c.mActive;
	mID = c.mID;
	mBlocks = c.mBlocks;
	mActiveBlocks = c.mActiveBlocks;
	mRItems = c.mRItems;
	mPosition = c.mPosition;
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

	mChangeInPlayerPos = Pos();
	mChunkRowsToLoad = 1 + 2 * mLoadedChunksAroundCurrentChunk;
	mChunksToLoad = (int)sqrtf(mChunkRowsToLoad);			//Change this to chube root
}

void WorldManager::CreateWorld() {
	for (int z = 0; z < mMaxLength; z++) {
		for (int y = 0; y < mMaxHeight; y++) {
			for (int x = 0; x < mMaxLength; x++) {
				mChunks.push_back(std::make_shared<Chunk>(Pos(x, y, z)));
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
	assert(x >= mMaxLength - 1 || y >= mMaxHeight - 1|| z >= mMaxLength - 1
			|| x < 0 || y < 0 || z < 0);
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
	UINT count = GetRenderItemCount();

	//Insert the chunk
	Block::sAllBlocks->insert(Block::sAllBlocks->begin() + chunk->GetBlockStartIndex(), blocksToInsert->begin(), blocksToInsert->end());

	GameObject::sAllGObjs->insert(GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex(), blocksToInsert->begin(), blocksToInsert->end());
	mRitemLayer[(int)GameData::RenderLayer::Main]->insert(mRitemLayer[(int)GameData::RenderLayer::Main]->begin() + chunk->GetRIStartIndex(), ris->begin(), ris->end());

	//Set the render items object CB index so it can be found and updated by the GPU
	if (mCreatedWorld) {
		//Once the world has been loaded, get the CB index from the list of free indexes
		for (size_t i = chunk->GetRIStartIndex(); i < chunk->GetRIStartIndex() + sChunkSize; i++) {
			mRitemLayer[(int)GameData::RenderLayer::Main]->at(i)->ObjCBIndex = GameData::GetObjectCBIndex();
			mRitemLayer[(int)GameData::RenderLayer::Main]->at(i)->NumFramesDirty = GameData::sNumFrameResources;
		}
	}
	else {
		//If these are the first chunks being loaded, set their CB index to be ordered 0.... inifinate. So there are no gaps.
		for (size_t i = chunk->GetRIStartIndex(); i < chunk->GetRIStartIndex() + sChunkSize; i++, count++) {
			mRitemLayer[(int)GameData::RenderLayer::Main]->at(i)->ObjCBIndex = count;
		}
	}
}

void WorldManager::UnloadChunk(int x, int y, int z) {
	std::shared_ptr<Chunk> chunk = GetChunk(x, y, z);
	
	if(!chunk->GetAcitve())
		return;

	chunk->SetAcitve(false);

	for (std::vector<std::shared_ptr<RenderItem>>::iterator it = chunk->GetRItems()->begin(); it != chunk->GetRItems()->end(); it++) {
		GameData::AddNewObjectCBIndex((*it)->ObjCBIndex);
	}

	Block::sAllBlocks->erase(Block::sAllBlocks->begin() + chunk->GetBlockStartIndex(), Block::sAllBlocks->begin() + chunk->GetBlockStartIndex() + sChunkSize - 1 );
	GameObject::sAllGObjs->erase(GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex(), GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex() + sChunkSize - 1);
	mRitemLayer[(int)GameData::RenderLayer::Main]->erase(mRitemLayer[(int)GameData::RenderLayer::Main]->begin() + chunk->GetRIStartIndex(),
		mRitemLayer[(int)GameData::RenderLayer::Main]->begin() + chunk->GetRIStartIndex() + sChunkSize - 1);

	chunk->SetStartIndexes(-1, -1, -1);
}

void WorldManager::SwapChunk(int x1, int y1, int z1, int x2, int y2, int z2) {
	std::shared_ptr<Chunk> oldChunk = GetChunk(x1, y1, z1);
	std::shared_ptr<Chunk> newChunk = GetChunk(x2, y2, z2);

	std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> oldRIs = oldChunk->GetRItems();
	std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> newRIs = newChunk->GetRItems();


	if (!oldChunk->GetAcitve() || newChunk->GetAcitve())
		return;

	oldChunk->SetAcitve(false);
	newChunk->SetAcitve(true);

	//Copy the chunk into the main lists, replacing the old chunk
	std::copy(newChunk->GetBlocks()->begin(), newChunk->GetBlocks()->end(), Block::sAllBlocks->begin() + oldChunk->GetBlockStartIndex());
	std::copy(newChunk->GetBlocks()->begin(), newChunk->GetBlocks()->end(), GameObject::sAllGObjs->begin() + oldChunk->GetGObjStartIndex());
	std::copy(newRIs->begin(), newRIs->end(),  mRitemLayer[(int)GameData::RenderLayer::Main]->begin() + oldChunk->GetRIStartIndex());

	//Swap the chunks object CB indexes
	for (size_t i = 0; i < sChunkSize; i++) {
		newRIs->at(i)->ObjCBIndex = oldRIs->at(i)->ObjCBIndex;
		newRIs->at(i)->NumFramesDirty = GameData::sNumFrameResources;
	}

	//Set the new iterators
	newChunk->SetStartIndexes(oldChunk->GetBlockStartIndex(), oldChunk->GetGObjStartIndex(), oldChunk->GetRIStartIndex());
	oldChunk->SetStartIndexes(-1, -1, -1);
}

UINT WorldManager::GetRenderItemCount() {
	size_t r = 0;
	for (size_t i = 0; i < (size_t)GameData::RenderLayer::Count; i++) {
		r += mRitemLayer[i]->size();
	}
	return (UINT)r;
}

std::shared_ptr<Block> WorldManager::GetBlock(DirectX::XMFLOAT3 pos) {
	//Get the chunk
	int cx = (int)floorf(pos.x / sChunkDimension);
	int cy = (int)floorf(pos.y / sChunkDimension);
	int cz = (int)floorf(pos.z / sChunkDimension);
	std::shared_ptr<Chunk> c = mChunks.at(cx + (cz * mMaxLength));

	//Find the block coords in terms of its chunk
	int bx = (int)floorf(pos.x) - cx * sChunkDimension;
	int by = (int)floorf(pos.y);// -cy * sChunkDimension;	//There is only one Y chunk!
	int bz = (int)floorf(pos.z) - cz * sChunkDimension;

	return c->GetBlocks()->at((size_t)by + (bx * sChunkDimension) + (bz * sChunkDimension * sChunkDimension) - 1);
}

int WorldManager::GetPlayerChunkIndex(DirectX::XMFLOAT3 pos) {
	int x = (int)floorf(pos.x / sChunkDimension);
	int y = (int)floorf(pos.y / sChunkDimension);
	int z = (int)floorf(pos.z / sChunkDimension);
	return x + (z * mMaxLength);
}

std::shared_ptr<WorldManager::Chunk> WorldManager::GetPlayerChunk(DirectX::XMFLOAT3 pos){
	int x = (int)floorf(pos.x / sChunkDimension);
	int y = (int)floorf(pos.y / sChunkDimension);
	int z = (int)floorf(pos.z / sChunkDimension);
	return GetChunk(x, 0, z);
}

void WorldManager::LoadFirstChunks(float playerX, float playerZ) {

	mPlayerPos = GetPlayerChunk({ playerX, 0, playerZ})->GetPos();
	Pos start(mPlayerPos.x - mLoadedChunksAroundCurrentChunk, 0, mPlayerPos.z - mLoadedChunksAroundCurrentChunk);
	for (int i = 0; i < mChunkRowsToLoad; i++) {
		//for (int j = 0; j < mChunkRowsToLoad; j++) {
			for (int k = 0; k < mChunkRowsToLoad; k++) {
				LoadChunk(k, 0, i);
			}
		//}
	}

	mCreatedWorld = true;
}

void WorldManager::UpdatePlayerPosition(DirectX::XMFLOAT3 pos) {
	Pos newPos = GetPlayerChunk(pos)->GetPos();
	if (mPlayerPos != newPos) {
		//Calculate the change
		mChangeInPlayerPos.x = newPos.x - mPlayerPos.x;
		mChangeInPlayerPos.y = newPos.y - mPlayerPos.y;
		mChangeInPlayerPos.z = newPos.z - mPlayerPos.z;


		//Load new chunks and unload old chunks
		//Change in X
		if (mChangeInPlayerPos.x != 0) {
			int oldX = (mLoadedChunksAroundCurrentChunk * -mChangeInPlayerPos.x) + mPlayerPos.x;
			int newX = (mLoadedChunksAroundCurrentChunk * mChangeInPlayerPos.x) + newPos.x;
			if (newX < 0 || newX >= mMaxLength)
				return;
			for (int z = -mLoadedChunksAroundCurrentChunk; z <= mLoadedChunksAroundCurrentChunk; z++) {
				int newZ = mPlayerPos.z + z;
				SwapChunk(oldX, 0, newZ, newX, 0, newZ);
			}
		}
		//Change in Z
		if (mChangeInPlayerPos.z != 0) {
			int oldZ = (mLoadedChunksAroundCurrentChunk * -mChangeInPlayerPos.z) + mPlayerPos.z;
			int newZ = (mLoadedChunksAroundCurrentChunk * mChangeInPlayerPos.z) + newPos.z;
			if (newZ < 0 || newZ >= mMaxLength)
				return;
			for (int x = -mLoadedChunksAroundCurrentChunk; x <= mLoadedChunksAroundCurrentChunk; x++) {
				int newX = mPlayerPos.x + x;
				SwapChunk(newX, 0, oldZ, newX, 0, newZ);
			}
		}

		mPlayerPos = newPos;
	}
}