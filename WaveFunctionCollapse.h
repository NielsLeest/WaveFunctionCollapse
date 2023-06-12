#pragma once

#include <gl/glew.h>
#include <string>
#include <memory>
#include "ModelComponent.h"
#include "ObjectManager.h"

//angles are counterclockwise from the orthogonally aligned corner
//Sides and triangles are implied to be rotated 180 degrees when flipped
enum Direction { side60 = 0, side180 = 1, side300 = 2, up = 3, down = 4, none = 5 };

enum Orientation { rotation0 = 0, rotation120 = 1, rotation240 = 2 };

enum SpecialProperty { emptyTile = 0, fullTile = 1, trackNode60 = 2, trackNode180 = 3, trackNode300 = 4 };

typedef int tileId_t;

struct Constraint
{
	tileId_t thisTile;
	tileId_t otherTile;
	Direction otherAdjacency;
	Orientation otherTileOrientation;
};

struct GridTile
{
	tileId_t id;
	Orientation orientation;
	std::vector<SpecialProperty> properties;
};

struct DataTile
{
	std::shared_ptr<GridTile> type;
	std::vector<int> faces;
};

typedef std::vector<std::shared_ptr<GridTile>> cellOption_t;

class WaveFunctionCollapse
{
private:
	std::vector<std::shared_ptr<Constraint>> constraints;
	int typeCount;
	//every cell has some number of options
	std::vector<cellOption_t> grid;

	int getNeighbourIndex(int position, Direction direction);
public:
	int sideLength;
	int height;

	WaveFunctionCollapse(int sideLength, int height, const cellOption_t& options);
	WaveFunctionCollapse(const WaveFunctionCollapse& old);
	~WaveFunctionCollapse();

	void selectTileOption(int index, const std::shared_ptr<GridTile>& option);
	const cellOption_t& getTileOptions(int index);
	void addConstraint(std::shared_ptr<Constraint> constraint);

	/// <summary>
	/// Locates the tile with the lowest amount of options
	/// </summary>
	/// <returns>The index of an undetermined tile with the lowest amount of options, or -1 if no such tile exists (the collapse was successful)</returns>
	int getWeakest();

	/// <summary>
	/// Gives all branches from a certain index
	/// </summary>
	/// <returns>All options wave functions for continuation. Empty means the branch is broken.</returns>
	std::shared_ptr<WaveFunctionCollapse> branch(int index, const std::shared_ptr<GridTile>& option);

	/// <summary>
	/// Reduces a tile by examining its neighbours. To be used whenever a neighbour has collapsed partially or fully
	/// </summary>
	/// <param name="index">The tile to reduce</param>
	/// <param name="source">The direction from which the reduction was called</param>
	bool reduceTile(int index, Direction source);

	void renderKnowns(ObjectManager objectManager, std::shared_ptr<std::vector<std::vector<std::string>>> fileNames);
};

std::shared_ptr<WaveFunctionCollapse> runWaveFunctionCollapse(std::shared_ptr<WaveFunctionCollapse> wfc);

std::shared_ptr<WaveFunctionCollapse> initialiseWFC(int width, int height, const std::string& fileName, std::shared_ptr<std::vector<std::vector<std::string>>> objectFiles);