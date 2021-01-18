#include "WorldManager.h"

PerlinNoise WorldManager::sNoise = PerlinNoise();
std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> WorldManager::sGeometries = nullptr;
std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Material>>> WorldManager::sMaterials = nullptr;
std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> WorldManager::sAllGObjs = nullptr;
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

//WorldManager::Chunk::~Chunk() {
//	//mRItems.~shared_ptr();
//	//mActiveBlocks.~shared_ptr();
//	//mBlocks.~shared_ptr();
//}

void WorldManager::Chunk::Init(DirectX::XMFLOAT3 pos) {
	mPosition = pos;

	for (float worldX = 0; worldX < (float)WorldManager::sChunkDimension; ++worldX)
	{
		for (float worldZ = 0; worldZ < (float)WorldManager::sChunkDimension; ++worldZ)
		{
			float y = (float)WorldManager::sChunkDimension + -20.f 
				+ roundf(10.0f * (float)WorldManager::sNoise.noise((double)worldX / ((double)WorldManager::sChunkDimension), 
				(double)worldZ / ((double)WorldManager::sChunkDimension), 
				0.8));
			
			CreateCube("mat_grass",
				{ pos.x * (float)WorldManager::sChunkDimension + (float)worldX,
				pos.y * y,
				pos.z * (float)WorldManager::sChunkDimension + 1.0f * (float)worldZ }, mBlocks, mRItems);

		}
	}
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
	sAllGObjs.reset();
	sMaterials.reset();
	sGeometries.reset();
}

void WorldManager::Init(std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> geos,
						std::shared_ptr < std::unordered_map<std::string, std::shared_ptr<Material>>> mats,
						std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> gobjs,
						std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks,
						std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> riLayers[]) {
	WorldManager::sGeometries = geos;
	WorldManager::sMaterials = mats;
	WorldManager::sAllGObjs = gobjs;
	mAllBlocks = blocks;

	for (int i = 0; i < (int)GameData::RenderLayer::Count; i++)
		mRitemLayer[i] = riLayers[i];
}

void WorldManager::CreateWorld() {
	for (int i = 0; i < mMaxLength; i++) {
		for (int j = 0; j < mMaxHeight; j++) {
			for (int k = 0; k < mMaxLength; k++) {
				//mChunks.push_back(std::make_unique<Chunk>(DirectX::XMFLOAT3( (float)i, (float)j, (float)k )));
				mChunks.push_back(Chunk(DirectX::XMFLOAT3( (float)i, (float)j, (float)k )));

			}
		}
	}
}

void WorldManager::CreateCube(std::string materialName, XMFLOAT3 pos, std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> ris) {
	//Creates a render item, then uses it to create a Block. Then adds it to the needed lists
	//auto ri = std::make_shared<RenderItem>(sGeometries->at("geo_shape").get(), "mesh_cube", sMaterials->at(materialName).get(), XMMatrixTranslation(pos.x, pos.y, pos.z));
	ris->push_back(std::make_shared<RenderItem>(sGeometries->at("geo_shape").get(), "mesh_cube", sMaterials->at(materialName).get(), XMMatrixTranslation(pos.x, pos.y, pos.z)));

	//Create the block directly inside the block list
	blocks->push_back(std::make_shared<Block>(sAllGObjs, ris->at(ris->size() - 1)));


}

WorldManager::Chunk& WorldManager::GetChunk(int x, int y, int z) {
	return mChunks.at((size_t)x + y * mMaxHeight + z * mMaxHeight * mMaxLength);
}

void WorldManager::LoadChunk() {

	std::vector<std::shared_ptr<Block>>::iterator allBlockStart = mAllBlocks->end();
	std::vector<std::shared_ptr<GameObject>>::iterator allObjsStart = sAllGObjs->end();
	//std::vector<std::shared_ptr<RenderItem>>::iterator allRIStart = mRitemLayer[(int)GameData::RenderLayer::Main]->end();

	std::shared_ptr<std::vector<std::shared_ptr<Block>>> b = GetChunk(0, 0, 0).GetBlocks();
	std::vector<std::shared_ptr<Block>>::iterator blockStart = b->begin();
	std::vector<std::shared_ptr<Block>>::iterator blockEnd = b->end();

	mAllBlocks->insert(allBlockStart, blockStart, blockEnd);
	sAllGObjs->insert(allObjsStart, blockStart, blockEnd);

	for (int i = 0; i < b->size(); i++){
		mRitemLayer[(int)GameData::RenderLayer::Main]->push_back(b->at(i)->GetRI());
	}

}