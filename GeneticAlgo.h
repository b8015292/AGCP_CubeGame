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
	Genes(Genes* parentA, Genes* parentB, size_t childIndex);

	void GenerateDungeonInfo(Genes* parentA, Genes* parentB);
	void GenerateDominance(Genes* parentA, Genes* ParentB);
	void GenerateMutation(Genes* parentA, Genes* ParentB);

	void Mutate();
	void MutateDungeonInfo(float random);
	float RandZeroToOne();

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
};

class GeneticAlgo {
public:
	GeneticAlgo(std::shared_ptr<WorldManager> wrlmgr, size_t numbOfGenerations);
	static std::string mOutputFolder;

private:
	void CreateFolder();
	void GenerateFirstGen();
	void GenerateOffspring(size_t parentGeneration);
	bool FitnessFunction(DunGen& d);

	std::shared_ptr<WorldManager> mWorldMgr;
	std::vector<std::vector<Genes>> mGenetics;

	const std::string mConstOutputFolder = "output";
};