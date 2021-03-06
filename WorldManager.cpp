#include "WorldManager.h"

#include <ctime>

PerlinNoise WorldManager::sNoise = PerlinNoise();
std::shared_ptr<std::unordered_map<std::string, int>> WorldManager::sMaterialIndexes = nullptr;
int WorldManager::sChunkMaxID = 0;
std::vector<WorldManager::Pos> WorldManager::sTreeStartPositions;

//************************************************************************
//						Chunk
//************************************************************************

WorldManager::Chunk::Chunk(Pos pos) {
	mBlocks = std::make_shared<std::vector<std::shared_ptr<Block>>>();
	mActiveBlocks = std::make_shared<std::vector<std::shared_ptr<Block>>>();
	mInstanceDatas = std::make_shared<std::vector<std::shared_ptr<InstanceData>>>();
	mID = WorldManager::sChunkMaxID++;

	Init(pos);
}

void WorldManager::Chunk::Init(Pos pos) {
	mPosition = pos;

	int treeLocationHeight = -1;

	for (float worldZ = 0; worldZ < (float)WorldManager::sChunkDimension; ++worldZ)
	{
		for (float worldX = 0; worldX < (float)WorldManager::sChunkDimension; ++worldX)
		{
			int minTerrainHeight = 0;
			float noise = minTerrainHeight + roundf(10.0f * (float)WorldManager::sNoise.OctavePerlin(((double)worldX + (pos.x * (double)WorldManager::sChunkDimension)) * 0.002f, (double)(worldZ + (pos.z * (double)WorldManager::sChunkDimension)) * 0.002f, 0.8, 6, 20));


			for (float worldY = 0; worldY < (float)WorldManager::sChunkDimension; ++worldY) {
				//Generate different noises for coal and iron, so they are grouped together
				float coalOreNoise = (float)WorldManager::sNoise.OctavePerlin(((double)worldX + (pos.x * (double)WorldManager::sChunkDimension)) * 0.002f, (double)(worldY + (pos.y * (double)WorldManager::sChunkDimension)) * 0.002f, (double)(worldZ + (pos.z * (double)WorldManager::sChunkDimension)) * 0.002f, 8, 20);
				float ironOreNoise = (float)WorldManager::sNoise.OctavePerlin(((double)worldX + (pos.x * (double)WorldManager::sChunkDimension)) * 0.002f, (double)(worldY + (pos.y * (double)WorldManager::sChunkDimension)) * 0.002f, (double)(worldZ + (pos.z * (double)WorldManager::sChunkDimension)) * 0.002f, 15, 20);

				float x = pos.x * (float)WorldManager::sChunkDimension + (float)worldX;
				float y = pos.y * (float)WorldManager::sChunkDimension + (float)worldY;
				float z = pos.z * (float)WorldManager::sChunkDimension + (float)worldZ;

				//By checking the y values of the world, we can determine what sort of block it should be
				if (worldY + pos.y * WorldManager::sChunkDimension < noise) {
					if (worldY + pos.y * WorldManager::sChunkDimension > 0){
						if (worldY + pos.y * WorldManager::sChunkDimension < noise - 3) {
							if (coalOreNoise > 0.69 && coalOreNoise < 0.75) {
								CreateCube("mat_coal_ore", { x, y, z }, true, mBlocks, mInstanceDatas);
							}
							else if (ironOreNoise < 0.25) {
								CreateCube("mat_iron_ore", { x, y, z }, true, mBlocks, mInstanceDatas);
							}
							else {
								CreateCube("mat_stone", { x, y, z }, true, mBlocks, mInstanceDatas);
							}
						}
						else {
							CreateCube("mat_dirt", { x, y, z }, true, mBlocks, mInstanceDatas);
						}
					}
					else {
						CreateCube("mat_bedrock", { x, y, z }, true, mBlocks, mInstanceDatas);
					}
				}
				else if(worldY + pos.y * WorldManager::sChunkDimension == noise){
					CreateCube("mat_grass", { x, y, z }, true, mBlocks, mInstanceDatas);
					//As this is the highest block at this x and z coordinate, then a tree can spawn here
					if (worldX == WorldManager::sChunkDimension / 2 && worldZ == WorldManager::sChunkDimension / 2) {
						treeLocationHeight = y + 1;
					}
				}
				else {
					CreateCube("mat_grass", { x, y, z }, false, mBlocks, mInstanceDatas);
				}
			}
		}
		//As we have seeded the rand function already, this will return the same value each time that seed is used.
		int trees = rand() % 100;

		//If the treeLocationHeight is 0 or less then this chunk has no grass in it
		if (treeLocationHeight >= 0 && trees > 90) {
			sTreeStartPositions.push_back({ pos.x * WorldManager::sChunkDimension + (WorldManager::sChunkDimension / 2), treeLocationHeight, pos.z * WorldManager::sChunkDimension + (WorldManager::sChunkDimension / 2) });
		}

	}
}

WorldManager::Chunk& WorldManager::Chunk::operator=(Chunk& c) {
	mActive = c.mActive;
	mID = c.mID;
	mBlocks = c.mBlocks;
	mActiveBlocks = c.mActiveBlocks;
	mPosition = c.mPosition;
	mInstanceDatas = c.mInstanceDatas;
	return (*this);
}


std::shared_ptr<Block> WorldManager::Chunk::GetBlock(Pos pos) {
	return GetBlocks()->at((size_t)(pos.y + (pos.x * sChunkDimension) + (pos.z * sChunkDimension * sChunkDimension)));
}


//************************************************************************
//						World Manager
//************************************************************************


WorldManager::WorldManager(){
	//Seed the noise
	srand(std::time(NULL));
	unsigned int seed = rand() % 10000;//237;
	sNoise = PerlinNoise(seed);
	srand(sNoise.GetSeed());

	//std::srand(std::time(nullptr));
	//noise = PerlinNoise(std::rand());
}

WorldManager::~WorldManager() {
	sMaterialIndexes.reset();
	mChunks.clear();
}

void WorldManager::Init(std::shared_ptr<std::unordered_map<std::string, int>> mats) {
	WorldManager::sMaterialIndexes = mats;

	mChangeInPlayerPos = Pos();
	mWorldSizes.x = mMaxLength;
	mWorldSizes.y = mMaxHeight;
	mWorldSizes.z = mMaxLength;
}

void WorldManager::CreateWorld() {
	const size_t threads = 1;//6;
	std::vector<std::shared_ptr<Chunk>> lists[threads];

//#pragma omp parallel for num_threads(threads)
	for (int z = 0; z < mMaxLength; z++) {
		//int thread = omp_get_thread_num();
		for (int y = 0; y < mMaxHeight; y++) {
			for (int x = 0; x < mMaxLength; x++) {
				//lists[thread].push_back(std::make_shared<Chunk>(Pos(x, y, z)));
				lists[0].push_back(std::make_shared<Chunk>(Pos(x, y, z)));
			}
		}
	}

	for (size_t i = 0; i < threads; i++) {
		mChunks.insert(mChunks.end(), lists[i].begin(), lists[i].end());
	}



	PopulateMapWithTrees();
}

void WorldManager::PopulateMapWithTrees() {
	const int trunkHeight = 5;
	const int foliageWidth = 2;
	const int foliageHeight = 1;
	const int totalHeight = trunkHeight + foliageHeight;

	//Create all the trees
	for each (Pos p in sTreeStartPositions) {
		std::shared_ptr<Chunk> mainChunk = GetChunkFromWorldCoords({ (float)p.x, (float)p.y, (float)p.z });
		std::shared_ptr<Chunk> aboveChunk;

		Pos stumpCoords = GetChunkCoordsFromWorldCoods(p);
		Pos currCoords = stumpCoords;

		//If the tree is in part of the chunk above it, then we need a reference to this chunk
		if ((p.y + totalHeight) >= sChunkDimension) {
			aboveChunk = GetChunkFromWorldCoords({ (float)p.x, (float)(p.y + totalHeight), (float)p.z });
		}
		
		bool above = false;
		std::shared_ptr<Chunk> chunkToUse = mainChunk;

		//Create the trunk
		for (int i = 0; i < trunkHeight; i++) {

			//If the trunk goes into the other chunk, we need to change the chunk we are modifying so we are changing the right blocks
			if (!above && p.y + i >= sChunkDimension) {
				chunkToUse = aboveChunk;
				currCoords.y = 0; 
				above = true;
			}

			SetBlockType(chunkToUse->GetBlock(currCoords), "mat_oak_log", true);
			currCoords.y ++;
		}

		//From here, we loop through coordinates to create the pattern of leaves for the trees
		//Each time we modify the y coordinate, we must check if we have changed chunk once again.
		currCoords.y = stumpCoords.y + trunkHeight - 3;
		if (currCoords.y >= sChunkDimension) {
			chunkToUse = aboveChunk;
			currCoords.y = 0;
		}
		else {
			//As the first set of leaves are below the height of the trunk, we might need to swap back to the original chunk
			chunkToUse = mainChunk;
		}

		for (currCoords.x = stumpCoords.x - foliageWidth; currCoords.x <= stumpCoords.x + foliageWidth; currCoords.x++) {
			for (currCoords.z = stumpCoords.z - foliageWidth; currCoords.z <= stumpCoords.z + foliageWidth; currCoords.z++) {
				if (!(currCoords.x == stumpCoords.x && currCoords.z == stumpCoords.z)) {
					SetBlockType(chunkToUse->GetBlock(currCoords), "mat_oak_leaf", true);
				}
			}
		}
		currCoords.y++;
		if (currCoords.y >= sChunkDimension) {
			chunkToUse = aboveChunk;
			currCoords.y = 0;
		}
		for (currCoords.x = stumpCoords.x - foliageWidth; currCoords.x <= stumpCoords.x + foliageWidth; currCoords.x++) {
			for (currCoords.z = stumpCoords.z - foliageWidth; currCoords.z <= stumpCoords.z + foliageWidth; currCoords.z++) {
				if (!(currCoords.x == stumpCoords.x && currCoords.z == stumpCoords.z)) {
					SetBlockType(chunkToUse->GetBlock(currCoords), "mat_oak_leaf", true);
				}
			}
		}
		currCoords.y++;
		if (currCoords.y >= sChunkDimension) {
			chunkToUse = aboveChunk;
			currCoords.y = 0;
		}
		for (currCoords.x = stumpCoords.x - foliageWidth / 2; currCoords.x <= stumpCoords.x + foliageWidth / 2; currCoords.x++) {
			for (currCoords.z = stumpCoords.z - foliageWidth / 2; currCoords.z <= stumpCoords.z + foliageWidth / 2; currCoords.z++) {
				if (!(currCoords.x == stumpCoords.x && currCoords.z == stumpCoords.z)) {
					SetBlockType(chunkToUse->GetBlock(currCoords), "mat_oak_leaf", true);
				}
			}
		}

		currCoords.y++;
		if (currCoords.y >= sChunkDimension) {
			chunkToUse = aboveChunk;
			currCoords.y = 0;
		}
		currCoords.x = stumpCoords.x + 1;
		currCoords.z = stumpCoords.z;
		SetBlockType(chunkToUse->GetBlock(currCoords), "mat_oak_leaf", true);
		currCoords.x = stumpCoords.x;
		currCoords.z = stumpCoords.z + 1;
		SetBlockType(chunkToUse->GetBlock(currCoords), "mat_oak_leaf", true);
		currCoords.x = stumpCoords.x - 1;
		currCoords.z = stumpCoords.z;
		SetBlockType(chunkToUse->GetBlock(currCoords), "mat_oak_leaf", true);
		currCoords.x = stumpCoords.x;
		currCoords.z = stumpCoords.z - 1;
		SetBlockType(chunkToUse->GetBlock(currCoords), "mat_oak_leaf", true);
		currCoords.x = stumpCoords.x;
		currCoords.z = stumpCoords.z;
		SetBlockType(chunkToUse->GetBlock(currCoords), "mat_oak_leaf", true);
	}
}

WorldManager::Pos WorldManager::GetChunkCoordsFromWorldCoods(WorldManager::Pos p) {
	p.x -= (int)floorf(p.x / sChunkDimension) * sChunkDimension;
	p.y -= (int)floorf(p.y / sChunkDimension) * sChunkDimension;
	p.z -= (int)floorf(p.z / sChunkDimension) * sChunkDimension;
	return p;
}

void WorldManager::CreateCube(std::string materialName, XMFLOAT3 pos, bool active, std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, std::shared_ptr<std::vector<std::shared_ptr<InstanceData>>> blockInstances) {
	//Creates a render item, then uses it to create a Block. Then adds it to the needed lists
	auto idata = std::make_shared<InstanceData>(XMMatrixTranslation(pos.x, pos.y, pos.z), sMaterialIndexes->at(materialName), materialName, Block::sBlockInstance->mBoundingBox);
	blockInstances->push_back(idata);
	
	//Create the block directly inside the block list
	blocks->push_back(std::make_shared<Block>(idata));
	blocks->at(blocks->size()- 1)->SetActive(active);
}

void WorldManager::SetBlockType(std::shared_ptr<Block> block, std::string materialName, bool active) {
	std::shared_ptr<InstanceData> id = block->GetInstanceData();
	id->MaterialName = materialName;
	id->MaterialIndex = sMaterialIndexes->at(materialName);
	block->SetActive(active);
}

std::shared_ptr<WorldManager::Chunk> WorldManager::GetChunk(int x, int y, int z) {
	return mChunks.at((size_t)x + (y * mMaxLength) + (z * mMaxHeight * mMaxLength));
	//return mChunks.at((size_t)x + (z * mMaxHeight * mMaxLength));
}

void WorldManager::SetChunkActive(std::shared_ptr<Chunk> chunk, bool active) {
	if (active) {
		//If there are no available indexes, add it to the end.
		if (mAvailableActiveChunkIndexes.size() == 0) {
			chunk->SetActiveIndex(mActiveChunks.size());
			mActiveChunks.push_back(chunk);
		}
		else {
			//If there are available indexes, swap the deactivated chunk with this one.
			size_t newIndex = mAvailableActiveChunkIndexes.at(mAvailableActiveChunkIndexes.size() - 1);
			mAvailableActiveChunkIndexes.pop_back();
			chunk->SetActiveIndex(newIndex);
			mActiveChunks.at(newIndex) = chunk;
			
		}
	}
	else {
		//Deactivate the chunk and add its indexs to the available list
		mAvailableActiveChunkIndexes.push_back(chunk->GetActiveIndex());
	}

	chunk->SetAcitve(active);
}
bool WorldManager::IsChunkCoordValid(int x, int y, int z) {
	if (x >= mMaxLength || y >= mMaxHeight || z >= mMaxLength
		|| x < 0 || y < 0 || z < 0)
		return false;

	return true;
}

void WorldManager::LoadChunk(int x, int y, int z) {
	//Check the coord is valid, if so get the chunk and check it isn't already active (loaded)
	if (!IsChunkCoordValid(x, y, z))return;

	std::shared_ptr<Chunk> chunk = GetChunk(x, y, z);
	if (chunk->GetAcitve())return;

	//Set the chunk to active and set its starting indexes
	SetChunkActive(chunk, true);
	if(!mCreatedWorld)
		chunk->SetStartIndexes(Block::sAllBlocks->size(), GameObject::sAllGObjs->size(), Block::sBlockInstance->Instances.size());

	//Get the vectors to insert
	std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocksToInsert = chunk->GetBlocks();
	std::shared_ptr<std::vector<std::shared_ptr<InstanceData>>> instances = chunk->GetInstanceDatas();

	//Get the start indexes
	size_t blockStart;
	size_t instanceStart;
	size_t gObjStart;

	if (mCreatedWorld) {
		//If the world exists, replace deactivated elements
		blockStart = mAvailableBlockStartIndexes.at(mAvailableBlockStartIndexes.size() - 1);
		instanceStart = mAvailableBlockInstanceStartIndexes.at(mAvailableBlockInstanceStartIndexes.size() - 1);
		gObjStart = mAvailableGObjStartIndexes.at(mAvailableGObjStartIndexes.size() - 1);

		mAvailableBlockStartIndexes.pop_back();
		mAvailableBlockInstanceStartIndexes.pop_back();
		mAvailableGObjStartIndexes.pop_back();

		//Delete the old values
		Block::sAllBlocks->erase(Block::sAllBlocks->begin() + blockStart, Block::sAllBlocks->begin() + blockStart + sChunkSize);
		GameObject::sAllGObjs->erase(GameObject::sAllGObjs->begin() + gObjStart, GameObject::sAllGObjs->begin() + gObjStart + sChunkSize);
		Block::sBlockInstance->Instances.erase(Block::sBlockInstance->Instances.begin() + instanceStart, Block::sBlockInstance->Instances.begin() + instanceStart + sChunkSize);
		chunk->SetStartIndexes(blockStart, gObjStart, instanceStart);

		//Insert the new values
		Block::sAllBlocks->insert(Block::sAllBlocks->begin() + blockStart, blocksToInsert->begin(), blocksToInsert->end());
		GameObject::sAllGObjs->insert(GameObject::sAllGObjs->begin() + gObjStart, blocksToInsert->begin(), blocksToInsert->end());
		Block::sBlockInstance->Instances.insert(Block::sBlockInstance->Instances.begin() + instanceStart, instances->begin(), instances->end());
	}
	else {
		//If the world doesn't exist, insert them
		blockStart = chunk->GetBlockStartIndex();
		instanceStart = chunk->GetInstanceStartIndex();
		gObjStart = chunk->GetGObjStartIndex();

		//Insert the chunk into the lists
		Block::sAllBlocks->insert(Block::sAllBlocks->begin() + blockStart, blocksToInsert->begin(), blocksToInsert->end());
		GameObject::sAllGObjs->insert(GameObject::sAllGObjs->begin() + gObjStart, blocksToInsert->begin(), blocksToInsert->end());
		Block::sBlockInstance->Instances.insert(Block::sBlockInstance->Instances.begin() + instanceStart, instances->begin(), instances->end());
	}

	//Set the render items object CB index so it can be found and updated by the GPU
	if (mCreatedWorld) {
		//Once the world has been loaded, get the CB index from the list of free indexes
		for (size_t i = instanceStart; i < instanceStart + sChunkSize; i++) {
			//Block::sBlockInstance->Instances.at(i)->BufferIndex = GameData::GetObjectCBIndex();
			Block::sBlockInstance->Instances.at(i)->NumFramesDirty = GameData::sNumFrameResources;
		}
	}
	//else {
	//	//If these are the first chunks being loaded, set their CB index to be ordered 0.... inifinate. So there are no gaps.
	//	for (size_t i = instanceStart; i < instanceStart + sChunkSize; i++, count++) {
	//		Block::sBlockInstance->Instances.at(i)->BufferIndex = count;
	//	}
	//}
}

void WorldManager::UnloadChunk(int x, int y, int z) {
	//Get the chunk and check it isn't already deactivated (un-loaded)
	std::shared_ptr<Chunk> chunk = GetChunk(x, y, z);
	if(!chunk->GetAcitve()) return;
		
	//Deactivate the chunk
	SetChunkActive(chunk, false);

	Block::sAllBlocks->erase(Block::sAllBlocks->begin() + chunk->GetBlockStartIndex(), Block::sAllBlocks->begin() + chunk->GetBlockStartIndex() + sChunkSize );
	GameObject::sAllGObjs->erase(GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex(), GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex() + sChunkSize);
	Block::sBlockInstance->Instances.erase(Block::sBlockInstance->Instances.begin() + chunk->GetInstanceStartIndex(),
		Block::sBlockInstance->Instances.begin() + chunk->GetInstanceStartIndex() + sChunkSize);

	chunk->SetStartIndexes(-1, -1, -1);
	
}

void WorldManager::SwapChunk(Pos old, Pos neew) {
	if (!IsChunkCoordValid(old.x, old.y, old.z) || !IsChunkCoordValid(neew.x, neew.y, neew.z))
 		return;

	//Get the chunks
	std::shared_ptr<Chunk> oldChunk = GetChunk(old.x, old.y, old.z);
	std::shared_ptr<Chunk> newChunk = GetChunk(neew.x, neew.y, neew.z);

	//If the new chunk is already active, exit
	if (newChunk->GetAcitve())
		return;

	//If the previous chunk wasnt loaded, then load it instead of swapping
	if (!oldChunk->GetAcitve()) {
   		LoadChunk(neew.x, neew.y, neew.z);
		return;
	}

	SetChunkActive(oldChunk, false);
	SetChunkActive(newChunk, true);

	//Get the block instance datas to swap
	std::shared_ptr<std::vector<std::shared_ptr<InstanceData>>> oldInstances = oldChunk->GetInstanceDatas();
	std::shared_ptr<std::vector<std::shared_ptr<InstanceData>>> newInstances = newChunk->GetInstanceDatas();

	//Copy the chunk into the main lists, replacing the old chunk
	std::copy(newChunk->GetBlocks()->begin(), newChunk->GetBlocks()->end(), Block::sAllBlocks->begin() + oldChunk->GetBlockStartIndex());
	std::copy(newChunk->GetBlocks()->begin(), newChunk->GetBlocks()->end(), GameObject::sAllGObjs->begin() + oldChunk->GetGObjStartIndex());
	std::copy(newInstances->begin(), newInstances->end(), Block::sBlockInstance->Instances.begin() + oldChunk->GetInstanceStartIndex());

	////Swap the chunks object CB indexes
	//for (size_t i = 0; i < sChunkSize; i++) {
	//	newInstances->at(i)->BufferIndex = oldInstances->at(i)->BufferIndex;
	//	newInstances->at(i)->NumFramesDirty = GameData::sNumFrameResources;
	//}

	//Set the new iterators
	newChunk->SetStartIndexes(oldChunk->GetBlockStartIndex(), oldChunk->GetGObjStartIndex(), oldChunk->GetInstanceStartIndex());
	oldChunk->SetStartIndexes(-1, -1, -1);
}

void WorldManager::UnloadAllChunks() {
	//for each (std::shared_ptr<Chunk> chunk in mActiveChunks) {
	for (int i = mActiveChunks.size() - 1; i >= 0; i--) {
		std::shared_ptr<Chunk> chunk = mActiveChunks.at(i);

		mAvailableBlockStartIndexes.push_back(chunk->GetBlockStartIndex());
		mAvailableBlockInstanceStartIndexes.push_back(chunk->GetInstanceStartIndex());
		mAvailableGObjStartIndexes.push_back(chunk->GetGObjStartIndex());

		SetChunkActive(chunk, false);
	}
}

std::shared_ptr<Block> WorldManager::GetBlock(DirectX::XMFLOAT3 pos) {
	//Get the chunk
	int cx = (int)floorf(pos.x / sChunkDimension);
	int cy = (int)floorf(pos.y / sChunkDimension);
	int cz = (int)floorf(pos.z / sChunkDimension);
	std::shared_ptr<Chunk> c = mChunks.at(size_t(cx + (cy * mMaxLength) + (cz * mMaxLength * mMaxHeight)));

	//Find the block coords in terms of its chunk
	int bx = (int)floorf(pos.x) - cx * sChunkDimension;
	int by = (int)floorf(pos.y)	- cy * sChunkDimension;
	int bz = (int)floorf(pos.z) - cz * sChunkDimension;

	return c->GetBlocks()->at((size_t)(by + (bx * sChunkDimension) + (bz * sChunkDimension * sChunkDimension) - 1));
}

int WorldManager::GetPlayerChunkIndex(DirectX::XMFLOAT3 pos) {
	int x = (int)floorf(pos.x / sChunkDimension);
	int y = (int)floorf(pos.y / sChunkDimension);
	int z = (int)floorf(pos.z / sChunkDimension);
	return x + (z * mMaxLength);
}

std::shared_ptr<WorldManager::Chunk> WorldManager::GetChunkFromWorldCoords(DirectX::XMFLOAT3 pos){
	int x = (int)floorf(pos.x / sChunkDimension);
	int y = (int)floorf(pos.y / sChunkDimension);
	int z = (int)floorf(pos.z / sChunkDimension);

	if (!IsChunkCoordValid(x, y, z))
		return GetChunk(0, 0, 0);

	return GetChunk(x, y, z);
}

WorldManager::Pos WorldManager::GetPlayerChunkCoords(DirectX::XMFLOAT3 pos) {
	return Pos((int)floorf(pos.x / sChunkDimension), (int)floorf(pos.y / sChunkDimension), (int)floorf(pos.z / sChunkDimension));
}

void WorldManager::LoadFirstChunks(DirectX::XMFLOAT3 pos) {

	mPlayerPos = GetChunkFromWorldCoords(pos)->GetPos();

	Pos start(mPlayerPos.x - mLoadedChunksAroundCurrentChunk, mPlayerPos.y - mLoadedChunksAroundCurrentChunk, mPlayerPos.z - mLoadedChunksAroundCurrentChunk);
	for (int i = 0; i < mChunkRowsToLoad; i++) {
		for (int j = 0; j < mChunkRowsToLoad; j++) {
			for (int k = 0; k < mChunkRowsToLoad; k++) {
				LoadChunk(start.x + k, start.y + j, start.z + i);
			}
		}
	}

	mCreatedWorld = true;
}

void WorldManager::RelocatePlayer(DirectX::XMFLOAT3 newPos) {
	UnloadAllChunks();
	LoadFirstChunks(newPos);

	Pos playerChunkPos = GetPlayerChunkCoords(newPos);
	mPlayerPos = GetChunk(playerChunkPos.x, playerChunkPos.y, playerChunkPos.z)->GetPos();
	mPlayerAtEdge = Pos();
}

void WorldManager::UpdatePlayerPosition(DirectX::XMFLOAT3 worldPos) {
	//Check if the playeys current chunk is valid
	Pos playerChunkPos = GetPlayerChunkCoords(worldPos);
	if (!IsChunkCoordValid(playerChunkPos.x, playerChunkPos.y, playerChunkPos.z))
		return;

	//Check the new chunk isn't the same as the previous chunk
	Pos newPos = GetChunk(playerChunkPos.x, playerChunkPos.y, playerChunkPos.z)->GetPos();
	if (mPlayerPos != newPos) {

		//Calculate the change
		mChangeInPlayerPos.x = newPos.x - mPlayerPos.x;
		mChangeInPlayerPos.y = newPos.y - mPlayerPos.y;
		mChangeInPlayerPos.z = newPos.z - mPlayerPos.z;

		//Loop each axis, but only check it if it has changed
		for (int changeAxis = 0; changeAxis < 3; changeAxis++) {
			if (mChangeInPlayerPos[changeAxis] != 0) {

				bool run = true;

				//Get the position the player has just left
				int oldAxis = (mLoadedChunksAroundCurrentChunk * -mChangeInPlayerPos[changeAxis]) + mPlayerPos[changeAxis];
				//Get the position the player is about to enter
				int newAxis = (mLoadedChunksAroundCurrentChunk * mChangeInPlayerPos[changeAxis]) + newPos[changeAxis];

				//If the new position is outside of the world, dont swap any chunks
				if (newAxis < mLoadedChunksAroundCurrentChunk) {
					mPlayerAtEdge[changeAxis] = 1;
					mPlayerPos = newPos;
					run = false;

				}
				else if (newAxis >= mWorldSizes[changeAxis]) {
					mPlayerAtEdge[changeAxis] = -1;
					mPlayerPos = newPos;
					run = false;
				}
				else {
					//If the player was previously at the edge, dont swap chunks
					if (mPlayerAtEdge[changeAxis]) {
						run = false;
					}
					mPlayerAtEdge[changeAxis] = 0;
				}

				if (run) {
					//Used as start and end iterators of the for loop
					int startSubAxis1 = -mLoadedChunksAroundCurrentChunk + mPlayerAtEdge[changeAxis + 1];
					int endSubAxis1 = mLoadedChunksAroundCurrentChunk + mPlayerAtEdge[changeAxis + 1];
					int startSubAxis2 = -mLoadedChunksAroundCurrentChunk + mPlayerAtEdge[changeAxis + 2];
					int endSubAxis2 = mLoadedChunksAroundCurrentChunk + mPlayerAtEdge[changeAxis + 2];

					Pos oldChunk;
					oldChunk[changeAxis] = oldAxis;
					Pos newChunk;
					newChunk[changeAxis] = newAxis;

					//Swaps the old and new Chunks with a constant X and iteratres through each Z. If confused, check Pos[] operator
					for (int i = startSubAxis1; i <= endSubAxis1; i++) {
						for (int j = startSubAxis2; j <= endSubAxis2; j++) {

							oldChunk[changeAxis + 1] = i + mPlayerPos[changeAxis + 1];
							oldChunk[changeAxis + 2] = j + mPlayerPos[changeAxis + 2];

							newChunk[changeAxis + 1] = i + mPlayerPos[changeAxis + 1];
							newChunk[changeAxis + 2] = j + mPlayerPos[changeAxis + 2];

							SwapChunk(oldChunk, newChunk);
						}

					}
				}
				
				mPlayerPos[changeAxis] += mChangeInPlayerPos[changeAxis];
			}
		}
		mPlayerPos = newPos;
		mChangeInPlayerPos = Pos();
	}















	////Check the new chunk isn't the same as the previous chunk
	//Pos newPos = GetChunk(playerChunkPos.x, playerChunkPos.y, playerChunkPos.z)->GetPos();
	//if (mPlayerPos != newPos) {

	//	//Calculate the change
	//	mChangeInPlayerPos.x = newPos.x - mPlayerPos.x;
	//	mChangeInPlayerPos.y = newPos.y - mPlayerPos.y;
	//	mChangeInPlayerPos.z = newPos.z - mPlayerPos.z;

	//	//Loop through each axis
	//	for (int axis = 0; axis < 3; axis++) {		
	//		//If there has been a change in that axis, update it
	//		if (mChangeInPlayerPos[axis] != 0) {

	//			//Used to check if an update needs to occure. E.g. if the player is at the edge of the map
	//			bool run = true;

	//			//Get the position the player has just left
	//			int oldAxis = (mLoadedChunksAroundCurrentChunk * -mChangeInPlayerPos[axis]) + mPlayerPos[axis];
	//			//Get the position the player is about to enter
	//			int newAxis = (mLoadedChunksAroundCurrentChunk * mChangeInPlayerPos[axis]) + newPos[axis];

	//			//Used as start and end iterators of the for loop
	//			int startSubAxis = -mLoadedChunksAroundCurrentChunk;
	//			int endSubAxis = mLoadedChunksAroundCurrentChunk;

	//			//Check the new value isn't out of range.
	//			if (newAxis < 0) {
	//				mPlayerAtEdge[axis] = 1;
	//				run = false;
	//			}
	//			else if (((newAxis == 0 || newAxis == 2) && newAxis >= mMaxLength) || (newAxis == 1 && newAxis >= mMaxHeight)) {
	//				mPlayerAtEdge[axis] = -1;
	//				run = false;
	//			}

	//			//Check if the player is at the edge of any other axies. Check [] operator for the Pos class if youre confused.
	//			for (int subAxis = 1; subAxis < 3; subAxis++) {
	//				int bothAxis = axis + subAxis;
	//				//If the player is at the edge
	//				if (mPlayerAtEdge[bothAxis] != 0) {
	//					//If the player has changed thair position along that axis, set mPlayerAtEdge[Axis] to zero
	//					//if (mChangeInPlayerPos[bothAxis] != 0) {
	//					if (mChangeInPlayerPos[bothAxis] == -mPlayerAtEdge[bothAxis]) {
	//						mPlayerAtEdge[bothAxis] = 0;
	//					}
	//					//Otherwise set the start iterator to take the edge into account
	//					else {
	//						startSubAxis += mPlayerAtEdge[bothAxis];
	//						endSubAxis += mPlayerAtEdge[bothAxis];
	//					}
	//				}
	//			}
	//			//Temp while Y is not implemented
	//			int a;
	//			if (axis == 0) a = 2;
	//			else if (axis == 2) a = 0;

	//			if (run) {
	//				//Swaps the old and new Chunks with a constant X and iteratres through each Z
	//				for (int iterator = startSubAxis; iterator <= endSubAxis; iterator++) {
	//					int currZ = mPlayerPos[a] + iterator;
	//					//Another FOR for the y axis
	//					Pos old;
	//					Pos neew;

	//					old[axis] = oldAxis;
	//					old.y = 0;
	//					old[a] = currZ;

	//					neew[axis] = newAxis;
	//					neew.y = 0;
	//					neew[a] = currZ;

	//					SwapChunk(old, neew);
	//				}
	//			}
	//		}
	//	}

	//	mPlayerPos = newPos;
	//}
}

void WorldManager::PrintChunkOrder() {
	std::string currentChunks = "\nCurrently loaded chunks: ";

	//Get chunk IDS
	for (int i =-mChunkRowsToLoad; i < mChunkRowsToLoad; i++) {
		//for (int j = 0; j < mChunkRowsToLoad; j++) {
		int j = 0;
		for (int k = -mChunkRowsToLoad; k < mChunkRowsToLoad; k++) {
			currentChunks += std::to_string(GetChunk(mPlayerPos.x + k, mPlayerPos.y + j, mPlayerPos.z + i)->GetID()) + ", ";
		}
		//}
	}

	GameData::Print("\n\n\nDEBUG MESSAGE:\n" + mChunkOrder + currentChunks + "\n\n\n");
}

int WorldManager::GetTotalAmountOfBlocks() {
	return mChunksToLoad * sChunkSize;
}