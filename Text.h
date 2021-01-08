#pragma once

#include "UI.h"

class Text : public UI {
public:
	void InitFont();	//Calculates and stores the location of each character in the font.dds.  Use this before using the main LoadTextures function in CubeGame.cpp.

	//Getters/Setters
	void SetString(std::string str, float posX, float posY);		//Sets a string. The positions are normalized (between 0 and 1f)
	Font* GetFont() { return &mFnt; };

private:
	void SetChar(char character, int position, std::vector<GeometryGenerator::Vertex>& vertices);	//Sets a single character. Called from the SetString function

private:
	Font mFnt;
};