#include "WaveFunctionCollapse.h"
#include <stack>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "ExtraString.h"

WaveFunctionCollapse::WaveFunctionCollapse(int sideLength, int height, const cellOption_t& options)
{
	this->sideLength = sideLength;
	this->height = height;
	this->typeCount = options.size();

	cellOption_t emptyOptions;
	cellOption_t fullOptions;
	cellOption_t trackOptions;


	for (std::shared_ptr<GridTile> tileType : options)
	{
		std::vector<SpecialProperty> properties = tileType->properties;
		if (std::find(properties.begin(), properties.end(), emptyTile) != properties.end())
		{
			emptyOptions.push_back(tileType);
		}
		if (std::find(properties.begin(), properties.end(), fullTile) != properties.end())
		{
			fullOptions.push_back(tileType);
		}
		if (std::find(properties.begin(), properties.end(), trackNode60) != properties.end()
			|| std::find(properties.begin(), properties.end(), trackNode180) != properties.end()
			|| std::find(properties.begin(), properties.end(), trackNode300) != properties.end())
		{
			trackOptions.push_back(tileType);
		}
	}

	std::cout << emptyOptions.size() << std::endl;
	std::cout << fullOptions.size() << std::endl;
	std::cout << trackOptions.size() << std::endl;


	for (size_t y = 1; y < height; y++)
	{
		for (size_t z = 0; z < sideLength * 2; z++)
		{
			for (size_t x = 0; x < sideLength * 4; x++)
			{

				//Coating the borders with empty

				if (y + 1 == height || (x < 1) || x > sideLength * 4 - 2 || (z == 0 && x % 2 == 1) || (z >= sideLength * 2 - 1 && x % 2 == 0))
				{
					grid.push_back(cellOption_t(emptyOptions));
				}
				else
				{
					grid.push_back(cellOption_t(options));
				}


				//Debug Code: use this instead if you want to remove border constraints
				//grid.push_back(cellOption_t(options));
			}
		}
	}

	//Fill the centre with full so that there must be terrain
	grid[sideLength * 2 - 1 + (sideLength - 1) * sideLength * 4] = cellOption_t({ fullOptions });
	grid[sideLength * 2 - 1 + sideLength * sideLength * 4] = cellOption_t({ fullOptions });
	grid[sideLength * 2 + (sideLength - 1) * sideLength * 4] = cellOption_t({ fullOptions });
	grid[sideLength * 2 + sideLength * sideLength * 4] = cellOption_t({ fullOptions });

	//a tile in the middle of the board is assigned a tile type to force its presence
	//grid[sideLength * 2 - 1 + (sideLength - 1) * sideLength * 4 + sideLength * sideLength * 8] = cellOption_t({ trackOptions });

}

WaveFunctionCollapse::WaveFunctionCollapse(const WaveFunctionCollapse& old)
{
	this->sideLength = old.sideLength;
	this->height = old.height;
	this->typeCount = old.typeCount;
	this->constraints = std::vector<std::shared_ptr<Constraint>>(old.constraints);

	for (cellOption_t options : old.grid)
	{
		this->grid.push_back(options);
	}
}

WaveFunctionCollapse::~WaveFunctionCollapse()
{

}

int WaveFunctionCollapse::getWeakest()
{
	//////std::cout << typeCount << std::endl;
	int lowestOptionCount = typeCount + 1;
	int lowestOptionIndex = -1;

	for (size_t i = 0; i < grid.size(); i++)
	{
		int optionCount = grid[i].size();
		//if(optionCount == 0)
		////std::cout << optionCount << std::endl;
		////std::cout << optionCount << std::endl;

		//No possibilities is managed sooner.
		if (optionCount == 0)
		{
			//std::cout << "died" << std::endl;
			return i;
		}

		//A cell has a lower amount of possible states, but can still branch (single state cells should be ignored).
		if (optionCount < lowestOptionCount && optionCount > 1)
		{
			////std::cout << lowestOptionCount << "->" << optionCount << std::endl;
			lowestOptionCount = optionCount;
			lowestOptionIndex = i;
		}
	}
	////std::cout << "did the count" << std::endl;

	return lowestOptionIndex;
}

const cellOption_t& WaveFunctionCollapse::getTileOptions(int index)
{
	return grid[index];
}

std::shared_ptr<WaveFunctionCollapse> WaveFunctionCollapse::branch(int index, const std::shared_ptr<GridTile>& option)
{
	std::vector<std::shared_ptr<WaveFunctionCollapse>> potentialStates;

	//There is no cell to collapse; It's finished
	if (index < 0)
	{
		return nullptr;
	}

	//Usual case: We branch out for the next iteration to be called

	WaveFunctionCollapse state(*this);
	//////std::cout << "Picking an option" << std::endl;
	state.selectTileOption(index, option);
	cellOption_t ownOptions;

	for (std::shared_ptr<GridTile> ownOption : grid[index])
	{
		if (ownOption != option)
		{
			ownOptions.push_back(ownOption);
		}
	}
	grid[index] = ownOptions;
	return std::make_shared<WaveFunctionCollapse>(state);
}

void WaveFunctionCollapse::selectTileOption(int index, const std::shared_ptr<GridTile>& option)
{
	grid[index] = cellOption_t({ option });
}

void WaveFunctionCollapse::addConstraint(std::shared_ptr<Constraint> constraint)
{
	constraints.push_back(constraint);
}

int WaveFunctionCollapse::getNeighbourIndex(int position, Direction direction)
{
	int zMult = (sideLength * 4);
	int zToYMult = (sideLength * 2);
	int yMult = zToYMult * zMult;

	int x = position % zMult;
	int z = (position / zMult) % zToYMult;
	int y = (position / yMult);
	//Triangles alternate orientation, true if down
	bool flip = position % 2;

	switch (direction)
	{
	case side60:
		if (flip)
		{
			if (x + 1 >= zMult)
			{
				return -1;
			}
			return (x + 1) + z * zMult + y * yMult;
		}
		else
		{
			if (x - 1 < 0)
			{
				return -1;
			}
			return (x - 1) + z * zMult + y * yMult;
		}
	case side180:
		if (flip)
		{
			if (z - 1 < 0 || x - 1 < 0)
			{
				return -1;
			}
			return (x - 1) + (z - 1) * zMult + y * yMult;
		}
		else
		{
			if (z + 1 >= zToYMult || x + 1 >= zMult)
			{
				return -1;
			}
			return (x + 1) + (z + 1) * zMult + y * yMult;
		}
		break;
	case side300:
		if (flip)
		{
			if (x - 1 < 0)
			{
				return -1;
			}
			return (x - 1) + z * zMult + y * yMult;
		}
		else
		{
			if (x + 1 >= zMult)
			{
				return -1;
			}
			return (x + 1) + z * zMult + y * yMult;
		}
		break;
	case up:
		if (y + 1 >= height)
		{
			return -1;
		}
		return x + z * zMult + (y + 1) * yMult;
	case down:
		if (y - 1 < 0)
		{
			return -1;
		}
		return x + z * zMult + (y - 1) * yMult;
	case none:
		return -1;
	default:
		return -1;
	}
}

bool WaveFunctionCollapse::reduceTile(int index, Direction source)
{
	//if out of bounds, ignore
	if (index < 0 || index >= grid.size())
	{
		return true;
	}

	int zMult = (sideLength * 4);
	int zToYMult = (sideLength * 2);
	int yMult = zToYMult * zMult;

	int x = index % zMult;
	int z = (index / zMult) % zToYMult;
	int y = (index / yMult);
	//Triangles alternate orientation, true if down
	bool flip = index % 2;


	int optionCount = grid[index].size();
	cellOption_t remainingOptions;
	//std::cout << index << ": /" << optionCount << " - " << source << std::endl;

	if (source != none)
	{
		for (std::shared_ptr<GridTile> tile : grid[index])
		{
			bool foundOption = false;

			for (std::shared_ptr<Constraint> constraint : constraints)
			{


				if (tile->id != constraint->thisTile)
				{
					continue;
				}

				int sourcePosition = getNeighbourIndex(index, source);

				if (sourcePosition == -1)
				{
					continue;
				}

				cellOption_t options = grid[sourcePosition];

				switch (source)
				{
				case side60:
				case side180:
				case side300:
				{
					Direction adjacencyEdge = static_cast<Direction>((source + 2 * tile->orientation) % 3);

					if (constraint->otherAdjacency != adjacencyEdge)
					{
						continue;
					}
					break;
				}
				case up:
				case down:
					if (constraint->otherAdjacency != source)
					{
						continue;
					}
					break;
				default:
					break;
				}

				for (std::shared_ptr<GridTile> option : options)
				{
					if (option->id != constraint->otherTile)
					{
						continue;
					}

					//Use 2*n instead of -n to prevent negative modulo jank
					int relativeRotation = (option->orientation + 2 * tile->orientation) % 3;

					if (relativeRotation == constraint->otherTileOrientation)
					{
						//Add the tile as one that is still possible
						remainingOptions.push_back(tile);
						foundOption = true;
						break;
					}
				}
				if (foundOption)
				{
					break;
				}
			}
		}

		if (remainingOptions.size() < 1)
		{
			return false;
		}
		grid[index] = remainingOptions;
	}

	if (grid[index].size() >= optionCount && source != none)
	{
		return true;
	}

	for (int i = 0; i < 5; i++)
	{
		Direction opposite;

		switch (i)
		{
		case side60:
		case side180:
		case side300:
			opposite = static_cast<Direction>(i);
			break;
		case up:
			opposite = down;
			break;
		case down:
			opposite = up;
			break;
		default:
			opposite = none;
			break;
		}

		if (source == opposite)
		{
			continue;
		}

		int target = getNeighbourIndex(index, static_cast<Direction>(i));

		if (target == -1)
		{
			continue;
		}

		if (!reduceTile(target, opposite))
		{
			return false;
		}
	}

	return true;
}

void WaveFunctionCollapse::renderKnowns(ObjectManager objectManager, std::shared_ptr<std::vector<std::vector<std::string>>> fileNames)
{
	float xOff = sideLength * sqrt(3.0f) / 2.0f;
	float zOff = sideLength * 3.0f / 2.0f;

	std::vector<int> knownTrackIndices;

	for (size_t i = 0; i < grid.size(); i++)
	{
		if (grid[i].size() != 1)
		{
			continue;
		}

		std::shared_ptr<GridTile> tile = grid[i][0];

		std::vector<std::string> files = (*fileNames)[tile->id];

		//set up 3d array coordinates
		int zMult = (sideLength * 4);
		int zToYMult = (sideLength * 2);
		int yMult = zToYMult * zMult;

		int x = i % zMult;
		int z = (i / zMult) % zToYMult;
		int y = (i / yMult);
		//Triangles alternate orientation, true if down
		bool flip = i % 2;

		float xPos = (x - z) * sqrt(3.0f) / 2.0f - xOff;
		float zPos = z * 3.0f / 2.0f + (flip ? -1 : 1) * (1.0f / 4.0f) - zOff;
		float yPos = y * 2;

		glm::vec3 tilePosition(xPos, yPos, zPos);

		for (std::string file : files)
		{
			objectManager.addEnvironmentObject(file, tilePosition, glm::vec3(0, glm::radians((float)(tile->orientation * 120 + 180 * flip)), 0));
		}

		std::vector<SpecialProperty> properties = tile->properties;
		std::vector<SpecialProperty> trackNodeProperties;

		for (SpecialProperty trackProperty : tile->properties)
		{
			switch (trackProperty)
			{
			case trackNode60:
			case trackNode180:
			case trackNode300:
				trackNodeProperties.push_back(trackProperty);
				break;
			default:
				break;
			}
		}

		//no track properties or already familiar can be skipped
		if (trackNodeProperties.size() == 0 || std::find(knownTrackIndices.begin(), knownTrackIndices.end(), i) != knownTrackIndices.end())
		{
			continue;
		}

		bool trackDone = false;

		std::vector<std::vector<glm::vec3>> coordinateChains;

		knownTrackIndices.push_back(i);
		for (SpecialProperty trackNodeProperty : trackNodeProperties)
		{
			std::vector<glm::vec3> coordinateChain;
			int index = i;

			switch (trackNodeProperty)
			{
			case trackNode60:
				index = getNeighbourIndex(index, side60);
				break;
			case trackNode180:
				index = getNeighbourIndex(index, side180);
				break;
			case trackNode300:
				index = getNeighbourIndex(index, side300);
				break;
			default:
				break;
			}

			SpecialProperty oldProperty = trackNodeProperty;

			while (index != i && index != -1)
			{
				std::cout << "Start: " << oldProperty << std::endl;

				x = index % zMult;
				z = (index / zMult) % zToYMult;
				y = (index / yMult);
				flip = index % 2;
				xPos = (x - z) * sqrt(3.0f) / 2.0f - xOff;
				zPos = z * 3.0f / 2.0f + (flip ? -1 : 1) * (1.0f / 4.0f) - zOff;
				yPos = y * 2;

				glm::vec3 position(xPos, yPos, zPos);

				coordinateChain.push_back(position);
				knownTrackIndices.push_back(index);

				bool changed = false;

				for (SpecialProperty newTrackProperty : grid[index][0]->properties)
				{
					//no backtracking
					if (newTrackProperty == oldProperty)
					{
						continue;
					}

					switch (newTrackProperty)
					{
					case trackNode60:
						index = getNeighbourIndex(index, side60);
						changed = true;
						break;
					case trackNode180:
						index = getNeighbourIndex(index, side180);
						changed = true;
						break;
					case trackNode300:
						index = getNeighbourIndex(index, side300);
						changed = true;
						break;
					default:
						continue;
					}

					oldProperty = newTrackProperty;

					std::cout << "Next: " << oldProperty << std::endl;

					break;
				}

				if (!changed)
				{
					break;
				}
			}

			if (index == i)
			{
				coordinateChain.push_back(tilePosition);
				//loop
				objectManager.addTrackPath("models/wfc_track_platform.obj", coordinateChain, glm::vec3(0), true, false);
				trackDone = true;
				break;
			}

			coordinateChains.push_back(coordinateChain);
		}

		if (trackDone)
		{
			continue;
		}

		//no need to think about ref copy stuff
		std::vector<glm::vec3> fullChain = std::vector<glm::vec3>(coordinateChains[0]);
		std::reverse(fullChain.begin(), fullChain.end());
		std::cout << fullChain.size() << std::endl;
		fullChain.push_back(tilePosition);
		std::cout << fullChain.size() << std::endl;
		if (coordinateChains.size() != 1)
		{
			for (glm::vec3 chainCoord : coordinateChains[1])
			{
				fullChain.push_back(chainCoord);
			}
		}
		objectManager.addTrackPath("models/wfc_track_platform.obj", fullChain, glm::vec3(0), false, false);
	}
}

std::shared_ptr<WaveFunctionCollapse> runWaveFunctionCollapse(std::shared_ptr<WaveFunctionCollapse> wfc)
{
	//std::cout << "Initial reduction 1: " << (wfc->reduceTile(0, none) ? "succeeded" : "failed") << std::endl;
	//std::cout << "Initial reduction 2: " << (wfc->reduceTile(wfc->sideLength * 2 - 1 + (wfc->sideLength - 1) * wfc->sideLength * 4, none) ? "succeeded" : "failed") << std::endl;

	std::stack<std::shared_ptr<WaveFunctionCollapse>> calculationStack;
	calculationStack.push(wfc);

	srand(time(NULL));

	while (!calculationStack.empty())
	{
		std::shared_ptr<WaveFunctionCollapse> branch = calculationStack.top();

		int reducedIndex = branch->getWeakest();
		//std::cout << "Index " << reducedIndex << std::endl;
		if (reducedIndex == -1)
		{
			return branch;
		}

		cellOption_t options = branch->getTileOptions(reducedIndex);
		int optionCount = options.size();
		//std::cout << optionCount << " options available for cell " << reducedIndex << std::endl;
		if (optionCount == 0)
		{
			continue;
		}

		int pick = rand() % optionCount;
		std::shared_ptr<WaveFunctionCollapse> newBranch = branch->branch(reducedIndex, options[pick]);

		//discard the alternative if that fails, before a potential push to the stack
		if (!branch->reduceTile(reducedIndex, none))
		{
			calculationStack.pop();
		}

		if (newBranch->reduceTile(reducedIndex, none))
		{
			calculationStack.push(newBranch);
		}
		else
		{
			//std::cout << "Backtracking..." << std::endl;
		}
	}

	return nullptr;
}


std::shared_ptr<WaveFunctionCollapse> initialiseWFC(int width, int height, const std::string& fileName, std::shared_ptr<std::vector<std::vector<std::string>>> objectFiles) {


	std::cout << "Loading Environment from " << fileName << std::endl;
	std::ifstream pFile(fileName.c_str());
	if (!pFile.is_open())
	{
		std::cout << "Could not open file " << fileName << std::endl;
		return nullptr;
	}

	std::vector<int> horizontalLinks;
	std::vector<int> verticalCycles;

	int tileIdCounter = 0;
	cellOption_t gridTiles;
	std::vector<DataTile> tiles;

	while (!pFile.eof())
	{
		std::string line;
		std::getline(pFile, line);
		line = cleanLine(line);
		if (line == "" || line[0] == '#') //skip empty or commented line
			continue;
		std::vector<std::string> params = split(line, " ");
		params[0] = toLower(params[0]);



		if (params[0] == "l")
		{
			int horizontalLinkCount = atoi(params[1].c_str());
			int verticalCycleCount = atoi(params[2].c_str());
			for (size_t i = 0; i < horizontalLinkCount; i++)
			{
				horizontalLinks.push_back(i);
			}
			for (size_t i = 0; i < verticalCycleCount; i++)
			{
				verticalCycles.push_back(i);
			}
		}

		else if (params[0] == "h")
		{
			std::vector<std::string> swapPositions = split(params[1], ",");
			if (swapPositions.size() != 2)
			{
				continue;
			}

			std::vector<int> swapInts;

			for (std::string swapPosition : swapPositions)
			{
				int swapInt = atoi(swapPosition.c_str());
				swapInts.push_back(swapInt);
			}

			for (size_t i = 0; i < swapInts.size(); i++)
			{
				horizontalLinks[swapInts[i]] = swapInts[(i + 1) % swapInts.size()];
			}
		}

		else if (params[0] == "v")
		{
			std::vector<std::string> cyclePositions = split(params[1], ",");

			std::vector<int> cycleInts;

			for (std::string cyclePosition : cyclePositions)
			{
				int cycleInt = atoi(cyclePosition.c_str());
				cycleInts.push_back(cycleInt);
			}

			for (size_t i = 0; i < cycleInts.size(); i++)
			{
				verticalCycles[cycleInts[i]] = cycleInts[(i + 1) % cycleInts.size()];
			}
		}

		else if (params[0] == "ts" || params[0] == "tr")
		{
			int rotationCount = params[0] == "tr" ? 3 : 1;
			std::vector<int> storedFacetype;

			std::vector<std::string> facetypes = split(params[1], ",");
			for (std::string facetype : facetypes)
			{
				storedFacetype.push_back(atoi(facetype.c_str()));
			}

			std::vector<std::string> files;
			for (size_t i = 2; i < params.size(); i++)
			{
				files.push_back(params[i]);
			}
			objectFiles->push_back(files);

			for (int i = 0; i < rotationCount; i++)
			{
				if (rotationCount == 3)
				{
					DataTile tile{ std::make_shared<GridTile>(GridTile{ tileIdCounter, static_cast<Orientation>(i), std::vector<SpecialProperty>() }), std::vector<int>() };

					gridTiles.push_back(tile.type);

					for (int facetype : storedFacetype)
					{
						tile.faces.push_back(facetype);
					}

					tiles.push_back(tile);
				}
				else
				{
					for (int j = 0; j < 3; j++)
					{
						DataTile tile{ std::make_shared<GridTile>(GridTile{ tileIdCounter, static_cast<Orientation>(j), std::vector<SpecialProperty>() }), std::vector<int>() };

						for (int facetype : storedFacetype)
						{
							tile.faces.push_back(facetype);
						}

						if (j == 0)
						{
							gridTiles.push_back(tile.type);
						}

						tiles.push_back(tile);
					}
				}



				if (i + 1 >= rotationCount)
					continue;
				//cycle horizontal
				int cycleMemory = storedFacetype[2];
				storedFacetype[2] = storedFacetype[1];
				storedFacetype[1] = storedFacetype[0];
				storedFacetype[0] = cycleMemory;
				//rotate vertical
				for (size_t j = 3; j < 5; j++)
				{
					storedFacetype[j] = verticalCycles[storedFacetype[j]];
				}
			}

			tileIdCounter++;
		}

		else if (params[0] == "s")
		{
			tileId_t tileId = atoi(params[1].c_str());

			for (DataTile tile : tiles)
			{
				if (tile.type->id == tileId)
				{

					if (params[2] == "empty")
					{
						tile.type->properties.push_back(emptyTile);
					}
					else if (params[2] == "full")
					{
						tile.type->properties.push_back(fullTile);
					}
					else if (params[2] == "track")
					{
						SpecialProperty trackProperties[3] = { trackNode60, trackNode180, trackNode300 };

						for (size_t i = 3; i < params.size(); i++)
						{
							tile.type->properties.push_back(trackProperties[(atoi(params[i].c_str()) + tile.type->orientation) % 3]);
						}
					}
				}
			}
		}
	}

	WaveFunctionCollapse collapse = WaveFunctionCollapse(width, height, gridTiles);

	//generating constraints
	for (size_t i = 0; i < tiles.size(); i++)
	{
		//for the first tile, only consider standard orientation
		if ((tiles[i].type).get()->orientation)
		{
			continue;
		}
		for (size_t j = 0; j < tiles.size(); j++)
		{
			//horizontal faces
			for (size_t f = 0; f < 3; f++)
			{
				if (horizontalLinks[tiles[i].faces[f]] == tiles[j].faces[f])
				{
					std::cout << (tiles[i].type).get()->id << "," << (tiles[j].type).get()->id << "," << static_cast<Direction>(f) << "," << (tiles[j].type).get()->orientation << std::endl;
					collapse.addConstraint(std::make_shared<Constraint>(Constraint{ (tiles[i].type).get()->id, (tiles[j].type).get()->id, static_cast<Direction>(f), (tiles[j].type).get()->orientation }));
				}
			}

			for (size_t f = 3; f < 5; f++)
			{
				//7-x swaps 3 and 4
				if (tiles[i].faces[f] == tiles[j].faces[7 - f])
				{
					std::cout << (tiles[i].type).get()->id << "," << (tiles[j].type).get()->id << "," << static_cast<Direction>(f) << "," << (tiles[j].type).get()->orientation << std::endl;
					collapse.addConstraint(std::make_shared<Constraint>(Constraint{ (tiles[i].type).get()->id, (tiles[j].type).get()->id, static_cast<Direction>(f), (tiles[j].type).get()->orientation }));
				}
			}
		}
	}

	return std::make_shared<WaveFunctionCollapse>(collapse);
}