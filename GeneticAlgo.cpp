#include "GeneticAlgo.h"

void Genes::Mutate() {
	float r = rand() % 1;
	float mutation;

	//Fertility
	if (r >= fertilityVolatility) {
		mutation = 1.f / (float)(rand());
		if (fmodf((float)rand(), 2.f) == 1)
			mutation *= -1;

		fertility += mutation;
	}

	//Dominance & Volitility
	for (int i = 0; i < GeneIndex::count; i++) {
		r = rand() % 1;
		if (r >= volatility[i]) {
			mutation = 1.f / (float)(rand());
			if (fmodf((float)rand(), 2.f) == 1)
				mutation *= -1;

			dominance[i] += mutation;
		}

		r = rand() % 1;
		if (r >= volatility[i]) {
			mutation = 1.f / (float)(rand());
			if (fmodf((float)rand(), 2.f) == 1)
				mutation *= -1;

			volatility[i] += mutation;
		}
	}

	MutateDungeonInfo();
}
void Genes::MutateDungeonInfo() {
	float r = rand() % 1;

	if (r >= volatility[GeneIndex::noiseSeed]) {
		unsigned int mutation = rand() % 100;
		if (fmodf((float)rand(), 2.f) == 1)
			di.noiseSeed -= mutation;
		else
			di.noiseSeed += mutation;
	}

	if (r >= volatility[GeneIndex::maxDistanceOfMainPath]) {
		int mutation = rand() % 100;
		if (fmodf((float)rand(), 2.f) == 1)
			mutation *= -1;

		di.maxDistanceOfMainPath += mutation;
	}

	if (r >= volatility[GeneIndex::minDistanceBeforeBranch]) {
		int mutation = rand() % 100;
		if (fmodf((float)rand(), 2.f) == 1)
			mutation *= -1;

		di.minDistanceBeforeBranch += mutation;
	}

	if (r >= volatility[GeneIndex::minDistanceOfMainPath]) {
		int mutation = rand() % 100;
		if (fmodf((float)rand(), 2.f) == 1)
			mutation *= -1;

		di.minDistanceOfMainPath += mutation;
	}

	if (r >= volatility[GeneIndex::ratioOfDeadEndsToReconnectingBranches]) {
		float mutation = fmodf((float)(rand()), 100);
		if (fmodf((float)rand(), 2.f) == 1)
			mutation *= -1;

		di.ratioOfDeadEndsToReconnectingBranches += mutation;
	}

	if (r >= volatility[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths]) {
		float mutation = fmodf((float)(rand()), 100);
		if (fmodf((float)rand(), 2.f) == 1)
			mutation *= -1;

		di.ratioOfLengthOfMainPathToNumberOfSidePaths += mutation;
	}

	if (r >= volatility[GeneIndex::ratioOfLengthOfMainPathToSidePaths]) {
		float mutation = fmodf((float)(rand()), 100);
		if (fmodf((float)rand(), 2.f) == 1)
			mutation *= -1;

		di.ratioOfLengthOfMainPathToSidePaths += mutation;
	}
}

GeneticAlgo::GeneticAlgo(std::shared_ptr<WorldManager> wrldmgr, size_t numbOfGenerations) {
	mWorldMgr = wrldmgr;

	CreateFolder();
	GenerateFirstGen();

	size_t index = 0;
	while (index <= numbOfGenerations && mGenetics.at(index).size() != 0) {
		
		//Create children from parents - crossover and mutation
		GenerateOffspring(index);
		std::vector<DunGen> dunGens;
		index++;

		//Generate dungeons from new genes and alayse them
		for (size_t i = 0; i < mGenetics.at(index).size(); i++) {
			DunGen dg(mWorldMgr, mGenetics.at(index).at(i).di);

			if (FitnessFunction(dg)) {
				mGenetics.at(index).at(i).fit = true;
			}
			else {
				std::ofstream output(mGenetics.at(index).at(i).di.filePath, std::ofstream::binary);
				output << "DUNGEON UNFIT\n";
				output.close();
			}

			dg.Output();
			dunGens.push_back(dg);
		}
		mDungeons.push_back(dunGens);
	}

	//Spawn the most recent fit dungeon into the world
	bool spawned = false;
	int geneIndex = mGenetics.size() - 1;
	int index2 = mGenetics.at(geneIndex).size() - 1;
	while (index2 == -1) {
		geneIndex--;
		index2 = mGenetics.at(geneIndex).size() - 1;
	}
	do {
		if (mGenetics.at(geneIndex).at(index2).fit) {
			spawned = true;
			mDungeons.at(geneIndex).at(index2).Spawn(true);
		}
		else {
			index2--;
			if (index2 == -1) {
				geneIndex--;
				index2 = mGenetics.at(geneIndex).size() - 1;
			}
		}
	}
	while (!spawned && geneIndex != -1);
}

void GeneticAlgo::CreateFolder() {
	time_t time1 = time(NULL);
	std::string folderName = ctime(&time1);
	folderName.pop_back();
	for (size_t i = 0; i < folderName.size(); i++) {
		if (folderName.at(i) == ' ')
			folderName.at(i) = '_';
		else if (folderName.at(i) == ':')
			folderName.at(i) = 'x';
	}

	mOutputFolder = mConstOutputFolder + folderName;

	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring wbuffer(buffer);
	std::string projectPath(wbuffer.begin(), wbuffer.end());

	//92 is double backslash
	int count = 3;
	while (count != 0) {
		if (*(projectPath.end() - 1) == 92) {
			projectPath.pop_back();
			count--;
		}
		else {
			projectPath.pop_back();
		}
	}

	mOutputFolder = projectPath + char(92) + mConstOutputFolder + "/" + folderName;
	std::filesystem::create_directory(mOutputFolder);

	for (size_t i = 0; i < mOutputFolder.size(); i++) {
		if (mOutputFolder.at(i) == 92)
			mOutputFolder.at(i) = '/';
	}
	mOutputFolder += "/";
}

bool GeneticAlgo::FitnessFunction(DunGen& d) {
	if(!d.IsValid())
		return false;

	const float mRatioOfStraightToBends = 0.4f;		//for every 4 straight sections there should be 10 bends
	const float mRatioOfTotalLengthToNumberOfParallelSections= 0.5f; //If more than half of the paths are parralel

	float length = (float)d.TotalLength();

	float a = (float)d.NumberOfParallelPathSpaces();
	float b = (float)d.NumberOfStraightSectionsToBends();	//Can return negative value so always less than mRatio

	if ((float)d.NumberOfParallelPathSpaces() / length >= mRatioOfTotalLengthToNumberOfParallelSections
		|| (float)d.NumberOfStraightSectionsToBends() / length >= mRatioOfStraightToBends)
		return false;
		

	return true;
}

void GeneticAlgo::GenerateFirstGen() {
	//First generation is created manually
	{
		std::vector<Genes> firstGen;

		//DungeonInfo(size_t nindex, size_t ngeneration, size_t nparentA, size_t nparentB, 
		//unsigned int nnoiseSeed, int nminDistanceOfMainPath, int nmaxDistanceOfMainPath, float nratioOfLengthOfMainPathToSidePaths, int nminDistanceBeforeBranch, 
		//float nratioOfDeadEndsToReconnectingBranches, float nratioOfLengthOfMainPathToNumberOfSidePaths
		float dominance1[GeneIndex::count] = { 0.156435f, 0.543638f, 0.8976435f, 0.3423f, 0.000123f, 0.123123f, 0.98537f, 0.762830f };
		float mutation1[GeneIndex::count] = { 0.95486f, 0.3242f, 0.98345f, 0.14178389f, 0.2348767f, 0.97141236f, 0.32423f, 0.8723423f };
		float dominance2[GeneIndex::count] = { 0.23445f, 0.3545f, 0.974358f, 0.982345f, 0.1234f, 0.1f, 0.485344f, 0.234324f };
		float mutation2[GeneIndex::count] = { 0.1234f, 0.64834f, 0.32478f, 0.8983f, 0.34322311f, 0.938437f, 0.8432f, 0.1324f};

		Genes g1({ mOutputFolder, 0, 0, 0, 0, 192u, 30, 90, 0.5, 10, 0.5f, 0.1f }, 1.f, 0.453f, 0.2342f, dominance1, mutation1);
		g1.Mutate();
		firstGen.push_back(g1);

		Genes g2({ mOutputFolder, firstGen.size(), mGenetics.size(), 0, 0, 43534u, 11, 10, 0.9, 3, 0.8f, 0.1f }, 0.5f, 0.6543f, 0.123f, mutation1, dominance1);
		g2.Mutate();
		firstGen.push_back(g2);

		Genes g3({ mOutputFolder, firstGen.size(), mGenetics.size(), 0, 0, 16632u, 66, 100, 0.3, 99, 0.2f, 0.1f }, 2.f, 0.96f, 0.1342f, dominance2, mutation2);
		g3.Mutate();
		firstGen.push_back(g3);

		Genes g4({ mOutputFolder, firstGen.size(), mGenetics.size(), 0, 0, 13421244u, 1, 32, 0.6, 20, 0.4f, 0.1f }, 1.f, 0.324f, 0.5433f, mutation2, dominance2);
		g4.Mutate();
		firstGen.push_back(g4);

		mGenetics.push_back(firstGen);
	}

	std::vector<DunGen> dunGens;
	//Generate and analyse each of the first generation dungeons
	for (size_t i = 0; i < mGenetics.at(0).size(); i++) {
		DunGen dg(mWorldMgr, mGenetics.at(0).at(i).di);
		mGenetics.at(0).at(i).fit = FitnessFunction(dg);
		dg.Output();
		dunGens.push_back(dg);
	}
	mDungeons.push_back(dunGens);
}

void GeneticAlgo::GenerateOffspring(size_t parentGeneration) {
	std::vector<size_t> validParents;
	std::vector<bool> takenParents;
	std::vector<Genes> newGeneration;

	//Find which genes from the previous generation are valid parents
	for (size_t i = 0; i < mGenetics.at(parentGeneration).size(); i++) {
		if (mGenetics.at(parentGeneration).at(i).fit) {
			validParents.push_back(i);
		}
	}

	//If there is an odd number of parents, add a second instance of one of the parents
	if ((int)(validParents.size()) % 2 != 0) {
		if (validParents.size() > 1) {
			validParents.push_back((size_t)rand() % validParents.size() - 1);
		}
		else {
			validParents.push_back(0);
		}
	}

	takenParents.insert(takenParents.begin(), validParents.size(), false);

	//Match up two random parents and create their child
	for (size_t i = 0; i < validParents.size() / 2; i++) {
		size_t parentA;
		size_t parentB;

		if (i < (validParents.size() / 2) - 1) {
			do {
				parentA = (size_t)(rand() % (validParents.size() - 1));
				parentB = (size_t)(rand() % (validParents.size() - 1));
			} while (validParents.at(parentA) == validParents.at(parentB) || takenParents.at(parentA) == true || takenParents.at(parentB) == true);
		}
		else {
			size_t j = 0;
			while (takenParents.at(j)) {
				j++;
			}
			parentA = j;

			j++;
			while (takenParents.at(j)) {
				j++;
			}
			parentB = j;
		}

		takenParents.at(parentA) = true;
		takenParents.at(parentB) = true;

		newGeneration.push_back(CreateChild(parentGeneration, validParents.at(parentA), validParents.at(parentB), i));
	}

	mGenetics.push_back(newGeneration);
}

Genes GeneticAlgo::CreateChild(size_t parentGeneration, size_t parentA, size_t parentB, size_t childIndex) {
	DungeonInfo childDI(mOutputFolder, childIndex, mGenetics.size(), parentA, parentB);
	GenerateDungeonInfo(&childDI);

	float* dominance = GenerateDominance(&childDI);
	float* mutation = GenerateMutation(&childDI);

	float fertility = Middle(mGenetics.at(childDI.generation - 1).at(childDI.parentA).fertility, mGenetics.at(childDI.generation - 1).at(childDI.parentB).fertility, 
		mGenetics.at(childDI.generation - 1).at(childDI.parentA).fertilityDominance, mGenetics.at(childDI.generation - 1).at(childDI.parentB).fertilityDominance);
	float fertilityDom = Middle(mGenetics.at(childDI.generation - 1).at(childDI.parentA).fertilityDominance, mGenetics.at(childDI.generation - 1).at(childDI.parentB).fertilityDominance,
		mGenetics.at(childDI.generation - 1).at(childDI.parentA).fertilityDominance, mGenetics.at(childDI.generation - 1).at(childDI.parentB).fertilityDominance);
	float fertilityVol = Middle(mGenetics.at(childDI.generation - 1).at(childDI.parentA).fertilityVolatility, mGenetics.at(childDI.generation - 1).at(childDI.parentB).fertilityVolatility,
		mGenetics.at(childDI.generation - 1).at(childDI.parentA).fertilityDominance, mGenetics.at(childDI.generation - 1).at(childDI.parentB).fertilityDominance);

	Genes childGenes(childDI, fertility, fertilityDom, fertilityVol, dominance, mutation);
	return childGenes;
}

void GeneticAlgo::GenerateDungeonInfo(DungeonInfo* di) {
	Genes pA = mGenetics.at(di->generation - 1).at(di->parentA);
	Genes pB = mGenetics.at(di->generation - 1).at(di->parentB);

	di->noiseSeed = Middle(pA.di.noiseSeed, pB.di.noiseSeed, pA.dominance[GeneIndex::noiseSeed], pB.dominance[GeneIndex::noiseSeed]);
	di->maxDistanceOfMainPath = Middle(pA.di.maxDistanceOfMainPath, pB.di.maxDistanceOfMainPath, pA.dominance[GeneIndex::maxDistanceOfMainPath], pB.dominance[GeneIndex::maxDistanceOfMainPath]);
	di->minDistanceBeforeBranch = Middle(pA.di.minDistanceBeforeBranch, pB.di.minDistanceBeforeBranch, pA.dominance[GeneIndex::minDistanceBeforeBranch], pB.dominance[GeneIndex::minDistanceBeforeBranch]);
	di->minDistanceOfMainPath = Middle(pA.di.minDistanceOfMainPath, pB.di.minDistanceOfMainPath, pA.dominance[GeneIndex::minDistanceOfMainPath], pB.dominance[GeneIndex::minDistanceOfMainPath]);
	di->ratioOfDeadEndsToReconnectingBranches = Middle(pA.di.ratioOfDeadEndsToReconnectingBranches, pB.di.ratioOfDeadEndsToReconnectingBranches, pA.dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches], pB.dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches]);
	di->ratioOfLengthOfMainPathToNumberOfSidePaths = Middle(pA.di.ratioOfLengthOfMainPathToNumberOfSidePaths, pB.di.ratioOfLengthOfMainPathToNumberOfSidePaths, pA.dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], pB.dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths]);
	di->ratioOfLengthOfMainPathToSidePaths = Middle(pA.di.ratioOfLengthOfMainPathToSidePaths, pB.di.ratioOfLengthOfMainPathToSidePaths, pA.dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths], pB.dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths]);
}

float* GeneticAlgo::GenerateDominance(DungeonInfo* di) {
	float ret[GeneIndex::count];

	Genes pA = mGenetics.at(di->generation - 1).at(di->parentA);
	Genes pB = mGenetics.at(di->generation - 1).at(di->parentB);

	ret[GeneIndex::noiseSeed] = Middle(pA.dominance[GeneIndex::noiseSeed], pB.dominance[GeneIndex::noiseSeed], pA.dominance[GeneIndex::noiseSeed], pB.dominance[GeneIndex::noiseSeed]);
	ret[GeneIndex::maxDistanceOfMainPath] = Middle(pA.dominance[GeneIndex::maxDistanceOfMainPath], pB.dominance[GeneIndex::maxDistanceOfMainPath], pA.dominance[GeneIndex::maxDistanceOfMainPath], pB.dominance[GeneIndex::maxDistanceOfMainPath]);
	ret[GeneIndex::minDistanceBeforeBranch] = Middle(pA.dominance[GeneIndex::minDistanceBeforeBranch], pB.dominance[GeneIndex::minDistanceBeforeBranch], pA.dominance[GeneIndex::minDistanceBeforeBranch], pB.dominance[GeneIndex::minDistanceBeforeBranch]);
	ret[GeneIndex::minDistanceOfMainPath] = Middle(pA.dominance[GeneIndex::minDistanceOfMainPath], pB.dominance[GeneIndex::minDistanceOfMainPath], pA.dominance[GeneIndex::minDistanceOfMainPath], pB.dominance[GeneIndex::minDistanceOfMainPath]);
	ret[GeneIndex::ratioOfDeadEndsToReconnectingBranches] = Middle(pA.dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches], pB.dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches], pA.dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches], pB.dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches]);
	ret[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths] = Middle(pA.dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], pB.dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], pA.dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], pB.dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths]);
	ret[GeneIndex::ratioOfLengthOfMainPathToSidePaths] = Middle(pA.dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths], pB.dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths], pA.dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths], pB.dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths]);

	return ret;
}

float* GeneticAlgo::GenerateMutation(DungeonInfo* di) {
	float ret[GeneIndex::count];

	Genes pA = mGenetics.at(di->generation - 1).at(di->parentA);
	Genes pB = mGenetics.at(di->generation - 1).at(di->parentB);

	ret[GeneIndex::noiseSeed] = Middle(pA.volatility[GeneIndex::noiseSeed], pB.volatility[GeneIndex::noiseSeed], pA.dominance[GeneIndex::noiseSeed], pB.dominance[GeneIndex::noiseSeed]);
	ret[GeneIndex::maxDistanceOfMainPath] = Middle(pA.volatility[GeneIndex::maxDistanceOfMainPath], pB.volatility[GeneIndex::maxDistanceOfMainPath], pA.dominance[GeneIndex::maxDistanceOfMainPath], pB.dominance[GeneIndex::maxDistanceOfMainPath]);
	ret[GeneIndex::minDistanceBeforeBranch] = Middle(pA.volatility[GeneIndex::minDistanceBeforeBranch], pB.volatility[GeneIndex::minDistanceBeforeBranch], pA.dominance[GeneIndex::minDistanceBeforeBranch], pB.dominance[GeneIndex::minDistanceBeforeBranch]);
	ret[GeneIndex::minDistanceOfMainPath] = Middle(pA.volatility[GeneIndex::minDistanceOfMainPath], pB.volatility[GeneIndex::minDistanceOfMainPath], pA.dominance[GeneIndex::minDistanceOfMainPath], pB.dominance[GeneIndex::minDistanceOfMainPath]);
	ret[GeneIndex::ratioOfDeadEndsToReconnectingBranches] = Middle(pA.volatility[GeneIndex::ratioOfDeadEndsToReconnectingBranches], pB.volatility[GeneIndex::ratioOfDeadEndsToReconnectingBranches], pA.dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches], pB.dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches]);
	ret[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths] = Middle(pA.volatility[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], pB.volatility[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], pA.dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], pB.dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths]);
	ret[GeneIndex::ratioOfLengthOfMainPathToSidePaths] = Middle(pA.volatility[GeneIndex::ratioOfLengthOfMainPathToSidePaths], pB.volatility[GeneIndex::ratioOfLengthOfMainPathToSidePaths], pA.dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths], pB.dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths]);

	return ret;
}







float GeneticAlgo::Crossover(float a, float b, float aDom, float bDom) {
	//Find number of digits

	//Get dominance

	//Take aDom number of digits from a, 


	float toOne = 1 - aDom + bDom;
	aDom *= toOne;
	bDom *= toOne;

	return 1;
}

float GeneticAlgo::Middle(float a, float b, float aDom, float bDom) {
	float toOne = 1.f / (aDom + bDom);
	aDom *= toOne;
	bDom *= toOne;

	a *= aDom;
	b *= bDom;

	return a + b;
}

float GeneticAlgo::Middle(double a, double b, float aDom, float bDom) {
	float toOne = 1.f / (aDom + bDom);
	aDom *= toOne;
	bDom *= toOne;

	a *= (double)aDom;
	b *= (double)bDom;

	return a + b;
}

float GeneticAlgo::Middle(int a, int b, float aDom, float bDom) {
	float toOne = 1.f / (aDom + bDom);
	aDom *= toOne;
	bDom *= toOne;

	a = (float)a * aDom;
	b = (float)b * bDom;

	return a + b;
}

float GeneticAlgo::Middle(unsigned int a, unsigned int b, float aDom, float bDom) {
	double toOne = 1.0 / ((double)aDom + (double)bDom);
	aDom *= toOne;
	bDom *= toOne;

	a = (double)a * aDom;
	b = (double)b * bDom;

	return a + b;
}