//
// Command
//

#include "stdafx.h"
#include "Command.h"

namespace app
{

	//! コマンドを文字に
	s3d::wchar charOfCommand(Command cmd)
	{
		switch(cmd){
		case Command::Up:		return L'U';
		case Command::Down:		return L'D';
		case Command::Left:		return L'L';
		case Command::Right:	return L'R';
		case Command::Wait:		return L'W';
		case Command::Abort:	return L'A';
		case Command::Shave:	return L'S';
		}
		return L'?';
	}

	//! 文字をコマンドに
	Command commandOfChar(s3d::wchar wc)
	{
		switch(wc){
		case L'U':		return Command::Up;
		case L'D':		return Command::Down;
		case L'L':		return Command::Left;
		case L'R':		return Command::Right;
		case L'W':		return Command::Wait;
		case L'A':		return Command::Abort;
		case L'S':		return Command::Shave;
		}
		return Command::None;
	}

	//===================================================================================
	// InputCommand
	//===================================================================================

	//! ctor
	InputCommand::InputCommand()
		: mInitialDelay(200)
		, mInterval(50)
		, mLastCommand(Command::None)
		, mLastDuration(0)
		, mRepeat(false)
	{
	}

	//! dtor
	InputCommand::~InputCommand()
	{
	}

	//! command
	Command InputCommand::command()
	{
		Command cmd = Command::None;
		u32 duration = 0;
		if(s3d::Input::KeyUp.pressed){
			cmd = Command::Up;
			duration = s3d::Input::KeyUp.pressedDuration;
		}
		else if(s3d::Input::KeyDown.pressed){
			cmd = Command::Down;
			duration = s3d::Input::KeyDown.pressedDuration;
		}
		else if(s3d::Input::KeyLeft.pressed){
			cmd = Command::Left;
			duration = s3d::Input::KeyLeft.pressedDuration;
		}
		else if(s3d::Input::KeyRight.pressed){
			cmd = Command::Right;
			duration = s3d::Input::KeyRight.pressedDuration;
		}
		else if(s3d::Input::KeyW.clicked || s3d::Input::KeySpace.clicked){
			cmd = Command::Wait;
		}
		else if(s3d::Input::KeyA.clicked){
			cmd = Command::Abort;
		}
		else if(s3d::Input::KeyS.clicked){
			cmd = Command::Shave;
		}

		bool cancel = false;
		if(cmd != Command::None && cmd == mLastCommand){
			cancel = true;

			if(mRepeat){
				if(duration - mLastDuration > mInterval){
					mLastDuration += mInterval;
					cancel = false;
				}
			}
			else if(duration > mInitialDelay){
				mLastDuration = duration;
				mRepeat = true;
				cancel = false;
			}
		}
		else {
			mRepeat = false;
			mLastDuration = 0;
		}

		if(cancel){
			cmd = Command::None;
		}else{
			mLastCommand = cmd;
		}

		//s3d::Println(duration);
		//s3d::Println(mLastDuration);

		return cmd;
	}

}