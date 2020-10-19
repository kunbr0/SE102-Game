#include "GameMap2.h"

CGameMap::CGameMap()
{

}

CGameMap::CGameMap(int width, int height, int tileWidth, int tileHeight)
{
	this->width = width;
	this->height = height;
	this->tileHeight = tileHeight;
	this->tileWidth = tileWidth;
}

void CGameMap::UpdateCamPosition(Vector2 newPos) {
	camPosition = newPos;
}

Vector2 CGameMap::ConvertToPositionInCam(Vector2 oldPos) {
	return Vector2(oldPos.x - camPosition.x, oldPos.y - camPosition.y);
}

Vector2 CGameMap::GetBound()
{
	return Vector2(this->width * tileWidth, this->height * tileHeight);
}

shared_ptr<CTileSet> CGameMap::GetTileSetByTileID(int id)
{
	/*return floor_entry(tilesets, id).second;*/

	return tilesets[1];
}

void CGameMap::AddTileSet(int firstgid, shared_ptr<CTileSet> tileSet)
{
	this->tilesets[firstgid] = tileSet;
}

void CGameMap::AddLayer(shared_ptr<CMapLayer> layer)
{
	this->layers.push_back(layer);
}

void CGameMap::Update(int dt)
{
}

void CGameMap::Render()
{
	/*int col = this->camera->Position.x / tileWidth;
	int row = this->camera->Position.y / tileHeight;*/
	int col = 18;
	int row = 22;


	if (col > 0) col--;
	if (row > 0) row--;

	Vector2 camSize = Vector2(640 / tileWidth, 640 / tileHeight);

	for (int i = col; i < camSize.x + col + 2; i++) {
		for (int j = row; j < camSize.y + row + 2; j++) {

			/*int x = i * tileWidth - this->camera->Position.x;
			int y = j * tileHeight - this->camera->Position.y;*/
			int x = i * tileWidth - 50;
			int y = j * tileHeight - 800;

			for (shared_ptr<CMapLayer> layer : layers) {
				if (layer->Hidden) continue;
				int id = layer->GetTileID(i % width, j % height);
				auto abc = this->GetTileSetByTileID(id);
				abc->Draw(id, ConvertToPositionInCam(Vector2(x,y)));
			}
		}
	}
}

shared_ptr<CGameMap> CGameMap::FromTMX(string filePath)
{
	string fullPath = filePath;
	TiXmlDocument doc(fullPath.c_str());

	if (doc.LoadFile()) {
		TiXmlElement* root = doc.RootElement();
		shared_ptr<CGameMap> gameMap = shared_ptr<CGameMap>(new CGameMap());

		root->QueryIntAttribute("width", &gameMap->width);
		root->QueryIntAttribute("height", &gameMap->height);
		root->QueryIntAttribute("tilewidth", &gameMap->tileWidth);
		root->QueryIntAttribute("tileheight", &gameMap->tileHeight);

		//Load tileset
		
		for (TiXmlElement* node = root->FirstChildElement("tileset"); node != nullptr; node = node->NextSiblingElement("tileset")) {
			CTileSet* tileSet = new CTileSet(node, filePath);
			auto abc = tileSet->GetFirstGID();
			gameMap->tilesets[tileSet->GetFirstGID()] = shared_ptr<CTileSet>(tileSet);
		}

		//Load layer
		for (TiXmlElement* node = root->FirstChildElement("layer"); node != nullptr; node = node->NextSiblingElement("layer")) {
			shared_ptr<CMapLayer> layer = shared_ptr<CMapLayer>(new CMapLayer(node));
			gameMap->AddLayer(layer);
		}

		return gameMap;
	}

	throw "Load map that bai!!";
}

CGameMap::~CGameMap()
{
	layers.clear();
	tilesets.clear();
}
