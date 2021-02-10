#include "WorldManager.h"

PerlinNoise WorldManager::sNoise = PerlinNoise();
std::shared_ptr<std::unordered_map<std::string, int>> WorldManager::sMaterialIndexes = nullptr;
int WorldManager::sChunkMaxID = 0;

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

	for (float worldZ = 0; worldZ < (float)WorldManager::sChunkDimension; ++worldZ)
	{
		for (float worldX = 0; worldX < (float)WorldManager::sChunkDimension; ++worldX)
		{

			//float noise = roundf(10.0f * (float)WorldManager::sNoise.noise((double)worldX / ((double)WorldManager::sChunkDimension),
			//		(double)worldZ / ((double)WorldManager::sChunkDimension), 0.8));
			float noise = roundf(40.0f * (float)WorldManager::sNoise.OctavePerlin(((double)worldX + (pos.x * (float)WorldManager::sChunkDimension)) * 0.1f, (double)(worldZ + (pos.z * (float)WorldManager::sChunkDimension)) * 0.1f, 0.8, 6, 10));

			for (float worldY = 0; worldY < (float)WorldManager::sChunkDimension; ++worldY) {
				if (worldY < noise) {
					CreateCube("mat_dirt",
						{ pos.x * (float)WorldManager::sChunkDimension + (float)worldX,
						pos.y * (float)WorldManager::sChunkDimension + (float)worldY,
						pos.z * (float)WorldManager::sChunkDimension + 1.0f * (float)worldZ }, true, mBlocks, mInstanceDatas);
				}
				else if(worldY == noise){
					CreateCube("mat_grass",
						{ pos.x * (float)WorldManager::sChunkDimension + (float)worldX,
						pos.y * (float)WorldManager::sChunkDimension + (float)worldY,
						pos.z * (float)WorldManager::sChunkDimension + 1.0f * (float)worldZ }, true, mBlocks, mInstanceDatas);
				}
				else {	//Blocks above the noise wave are not active upon creation
					CreateCube("mat_grass",
						{ pos.x * (float)WorldManager::sChunkDimension + (float)worldX,
						pos.y * (float)WorldManager::sChunkDimension + (float)worldY,
						pos.z * (float)WorldManager::sChunkDimension + 1.0f * (float)worldZ }, false, mBlocks, mInstanceDatas);
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
	mPosition = c.mPosition;
	mInstanceDatas = c.mInstanceDatas;
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
	mChunkRowsToLoad = 1 + 2 * mLoadedChunksAroundCurrentChunk;
	mChunksToLoad = mChunkRowsToLoad * mChunkRowsToLoad * mChunkRowsToLoad;
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

void WorldManager::CreateCube(std::string materialName, XMFLOAT3 pos, bool active, std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, std::shared_ptr<std::vector<std::shared_ptr<InstanceData>>> blockInstances) {
	//Creates a render item, then uses it to create a Block. Then adds it to the needed lists
	auto idata = std::make_shared<InstanceData>(XMMatrixTranslation(pos.x, pos.y, pos.z), sMaterialIndexes->at(materialName));
	blockInstances->push_back(idata);

	//Create the block directly inside the block list
	blocks->push_back(std::make_shared<Block>(idata));
	blocks->at(blocks->size()- 1)->SetActive(active);
}

std::shared_ptr<WorldManager::Chunk> WorldManager::GetChunk(int x, int y, int z) {
	return mChunks.at((size_t)x + (y * mMaxLength) + (z * mMaxHeight * mMaxLength));
	//return mChunks.at((size_t)x + (z * mMaxHeight * mMaxLength));
}

bool WorldManager::IsChunkCoordValid(int x, int y, int z) {
	if (x >= mMaxLength || y >= mMaxHeight || z >= mMaxLength
		|| x < 0 || y < 0 || z < 0)
		return false;

	return true;
}

void WorldManager::LoadChunk(int x, int y, int z) {
	//Check the coord is valid, if so get the chunk and check it isn't already active (loaded)
	if (!IsChunkCoordValid(x, y, z)) return;
	std::shared_ptr<Chunk> chunk = GetChunk(x, y, z);
	if (chunk->GetAcitve()) return;
		
	//Set the chunk to active and set its starting indexes
	chunk->SetAcitve(true);
	chunk->SetStartIndexes(Block::sAllBlocks->size(), GameObject::sAllGObjs->size(), Block::sBlockInstance->Instances.size());
	
	//Get the vectors to insert
	std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocksToInsert = chunk->GetBlocks();
	std::shared_ptr<std::vector<std::shared_ptr<InstanceData>>> instances = chunk->GetInstanceDatas();
	const size_t instanceStart = chunk->GetInstanceStartIndex();
	UINT count = (UINT)Block::sBlockInstance->Instances.size();

	//Insert the chunk into the lists
	Block::sAllBlocks->insert(Block::sAllBlocks->begin() + chunk->GetBlockStartIndex(), blocksToInsert->begin(), blocksToInsert->end());
	GameObject::sAllGObjs->insert(GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex(), blocksToInsert->begin(), blocksToInsert->end());
	Block::sBlockInstance->Instances.insert(Block::sBlockInstance->Instances.begin() + instanceStart, instances->begin(), instances->end());

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
	chunk->SetAcitve(false);

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

	//Check they are/aren't active. Then set their new active state
	if (!oldChunk->GetAcitve() || newChunk->GetAcitve())
		return;
	oldChunk->SetAcitve(false);
	newChunk->SetAcitve(true);

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

std::shared_ptr<Block> WorldManager::GetBlock(DirectX::XMFLOAT3 pos) {
	//Get the chunk
	int cx = (int)floorf(pos.x / sChunkDimension);
	int cy = (int)floorf(pos.y / sChunkDimension);
	int cz = (int)floorf(pos.z / sChunkDimension);
	std::shared_ptr<Chunk> c = mChunks.at(size_t(cx + (cy * mMaxLength) + (cz * mMaxLength * mMaxHeight)));

	//Find the block coords in terms of its chunk
	int bx = (int)floorf(pos.x) - cx * sChunkDimension;
	int by = (int)floorf(pos.y)	- cy * sChunkDimension;	//There is only one Y chunk!
	int bz = (int)floorf(pos.z) - cz * sChunkDimension;

	return c->GetBlocks()->at((size_t)(by + (bx * sChunkDimension) + (bz * sChunkDimension * sChunkDimension) - 1));
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

	if (!IsChunkCoordValid(x, y, z))
		return GetChunk(0, 0, 0);

	return GetChunk(x, y, z);
}

WorldManager::Pos WorldManager::GetPlayerChunkCoords(DirectX::XMFLOAT3 pos) {
	return Pos((int)floorf(pos.x / sChunkDimension), (int)floorf(pos.y / sChunkDimension), (int)floorf(pos.z / sChunkDimension));
}

void WorldManager::LoadFirstChunks(float playerX, float playerZ) {

	mPlayerPos = GetPlayerChunk({ playerX, 0, playerZ})->GetPos();
	mPlayerPos.y = 2;
	//Pos start(mPlayerPos.x - mLoadedChunksAroundCurrentChunk, mPlayerPos.y - mLoadedChunksAroundCurrentChunk, mPlayerPos.z - mLoadedChunksAroundCurrentChunk);
	Pos start(mPlayerPos.x - mLoadedChunksAroundCurrentChunk, mPlayerPos.y - mLoadedChunksAroundCurrentChunk, mPlayerPos.z - mLoadedChunksAroundCurrentChunk);
	for (int i = 0; i < mChunkRowsToLoad; i++) {
		for (int j = 0; j < mChunkRowsToLoad; j++) {
			for (int k = 0; k < mChunkRowsToLoad; k++) {
				LoadChunk(k, j, i);
			}
		}
	}

	mCreatedWorld = true;
}

void WorldManager::UpdatePlayerPosition(DirectX::XMFLOAT3 worldPos) {

	Pos playerChunkPos = GetPlayerChunkCoords(worldPos);
	if (!IsChunkCoordValid(playerChunkPos.x, playerChunkPos.y, playerChunkPos.z))
		return;

	Pos newPos = GetChunk(playerChunkPos.x, playerChunkPos.y, playerChunkPos.z)->GetPos();
	if (mPlayerPos != newPos) {

		//Calculate the change
		mChangeInPlayerPos.x = newPos.x - mPlayerPos.x;
		mChangeInPlayerPos.y = newPos.y - mPlayerPos.y;
		mChangeInPlayerPos.z = newPos.z - mPlayerPos.z;

		//Loop through each axis to check if it needs updating
		for (int axis = 0; axis < 3; axis++) {				
			if (mChangeInPlayerPos[axis] != 0 && axis != 1) {


				//DEBUG
				mChunkOrder += std::to_string(GetChunk(playerChunkPos.x, playerChunkPos.y, playerChunkPos.z)->GetID()) + ", ";


				bool run = true;

				//Get the position the player has just left
				int oldAxis = (mLoadedChunksAroundCurrentChunk * -mChangeInPlayerPos[axis]) + mPlayerPos[axis];
				//Get the position the player is about to enter
				int newAxis = (mLoadedChunksAroundCurrentChunk * mChangeInPlayerPos[axis]) + newPos[axis];

				int startSubAxis = -mLoadedChunksAroundCurrentChunk;
				int endSubAxis = mLoadedChunksAroundCurrentChunk;

				//Check the new value isn't out of range.
				if (newAxis < 0) {
					mPlayerAtEdge[axis] = 1;
					run = false;
				}
				else if (newAxis >= mMaxLength) {
					mPlayerAtEdge[axis] = -1;
					run = false;
				}

				//Check if the player is at the edge of any other axies
				for (int subAxis = 1; subAxis < 3; subAxis++) {
					//If the player is at the edge
					if (mPlayerAtEdge[axis + subAxis] != 0 && !((axis + subAxis == 1) || (axis + subAxis == 4))) {
						//If the player has changed thair position along that axis, set mPlayerAtEdge[Axis] to zero
						if (mChangeInPlayerPos[axis + subAxis] != 0) {
							mPlayerAtEdge[axis + subAxis] = 0;
						}
						//Otherwise set the start iterator to take the edge into account
						else {
							startSubAxis += mPlayerAtEdge[axis + subAxis];
							endSubAxis += mPlayerAtEdge[axis + subAxis];
						}
					}
				}
				//Temp while Y is not implemented
				int a;
				if (axis == 0) a = 2;
				else if (axis == 2) a = 0;

				if (run) {
					//Swaps the old and new Chunks with a constant X and iteratres through each Z
					for (int iterator = startSubAxis; iterator <= endSubAxis; iterator++) {
						int currZ = mPlayerPos[a] + iterator;
						//Another FOR for the y axis
						Pos old;
						Pos neew;

						old[axis] = oldAxis;
						old.y = 0;
						old[a] = currZ;

						neew[axis] = newAxis;
						neew.y = 0;
						neew[a] = currZ;

						SwapChunk(old, neew);
					}
				}
			}
		}

		mPlayerPos = newPos;
	}
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