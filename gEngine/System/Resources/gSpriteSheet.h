
#pragma once

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <typeindex>
#include <type_traits>
#include "..\..\Useful.h"

namespace GENG
{
	namespace Resources
	{
		class gSpriteSheet
		{
		protected:
			struct xmlSprite
			{
				std::string m_name;
				std::string m_bounds;
			};
			struct xmlSheet
			{
				std::string m_name;
				std::string m_imagePath;
				std::vector<xmlSprite> m_sprites;
			};
			typedef std::unique_ptr<SDL_Surface, std::function<void(SDL_Surface*)>> tySpriteImage;
			typedef std::map<std::string, std::map<int, Vec2[2]>> tySpriteData;
		protected:
			std::string m_name;
			tySpriteImage m_pImage;
			tySpriteData m_sprites;

		protected:
			static void _getXMLData(const std::string & xmlData, 
				xmlSheet & sheet)
			{
				//////////////////////////////////////////////////////////////////////////
				// Load XML Data
				rapidxml::xml_document<> xmlSpriteSheet;
				xmlSpriteSheet.parse<0>(const_cast<char*>(xmlData.c_str()));
				auto root = xmlSpriteSheet.first_node("sheet");
				sheet.m_name = root->first_attribute("name")->value();
				sheet.m_imagePath = root->first_attribute("asset")->value();

				for (auto node = root->first_node(); node; node = node->next_sibling())
				{
					xmlSprite sprite;
					sprite.m_name = node->first_attribute("name")->value();
					sprite.m_bounds = node->first_attribute("bounds")->value();
					sheet.m_sprites.push_back(sprite);
				}
			}
			static void _getSpriteData(const xmlSheet & sheetData, 
				std::string & sheetName, tySpriteImage & spriteImage, tySpriteData & spriteData)
			{
				sheetName = sheetData.m_name;

				// Load the master image
				spriteImage = std::unique_ptr<SDL_Surface, std::function<void(SDL_Surface*)>>(
					SDL_LoadBMP(sheetData.m_imagePath.c_str()), [](SDL_Surface * f) { SDL_FreeSurface(f); });

				// Get sprite bounds
				for (auto sprite : sheetData.m_sprites)
				{
					auto sPair = _getStringOrder(sprite.m_name);
					std::string sName = sPair.first;
					int sNum = sPair.second;

					auto points = _getDelimited(sprite.m_bounds);
					std::map<int, Vec2[2]> & end = spriteData[sName];
					end[sNum][0].x = std::atof(points[0].c_str());
					end[sNum][0].y = std::atof(points[1].c_str());
					end[sNum][1].x = std::atof(points[2].c_str());
					end[sNum][1].y = std::atof(points[3].c_str());
				}
			}
		public:
			gSpriteSheet(){ DBGINIT; };
			gSpriteSheet(const std::string filePath) { load(filePath); };
			~gSpriteSheet(){ DBGDEST; };

			void load(const std::string & filePath)
			{										
				xmlSheet sheet;
				_getXMLData(_getFile(filePath), sheet);
				_getSpriteData(sheet, m_name, m_pImage, m_sprites);
			}
		};
	}
};