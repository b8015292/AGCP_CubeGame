#pragma once

#include "DunGen.h"
#include <filesystem>

class GeneIndex {
public:
	static const size_t noiseSeed = 0;
	static const size_t minDistanceOfMainPath = 1;
	static const size_t maxDistanceOfMainPath = 2;
	static const size_t ratioOfLengthOfMainPathToSidePaths = 3;
	static const size_t minDistanceBeforeBranch = 4;
	static const size_t ratioOfDeadEndsToReconnectingBranches = 5;
	static const size_t ratioOfLengthOfMainPathToNumberOfSidePaths = 6;

	static const size_t count = 8;
};

struct Genes {
public:

	Genes(DungeonInfo ndi, float nFertility, float nFertilityDom, float nFertilityVol, float nDominance[GeneIndex::count], float nVolatility[GeneIndex::count]);
	Genes(Genes* parentA, Genes* parentB, size_t childIndex, size_t generation);

	void GenerateDungeonInfo(Genes* parentA, Genes* parentB);
	void GenerateDominance(Genes* parentA, Genes* ParentB);
	void GenerateMutation(Genes* parentA, Genes* ParentB);

	void ForceMutate();
	void Mutate();
	void MutateDungeonInfo(float random);
	float RandZeroToOne();

	void ValidateVariables();

	void IncrementNumbOfUnfit() {
		numberOfUnfitOffspring++;
		totalNumberOfUnfitOffspring++;
	}

	static float Crossover(float a, float b, float aDom, float bDom);
	static float Middle(float a, float b, float aDom, float bDom);
	static float Middle(double a, double b, float aDom, float bDom);
	static float Middle(int a, int b, float aDom, float bDom);
	static float Middle(unsigned int a, unsigned int b, float aDom, float bDom);

	DungeonInfo di;

	//Does it pass the fitness funciton, so can it reproduce
	bool fit = false;

	//Change of producing offspring
	float fertility = 0;
	float fertilityDominance = 0;
	float fertilityVolatility = 0;

	//Gene dominace/recessive
	float dominance[GeneIndex::count];
	//mutation liklihoods
	float volatility[GeneIndex::count];
	//Dominance of the volitily (Change this mutation value will be passed to the child)
	//float mutationDominance[GeneIndex::count];

	int numberOfUnfitOffspring = 0;
	int totalNumberOfUnfitOffspring = 0;

	const float mMutationMultiplier = 0.1f;
};

class GeneticAlgo {
public:
	GeneticAlgo(std::shared_ptr<WorldManager> wrlmgr, size_t numbOfGenerations);
	static std::string mOutputFolder;

private:
	void CreateFolder();
	void GenerateFirstGen();

	void GenerateOffspringGenes(size_t parentGeneration);
	void ReGenerateOffspringGenes(size_t parentGeneration);
	void MutateUnfitParents(size_t parentGeneration);
	void DiscardUnfitParents(size_t parentGeneration);

	std::vector<DunGen> GenerateOffspringDungeons(size_t generation);
	bool FitnessFunction(DunGen& d);

	std::shared_ptr<WorldManager> mWorldMgr;
	std::vector<std::vector<Genes>> mGenetics;
	std::vector<std::vector<DunGen>> mDungeons;

	const bool mSaveAllDungeons = false;
	DunGen mPreviousFitDungeon;

	const std::string mConstOutputFolder = "output";
	const char mDoubleBackslash = 92;

	const float mRatioOfStraightToBends = 0.2f;
	const float mRatioOfTotalLengthToNumberOfParallelSections = 0.5f;

	const float mFertilityCostPerOffspring = 0.4f;
	const int mMaxNumbOfUnfitOffspringBeforeForceMutate = 4;
	const int mMaxNumbOfUnfitOffspringBeforeDiscarding = 20;
};