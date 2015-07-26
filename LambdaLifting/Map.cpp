//
// Map
//

#include "stdafx.h"
#include "Map.h"

namespace app
{

	//-----------------------------------------------------------------------------
	//! CommandÇï∂éöÇ…
	//-----------------------------------------------------------------------------
	s3d::wchar charOfCommand(Command cmd)
	{
		return static_cast<s3d::wchar>(cmd);
	}

	//-----------------------------------------------------------------------------
	//! ï∂éöÇCommandÇ…
	//-----------------------------------------------------------------------------
	Command commandOfChar(s3d::wchar c)
	{
		switch (c) {
		case L'U':
		case L'D':
		case L'L':
		case L'R':
		case L'W':
		case L'A':
		case L'S':
			return static_cast<Command>(c);
		}
		return Command::None;
	}

	//-----------------------------------------------------------------------------
	//! CellÇï∂éöóÒÇ…
	//-----------------------------------------------------------------------------
	const s3d::wchar* stringOfCell(Cell cell)
	{
		switch (cell) {
		case Cell::Empty:		return L"Å@";
		case Cell::Robot:		return L"Çq";
		case Cell::Rock:		return L"Åñ";
		case Cell::Lambda:		return L"Å_";
		case Cell::Earth:		return L"ÅE";
		case Cell::Wall:		return L"Åî";
		case Cell::ClosedLift:	return L"Çk";
		case Cell::OpenLift:	return L"Çn";

		case Cell::TrampolineA:	return L"Ç`";
		case Cell::TrampolineB:	return L"Ça";
		case Cell::TrampolineC:	return L"Çb";
		case Cell::TrampolineD:	return L"Çc";
		case Cell::TrampolineE:	return L"Çd";
		case Cell::TrampolineF:	return L"Çe";
		case Cell::TrampolineG:	return L"Çf";
		case Cell::TrampolineH:	return L"Çg";
		case Cell::TrampolineI:	return L"Çh";

		case Cell::Target1:		return L"ÇP";
		case Cell::Target2:		return L"ÇQ";
		case Cell::Target3:		return L"ÇR";
		case Cell::Target4:		return L"ÇS";
		case Cell::Target5:		return L"ÇT";
		case Cell::Target6:		return L"ÇU";
		case Cell::Target7:		return L"ÇV";
		case Cell::Target8:		return L"ÇW";
		case Cell::Target9:		return L"ÇX";

		case Cell::Beard:		return L"Çv";
		case Cell::Razor:		return L"ÅI";

		case Cell::HORock:		return L"Åó";
		}
		return L"ÅH";
	}

	//-----------------------------------------------------------------------------
	//! ConditionÇï∂éöóÒÇ…
	//-----------------------------------------------------------------------------
	const s3d::wchar* stringOfCondition(Condition cond)
	{
		switch (cond) {
		case Condition::Playing:	return L"Playing";
		case Condition::Winning:	return L"Winning";
		case Condition::Abort:		return L"Abort";
		case Condition::Losing:		return L"Losing";
		}
		return L"Unknown";
	}


	//-----------------------------------------------------------------------------
	//! ÉNÉäÉA
	//-----------------------------------------------------------------------------
	void Map::clear()
	{
		cell.clear();
		robotPos.set(0, 0);
		lambda = 0;
		lambdaCollected = 0;
		stepCount = 0;
		score = 0;
		condition = Condition::Playing;

		water = 0;
		floodingCount = 0;
		waterproofCount = 0;

		growthCount = 0;
		razor = 0;
		beard = 0;
	}


	//-----------------------------------------------------------------------------
	//! ÉNÉäÉA
	//-----------------------------------------------------------------------------
	void MapInfo::clear()
	{
		liftPos.set(0, 0);

		flooding = 0;
		waterproof = 10;

		growth = 25;

		std::memset(trampolinePos, 0, sizeof(trampolinePos));
		std::memset(targetPos, 0, sizeof(targetPos));
		std::fill(jump, jump + sizeof(jump), 0xFF);
	}

}
