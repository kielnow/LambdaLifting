//
// Command
//

#pragma once

namespace app
{

	//===================================================================================
	//! @enum コマンド
	//===================================================================================
	enum class Command
	{
		None,
		Up,
		Down,
		Left,
		Right,
		Wait,
		Abort,
		Shave,
	};

	//! コマンドを文字に
	s3d::wchar charOfCommand(Command cmd);

	//! 文字をコマンドに
	Command commandOfChar(s3d::wchar wc);

	//===================================================================================
	//! @class InputCommand
	//===================================================================================
	class InputCommand
	{
	public:
		InputCommand();
		~InputCommand();

		Command command();

	private:
		u32 mInitialDelay;
		u32 mInterval;
		Command mLastCommand;
		u32 mLastDuration;
		bool mRepeat;
	};

}