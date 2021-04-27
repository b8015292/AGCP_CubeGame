#include "GeneticAlgo.h"
std::string GeneticAlgo::mOutputFolder;

Genes::Genes(DungeonInfo ndi, float nFertility, float nFertilityDom, float nFertilityVol, float nDominance[GeneIndex::count], float nVolatility[GeneIndex::count]) {
	di = ndi;

	fertility = nFertility;
	fertilityDominance = nFertilityDom;
	fertilityVolatility = nFertilityVol;

	for (int i = 0; i < GeneIndex::count; i++) {
		dominance[i] = nDominance[i];
		volatility[i] = nVolatility[i];
	}
};

Genes::Genes(Genes* parentA, Genes* parentB, size_t childIndex) : di(GeneticAlgo::mOutputFolder, childIndex, parentA->di.generation++, parentA->di.index, parentB->di.index){
	GenerateDungeonInfo(parentA, parentB);
	GenerateDominance(parentA, parentB);
	GenerateMutation(parentA, parentB);

	fertility = Middle(parentA->fertility, parentB->fertility,
		parentA->fertilityDominance, parentB->fertilityDominance);
	fertilityDominance = Middle(parentA->fertilityDominance, parentB->fertilityDominance,
		parentA->fertilityDominance, parentB->fertilityDominance);
	fertilityVolatility = Middle(parentA->fertilityVolatility, parentB->fertilityVolatility,
		parentA->fertilityDominance, parentB->fertilityDominance);
}

void Genes::GenerateDungeonInfo(Genes* parentA, Genes* parentB) {
	di.noiseSeed = Middle(parentA->di.noiseSeed, parentB->di.noiseSeed, parentA->dominance[GeneIndex::noiseSeed], parentB->dominance[GeneIndex::noiseSeed]);
	di.maxDistanceOfMainPath = Middle(parentA->di.maxDistanceOfMainPath, parentB->di.maxDistanceOfMainPath, parentA->dominance[GeneIndex::maxDistanceOfMainPath], parentB->dominance[GeneIndex::maxDistanceOfMainPath]);
	di.minDistanceBeforeBranch = Middle(parentA->di.minDistanceBeforeBranch, parentB->di.minDistanceBeforeBranch, parentA->dominance[GeneIndex::minDistanceBeforeBranch], parentB->dominance[GeneIndex::minDistanceBeforeBranch]);
	di.minDistanceOfMainPath = Middle(parentA->di.minDistanceOfMainPath, parentB->di.minDistanceOfMainPath, parentA->dominance[GeneIndex::minDistanceOfMainPath], parentB->dominance[GeneIndex::minDistanceOfMainPath]);
	di.ratioOfDeadEndsToReconnectingBranches = Middle(parentA->di.ratioOfDeadEndsToReconnectingBranches, parentB->di.ratioOfDeadEndsToReconnectingBranches, parentA->dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches], parentB->dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches]);
	di.ratioOfLengthOfMainPathToNumberOfSidePaths = Middle(parentA->di.ratioOfLengthOfMainPathToNumberOfSidePaths, parentB->di.ratioOfLengthOfMainPathToNumberOfSidePaths, parentA->dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], parentB->dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths]);
	di.ratioOfLengthOfMainPathToSidePaths = Middle(parentA->di.ratioOfLengthOfMainPathToSidePaths, parentB->di.ratioOfLengthOfMainPathToSidePaths, parentA->dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths], parentB->dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths]);
}

void Genes::GenerateDominance(Genes* parentA, Genes* parentB) {

	dominance[GeneIndex::noiseSeed] = Middle(parentA->dominance[GeneIndex::noiseSeed], parentB->dominance[GeneIndex::noiseSeed], parentA->dominance[GeneIndex::noiseSeed], parentB->dominance[GeneIndex::noiseSeed]);
	dominance[GeneIndex::maxDistanceOfMainPath] = Middle(parentA->dominance[GeneIndex::maxDistanceOfMainPath], parentB->dominance[GeneIndex::maxDistanceOfMainPath], parentA->dominance[GeneIndex::maxDistanceOfMainPath], parentB->dominance[GeneIndex::maxDistanceOfMainPath]);
	dominance[GeneIndex::minDistanceBeforeBranch] = Middle(parentA->dominance[GeneIndex::minDistanceBeforeBranch], parentB->dominance[GeneIndex::minDistanceBeforeBranch], parentA->dominance[GeneIndex::minDistanceBeforeBranch], parentB->dominance[GeneIndex::minDistanceBeforeBranch]);
	dominance[GeneIndex::minDistanceOfMainPath] = Middle(parentA->dominance[GeneIndex::minDistanceOfMainPath], parentB->dominance[GeneIndex::minDistanceOfMainPath], parentA->dominance[GeneIndex::minDistanceOfMainPath], parentB->dominance[GeneIndex::minDistanceOfMainPath]);
	dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches] = Middle(parentA->dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches], parentB->dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches], parentA->dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches], parentB->dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches]);
	dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths] = Middle(parentA->dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], parentB->dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], parentA->dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], parentB->dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths]);
	dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths] = Middle(parentA->dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths], parentB->dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths], parentA->dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths], parentB->dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths]);
}

void Genes::GenerateMutation(Genes* parentA, Genes* parentB) {
	volatility[GeneIndex::noiseSeed] = Middle(parentA->volatility[GeneIndex::noiseSeed], parentB->volatility[GeneIndex::noiseSeed], parentA->dominance[GeneIndex::noiseSeed], parentB->dominance[GeneIndex::noiseSeed]);
	volatility[GeneIndex::maxDistanceOfMainPath] = Middle(parentA->volatility[GeneIndex::maxDistanceOfMainPath], parentB->volatility[GeneIndex::maxDistanceOfMainPath], parentA->dominance[GeneIndex::maxDistanceOfMainPath], parentB->dominance[GeneIndex::maxDistanceOfMainPath]);
	volatility[GeneIndex::minDistanceBeforeBranch] = Middle(parentA->volatility[GeneIndex::minDistanceBeforeBranch], parentB->volatility[GeneIndex::minDistanceBeforeBranch], parentA->dominance[GeneIndex::minDistanceBeforeBranch], parentB->dominance[GeneIndex::minDistanceBeforeBranch]);
	volatility[GeneIndex::minDistanceOfMainPath] = Middle(parentA->volatility[GeneIndex::minDistanceOfMainPath], parentB->volatility[GeneIndex::minDistanceOfMainPath], parentA->dominance[GeneIndex::minDistanceOfMainPath], parentB->dominance[GeneIndex::minDistanceOfMainPath]);
	volatility[GeneIndex::ratioOfDeadEndsToReconnectingBranches] = Middle(parentA->volatility[GeneIndex::ratioOfDeadEndsToReconnectingBranches], parentB->volatility[GeneIndex::ratioOfDeadEndsToReconnectingBranches], parentA->dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches], parentB->dominance[GeneIndex::ratioOfDeadEndsToReconnectingBranches]);
	volatility[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths] = Middle(parentA->volatility[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], parentB->volatility[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], parentA->dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths], parentB->dominance[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths]);
	volatility[GeneIndex::ratioOfLengthOfMainPathToSidePaths] = Middle(parentA->volatility[GeneIndex::ratioOfLengthOfMainPathToSidePaths], parentB->volatility[GeneIndex::ratioOfLengthOfMainPathToSidePaths], parentA->dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths], parentB->dominance[GeneIndex::ratioOfLengthOfMainPathToSidePaths]);
}

float Genes::RandZeroToOne() {
	return (float)rand() / RAND_MAX;
}

void Genes::Mutate() {
	//float r = fmodf((float)rand(), 1.f);
	//float r = 1.f / (float)rand();
	float random = RandZeroToOne();
	float mutation;

	//Fertility
	if (random >= fertilityVolatility) {
		mutation = fertility * (RandZeroToOne() * 0.1);
		if (RandZeroToOne() > 0.5)
			mutation *= -1;

		fertility += mutation;
	}

	//Dominance & Volitility
	for (int i = 0; i < GeneIndex::count; i++) {
		if (random >= volatility[i]) {
			mutation = dominance[i] * (RandZeroToOne() * 0.1);
			if (RandZeroToOne() > 0.5)
				mutation *= -1;

			dominance[i] += mutation;
		}
		if (random >= volatility[i]) {
			mutation = volatility[i] * (RandZeroToOne() * 0.1);
			if (RandZeroToOne() > 0.5)
				mutation *= -1;

			volatility[i] += mutation;
		}
	}

	MutateDungeonInfo(random);
}
void Genes::MutateDungeonInfo(float random) {
	float mutation;
	if (random >= volatility[GeneIndex::noiseSeed]) {
		mutation = di.noiseSeed * (RandZeroToOne() * 0.1);
		if (RandZeroToOne() > 0.5)
			mutation *= -1;

		di.noiseSeed += mutation;
	}
	if (random >= volatility[GeneIndex::maxDistanceOfMainPath]) {
		mutation = di.noiseSeed * (RandZeroToOne() * 0.1);
		if (RandZeroToOne() > 0.5)
			mutation *= -1;

		di.maxDistanceOfMainPath += mutation;
	}
	if (random >= volatility[GeneIndex::minDistanceBeforeBranch]) {
		mutation = di.noiseSeed * (RandZeroToOne() * 0.1);
		if (RandZeroToOne() > 0.5)
			mutation *= -1;

		di.minDistanceBeforeBranch += mutation;
	}
	if (random >= volatility[GeneIndex::minDistanceOfMainPath]) {
		mutation = di.noiseSeed * (RandZeroToOne() * 0.1);
		if (RandZeroToOne() > 0.5)
			mutation *= -1;

		di.minDistanceOfMainPath += mutation;
	}
	if (random >= volatility[GeneIndex::ratioOfDeadEndsToReconnectingBranches]) {
		mutation = di.noiseSeed * (RandZeroToOne() * 0.1);
		if (RandZeroToOne() > 0.5)
			mutation *= -1;

		di.ratioOfDeadEndsToReconnectingBranches += mutation;
	}
	if (random >= volatility[GeneIndex::ratioOfLengthOfMainPathToNumberOfSidePaths]) {
		mutation = di.noiseSeed * (RandZeroToOne() * 0.1);
		if (RandZeroToOne() > 0.5)
			mutation *= -1;

		di.ratioOfLengthOfMainPathToNumberOfSidePaths += mutation;
	}
	if (random >= volatility[GeneIndex::ratioOfLengthOfMainPathToSidePaths]) {
		mutation = di.noiseSeed * (RandZeroToOne() * 0.1);
		if (RandZeroToOne() > 0.5)
			mutation *= -1;

		di.ratioOfLengthOfMainPathToSidePaths += mutation;
	}
}

float Genes::Crossover(float a, float b, float aDom, float bDom) {
	//Find number of digits

	//Get dominance

	//Take aDom number of digits from a, 


	float toOne = 1 - aDom + bDom;
	aDom *= toOne;
	bDom *= toOne;

	return 1;
}

float Genes::Middle(float a, float b, float aDom, float bDom) {
	float toOne = 1.f / (aDom + bDom);
	aDom *= toOne;
	bDom *= toOne;

	a *= aDom;
	b *= bDom;

	return a + b;
}

float Genes::Middle(double a, double b, float aDom, float bDom) {
	float toOne = 1.f / (aDom + bDom);
	aDom *= toOne;
	bDom *= toOne;

	a *= (double)aDom;
	b *= (double)bDom;

	return a + b;
}

float Genes::Middle(int a, int b, float aDom, float bDom) {
	float toOne = 1.f / (aDom + bDom);
	aDom *= toOne;
	bDom *= toOne;

	a = (float)a * aDom;
	b = (float)b * bDom;

	return a + b;
}

float Genes::Middle(unsigned int a, unsigned int b, float aDom, float bDom) {
	double toOne = 1.0 / ((double)aDom + (double)bDom);
	aDom *= toOne;
	bDom *= toOne;

	a = (double)a * aDom;
	b = (double)b * bDom;

	return a + b;
}







GeneticAlgo::GeneticAlgo(std::shared_ptr<WorldManager> wrldmgr, size_t numbOfGenerations) {
	mWorldMgr = wrldmgr;

	CreateFolder();
	GenerateFirstGen();

	size_t index = 0;
	while (index <= numbOfGenerations && mGenetics.at(index).size() != 0) {
		
		//Create children from parents - crossover and mutation
		GenerateOffspring(index);
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
		}
	}
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

	const float mRatioOfStraightToBends = 0.2f;
	const float mRatioOfTotalLengthToNumberOfParallelSections= 0.2f;

	float length = (float)d.TotalLength();

	float a = (float)d.NumberOfParallelPathSpaces();
	float b = (float)d.NumberOfStraightSectionsToBends();

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
		firstGen.push_back(g1);

		Genes g2({ mOutputFolder, firstGen.size(), mGenetics.size(), 0, 0, 43534u, 11, 10, 0.9, 3, 0.8f, 0.1f }, 0.5f, 0.6543f, 0.123f, mutation1, dominance1);
		firstGen.push_back(g2);

		Genes g3({ mOutputFolder, firstGen.size(), mGenetics.size(), 0, 0, 16632u, 66, 100, 0.3, 99, 0.2f, 0.1f }, 2.f, 0.96f, 0.1342f, dominance2, mutation2);
		firstGen.push_back(g3);

		Genes g4({ mOutputFolder, firstGen.size(), mGenetics.size(), 0, 0, 13421244u, 1, 32, 0.6, 20, 0.4f, 0.1f }, 1.f, 0.324f, 0.5433f, mutation2, dominance2);
		firstGen.push_back(g4);

		mGenetics.push_back(firstGen);
	}

	//Generate and analyse each of the first generation dungeons
	for (size_t i = 0; i < mGenetics.at(0).size(); i++) {
		DunGen dg(mWorldMgr, mGenetics.at(0).at(i).di);
		mGenetics.at(0).at(i).fit = FitnessFunction(dg);
		dg.Output();
	}
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

		newGeneration.push_back({ &mGenetics.at(parentGeneration).at(validParents.at(parentA)), &mGenetics.at(parentGeneration).at(validParents.at(parentB)), i });
		newGeneration.at(newGeneration.size() - 1).Mutate();
	}

	mGenetics.push_back(newGeneration);
}