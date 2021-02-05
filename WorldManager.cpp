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
			float noise = roundf(40.0f * (float)WorldManager::sNoise.OctavePerlin(((double)worldX + (pos.x * (float)WorldManager::sChunkDimension)) * 0.1f, (double)(worldZ + (pos.z * (float)WorldManager::sChunkDimension)) * 0.1f, 0.8, 6, 10));

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

	//std::srand(std::time(nullptr));
	//noise = PerlinNoise(std::rand());
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
	//DB
	//for (int z = 0; z < mMaxLength; z++) {
	//	for (int y = 0; y < mMaxHeight; y++) {
	//		for (int x = 0; x < mMaxLength; x++) {
	//			std::shared_ptr<Chunk> c = GetChunk(x, y, z);
	//			Pos p = c->GetPos();
	//			GameData::Print(std::to_string(c->GetID()) + ": x" + std::to_string(p.x) + ". y" + std::to_string(p.y) + ". z" + std::to_string(p.z) + '\n');
	//		}
	//	}
	//}
}

void WorldManager::CreateCube(std::string materialName, XMFLOAT3 pos, bool active, std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> ris) {
	//Creates a render item, then uses it to create a Block. Then adds it to the needed lists
	ris->push_back(std::make_shared<RenderItem>(sGeometries->at("geo_shape").get(), "mesh_cube", sMaterials->at(materialName).get(), XMMatrixTranslation(pos.x, pos.y, pos.z)));

	//Create the block directly inside the block list
	blocks->push_back(std::make_shared<Block>( ris->at(ris->size() - 1)));
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
	if (!IsChunkCoordValid(x, y, z))
		return;

	std::shared_ptr<Chunk> chunk = GetChunk(x, y, z);

	if (chunk->GetAcitve())
		return;

	chunk->SetAcitve(true);
	chunk->SetStartIndexes(Block::sAllBlocks->size(), GameObject::sAllGObjs->size(), mRitemLayer[mRenderLayer]->size());
	
	//Get the vectors to insert
	std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocksToInsert = chunk->GetBlocks();
	std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> ris = chunk->GetRItems();
	UINT count;
	if(!mCreatedWorld )
		count = GetRenderItemCount();

	//Insert the chunk
	Block::sAllBlocks->insert(Block::sAllBlocks->begin() + chunk->GetBlockStartIndex(), blocksToInsert->begin(), blocksToInsert->end());

	GameObject::sAllGObjs->insert(GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex(), blocksToInsert->begin(), blocksToInsert->end());
	mRitemLayer[mRenderLayer]->insert(mRitemLayer[mRenderLayer]->begin() + chunk->GetRIStartIndex(), ris->begin(), ris->end());

	//Set the render items object CB index so it can be found and updated by the GPU
	if (mCreatedWorld) {
		//Once the world has been loaded, get the CB index from the list of free indexes
		for (size_t i = chunk->GetRIStartIndex(); i < chunk->GetRIStartIndex() + sChunkSize; i++) {
			mRitemLayer[mRenderLayer]->at(i)->ObjCBIndex = GameData::GetObjectCBIndex();
			mRitemLayer[mRenderLayer]->at(i)->NumFramesDirty = GameData::sNumFrameResources;
		}
	}
	else {
		//If these are the first chunks being loaded, set their CB index to be ordered 0.... inifinate. So there are no gaps.
		for (size_t i = chunk->GetRIStartIndex(); i < chunk->GetRIStartIndex() + sChunkSize; i++, count++) {
			mRitemLayer[mRenderLayer]->at(i)->ObjCBIndex = count;
		}
	}
}

void WorldManager::UnloadChunk(int x, int y, int z) {
	std::shared_ptr<Chunk> chunk = GetChunk(x, y, z);
	
	GameData::Print(std::to_string(chunk->GetID()));

	if(!chunk->GetAcitve())
		return;

	chunk->SetAcitve(false);

	for (std::vector<std::shared_ptr<RenderItem>>::iterator it = chunk->GetRItems()->begin(); it != chunk->GetRItems()->end(); it++) {
		GameData::AddNewObjectCBIndex((*it)->ObjCBIndex);
	}

	Block::sAllBlocks->erase(Block::sAllBlocks->begin() + chunk->GetBlockStartIndex(), Block::sAllBlocks->begin() + chunk->GetBlockStartIndex() + sChunkSize );
	GameObject::sAllGObjs->erase(GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex(), GameObject::sAllGObjs->begin() + chunk->GetGObjStartIndex() + sChunkSize);
	mRitemLayer[mRenderLayer]->erase(mRitemLayer[mRenderLayer]->begin() + chunk->GetRIStartIndex(),
		mRitemLayer[mRenderLayer]->begin() + chunk->GetRIStartIndex() + sChunkSize);

	chunk->SetStartIndexes(-1, -1, -1);
}

void WorldManager::SwapChunk(Pos old, Pos neew) {
	if (!IsChunkCoordValid(old.x, old.y, old.z) || !IsChunkCoordValid(neew.x, neew.y, neew.z))
		return;

	std::shared_ptr<Chunk> oldChunk = GetChunk(old.x, old.y, old.z);
	std::shared_ptr<Chunk> newChunk = GetChunk(neew.x, neew.y, neew.z);

	std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> oldRIs = oldChunk->GetRItems();
	std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> newRIs = newChunk->GetRItems();


	if (!oldChunk->GetAcitve() || newChunk->GetAcitve())
		return;

	oldChunk->SetAcitve(false);
	newChunk->SetAcitve(true);

	//Copy the chunk into the main lists, replacing the old chunk
	std::copy(newChunk->GetBlocks()->begin(), newChunk->GetBlocks()->end(), Block::sAllBlocks->begin() + oldChunk->GetBlockStartIndex());
	std::copy(newChunk->GetBlocks()->begin(), newChunk->GetBlocks()->end(), GameObject::sAllGObjs->begin() + oldChunk->GetGObjStartIndex());
	std::copy(newRIs->begin(), newRIs->end(),  mRitemLayer[mRenderLayer]->begin() + oldChunk->GetRIStartIndex());

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

	if (!IsChunkCoordValid(x, y, z))
		return GetChunk(0, 0, 0);

	return GetChunk(x, y, z);
}

WorldManager::Pos WorldManager::GetPlayerChunkCoords(DirectX::XMFLOAT3 pos) {
	return Pos((int)floorf(pos.x / sChunkDimension), (int)floorf(pos.y / sChunkDimension), (int)floorf(pos.z / sChunkDimension));
}

void WorldManager::LoadFirstChunks(float playerX, float playerZ) {

	mPlayerPos = GetPlayerChunk({ playerX, 0, playerZ})->GetPos();
	Pos start(mPlayerPos.x - mLoadedChunksAroundCurrentChunk, mPlayerPos.y - mLoadedChunksAroundCurrentChunk, mPlayerPos.z - mLoadedChunksAroundCurrentChunk);
	//Pos start(mPlayerPos.x - mLoadedChunksAroundCurrentChunk, mPlayerPos.y - mLoadedChunksAroundCurrentChunk, mPlayerPos.z - mLoadedChunksAroundCurrentChunk);
	for (int i = 0; i < mChunkRowsToLoad; i++) {
		//for (int j = 0; j < mChunkRowsToLoad; j++) {
		int j = 0;
			for (int k = 0; k < mChunkRowsToLoad; k++) {
				LoadChunk(k, j, i);
			}
		//}
	}

	mCreatedWorld = true;
}

void WorldManager::UpdatePlayerPosition(DirectX::XMFLOAT3 worldPos) {

	Pos playerChunkPos = GetPlayerChunkCoords(worldPos);
	if (!IsChunkCoordValid(playerChunkPos.x, playerChunkPos.y, playerChunkPos.z))
		return;

	Pos newPos = GetChunk(playerChunkPos.x, playerChunkPos.y, playerChunkPos.z)->GetPos();
	if (mPlayerPos != newPos) {
		if (playerChunkPos.x == 3 && playerChunkPos.z == 3) {
			int q = 7;
		}

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
					if (mPlayerAtEdge[axis + subAxis] != 0 && ((axis + subAxis != 1) || (axis + subAxis != 4))) {
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