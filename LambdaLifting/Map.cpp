//
// Map
//

#include "stdafx.h"
#include "Map.h"

namespace app
{

	//-----------------------------------------------------------------------------
	//! Command�𕶎���
	//-----------------------------------------------------------------------------
	s3d::wchar charOfCommand(Command cmd)
	{
		return static_cast<s3d::wchar>(cmd);
	}

	//-----------------------------------------------------------------------------
	//! ������Command��
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
	//! Cell�𕶎����
	//-----------------------------------------------------------------------------
	const s3d::wchar* stringOfCell(Cell cell)
	{
		switch (cell) {
		case Cell::Empty:		return L"�@";
		case Cell::Robot:		return L"�q";
		case Cell::Rock:		return L"��";
		case Cell::Lambda:		return L"�_";
		case Cell::Earth:		return L"�E";
		case Cell::Wall:		return L"��";
		case Cell::ClosedLift:	return L"�k";
		case Cell::OpenLift:	return L"�n";

		case Cell::TrampolineA:	return L"�`";
		case Cell::TrampolineB:	return L"�a";
		case Cell::TrampolineC:	return L"�b";
		case Cell::TrampolineD:	return L"�c";
		case Cell::TrampolineE:	return L"�d";
		case Cell::TrampolineF:	return L"�e";
		case Cell::TrampolineG:	return L"�f";
		case Cell::TrampolineH:	return L"�g";
		case Cell::TrampolineI:	return L"�h";

		case Cell::Target1:		return L"�P";
		case Cell::Target2:		return L"�Q";
		case Cell::Target3:		return L"�R";
		case Cell::Target4:		return L"�S";
		case Cell::Target5:		return L"�T";
		case Cell::Target6:		return L"�U";
		case Cell::Target7:		return L"�V";
		case Cell::Target8:		return L"�W";
		case Cell::Target9:		return L"�X";

		case Cell::Beard:		return L"�v";
		case Cell::Razor:		return L"�I";

		case Cell::HORock:		return L"��";
		}
		return L"�H";
	}

	//-----------------------------------------------------------------------------
	//! Condition�𕶎����
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
	//! �N���A
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
	//! �N���A
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
