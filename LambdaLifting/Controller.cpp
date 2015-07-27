//
// Controller
//

#include "stdafx.h"
#include "Controller.h"

#include "Map.h"
#include "Simulator.h"

namespace app
{

#pragma region InteractiveController

	//-----------------------------------------------------------------------------
	//! ctor
	//-----------------------------------------------------------------------------
	InteractiveController::InteractiveController()
		: mInitialDelay(200)
		, mInterval(50)
		, mLastCommand(Command::None)
		, mLastDuration(0)
		, mRepeat(false)
	{
	}

	//-----------------------------------------------------------------------------
	//! dtor
	//-----------------------------------------------------------------------------
	InteractiveController::~InteractiveController()
	{
	}

	//-----------------------------------------------------------------------------
	//! 更新
	//-----------------------------------------------------------------------------
	void InteractiveController::update(class Simulator& simulator)
	{
		Command cmd = readCommand();
		if (cmd != Command::None && simulator.isPlaying()) {
			if (!simulator.step(cmd)) {
				simulator.undo();
			}
		}
	}

	//-----------------------------------------------------------------------------
	//! コマンドを読み込む
	//-----------------------------------------------------------------------------
	Command InteractiveController::readCommand()
	{
		Command cmd = Command::None;
		u32 duration = 0;

		if (s3d::Input::KeyUp.pressed)
		{
			cmd = Command::Up;
			duration = s3d::Input::KeyUp.pressedDuration;
		}
		else if (s3d::Input::KeyDown.pressed)
		{
			cmd = Command::Down;
			duration = s3d::Input::KeyDown.pressedDuration;
		}
		else if (s3d::Input::KeyLeft.pressed)
		{
			cmd = Command::Left;
			duration = s3d::Input::KeyLeft.pressedDuration;
		}
		else if (s3d::Input::KeyRight.pressed)
		{
			cmd = Command::Right;
			duration = s3d::Input::KeyRight.pressedDuration;
		}
		else if (s3d::Input::KeyW.clicked || s3d::Input::KeySpace.clicked)
		{
			cmd = Command::Wait;
		}
		else if (s3d::Input::KeyA.clicked)
		{
			cmd = Command::Abort;
		}
		else if (s3d::Input::KeyS.clicked)
		{
			cmd = Command::Shave;
		}

		bool cancel = false;
		if (cmd != Command::None && cmd == mLastCommand)
		{
			cancel = true;

			if (mRepeat)
			{
				if (duration - mLastDuration > mInterval) {
					mLastDuration += mInterval;
					cancel = false;
				}
			}
			else if (duration > mInitialDelay)
			{
				mLastDuration = duration;
				mRepeat = true;
				cancel = false;
			}
		}
		else
		{
			mRepeat = false;
			mLastDuration = 0;
		}

		if (cancel) {
			cmd = Command::None;
		} else {
			mLastCommand = cmd;
		}

		//s3d::Println(duration);
		//s3d::Println(mLastDuration);

		return cmd;
	}

#pragma endregion


#pragma region AutoController

	//-----------------------------------------------------------------------------
	//! ctor
	//-----------------------------------------------------------------------------
	AutoController::AutoController()
		: mCommandPos(0)
		, mState(State::Stop)
		, mReset(true)
		, mInterval(0)
		, mIntervalCount(0)
	{
	}

	//-----------------------------------------------------------------------------
	//! dtor
	//-----------------------------------------------------------------------------
	AutoController::~AutoController()
	{
	}

	//-----------------------------------------------------------------------------
	//! 更新
	//-----------------------------------------------------------------------------
	void AutoController::update(class Simulator& simulator)
	{
		switch (mState)
		{
		case State::Play:
			if (mReset) {
				mReset = false;
				simulator.reset();
				mCommandPos = 0;
				mValidCommands.clear();
			}

			if (mCommandPos < mCommands.length && simulator.isPlaying())
			{
				if (mIntervalCount++ < mInterval) {
					break;
				}
				mIntervalCount = 0;

				if (simulator.redoable()) {
					simulator.redo();
				} else {
					s3d::wchar cmd = mCommands[mCommandPos++];
					if (simulator.step(cmd)) {
						mValidCommands += cmd;
					}
				}
			}
			else
			{
				mState = State::Stop;
			}
			break;

		case State::Pause:
		case State::Stop:
			break;
		}
	}

	//-----------------------------------------------------------------------------
	//! コマンドを設定
	//-----------------------------------------------------------------------------
	void AutoController::setCommand(const s3d::String& cmds, bool reset)
	{
		mCommands = cmds;
		mState = State::Reset;
		mReset = reset;
	}

	//-----------------------------------------------------------------------------
	//! 再生
	//-----------------------------------------------------------------------------
	void AutoController::play(bool reset)
	{
		if (mState == State::Stop) {
			mReset = reset;
		}
		mState = State::Play;
	}

	//-----------------------------------------------------------------------------
	//! 一時停止
	//-----------------------------------------------------------------------------
	void AutoController::pause()
	{
		mState = State::Pause;
	}

	//-----------------------------------------------------------------------------
	//! 停止
	//-----------------------------------------------------------------------------
	void AutoController::stop()
	{
		mState = State::Stop;
		mValidCommands.clear();
	}

#pragma endregion

}