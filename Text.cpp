#include "Text.h"

void Text::InitFont() {
	mFnt.filePath = L"data/font.dds";
	//1 / number of rows or columns
	float size = 1.f / 9.f;

	int rows = 3;
	int cols = 9;
	int row = 0;
	int col = 0;

	//Capitals
	for (int i = 65; i <= 65 + 25; i++) {
		char c = (char)i;

		Font::myChar temp(col * size, row * size, size, size);
		mFnt.chars[c] = temp;

		col++;
		if (col >= cols) {
			col = 0;
			row++;
		}
	}

	//Lower case
	row = 3;
	col = 0;
	for (int i = 97; i <= 97 + 25; i++) {
		char c = (char)i;

		Font::myChar temp(col * size, row * size, size, size);
		mFnt.chars[c] = temp;

		col++;
		if (col >= cols) {
			col = 0;
			row++;
		}
	}

	//Space
	Font::myChar temp(col * size, row * size, size, size);
	mFnt.chars[' '] = temp;

	//numbers
	row = 6;
	col = 0;
	for (int i = 48; i <= 48 + 9; i++) {
		char c = (char)i;

		Font::myChar temp(col * size, row * size, size, size);
		mFnt.chars[c] = temp;

		col++;
		if (col >= cols) {
			col = 0;
			row++;
		}
	}

	//Specials
	row = 7;
	col = 1;
	char spec[] = { '.', ',', ';', ':', '$', '#', '"', '!', '"', '/', '?', '%', '&', '(', ')', '@' };
	for (int i = 0; i <= 16 + 9; i++) {

		Font::myChar temp(col * size, row * size, size, size);
		mFnt.chars[spec[i]] = temp;

		col++;
		if (col >= cols) {
			col = 0;
			row++;
		}
	}


}

void Text::SetChar(char c, int p, std::vector<GeometryGenerator::Vertex>& vs) {
	//Skip every other position
	int pos = p + p;				//Skip every other colomn
	int row = (p / (mSizeX / 2));	//Calculate the row number
	row *= mSizeX;					//Multiply the row number by the size of each row
	pos += row;						//Add the x and y positions

	//If there aren't enough spaces, don't set the char
	if (pos >= vs.size()) return;

	//Set a square of texture coords
	vs[pos].TexC = { mFnt.chars.at(c).posX, mFnt.chars.at(c).posY };
	vs[pos + 1].TexC = { mFnt.chars.at(c).posX + mFnt.chars.at(c).width, mFnt.chars.at(c).posY };
	vs[pos + mSizeX].TexC = { mFnt.chars.at(c).posX, mFnt.chars.at(c).posY + mFnt.chars.at(c).height };
	vs[pos + mSizeX + 1].TexC = { mFnt.chars.at(c).posX + mFnt.chars.at(c).width, mFnt.chars.at(c).posY + mFnt.chars.at(c).height };
}

void Text::SetString(std::string str, float posX, float posY) {
	//Make sure the position isn't outside plane
	if (posX > 1.f) posX = 1.f;
	if (posY > 1.f) posY = 1.f;

	//Calculate the starting position
	posX *= mSizeX * 0.5f;
	posY *= mSizeY * 0.5f;
	posY *= mSizeX * 0.5f;

	int pos = (int)(posX + posY);

	//Change the texture coords of each sub-square on the UI plane to match those in the font sprite map
	for each (char c in str) {
		if (c == '-') c = 'm';
		SetChar(c, pos, mVertices);
		pos++;
	}

	//Update the CPUs version of the verticies
	CopyMemory(mRI->Geo->VertexBufferCPU->GetBufferPointer(), mVertices.data(), mVbByteSize);

	//Make sure the render item is updated on each Frame Resource	
	SetDirtyFlag();
}