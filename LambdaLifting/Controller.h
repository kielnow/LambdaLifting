//
// Controller
//

#pragma once

namespace app
{
	// Forward decralation
	enum class Command;

	//===================================================================================
	//! @class Controller
	//===================================================================================
	class Controller
	{
	public:
		Controller(){}
		virtual ~Controller(){}

		virtual void update(class Simulator& simulator) = 0;
	};


	//===================================================================================
	//! @class InteractiveController
	//===================================================================================
	class InteractiveController : public Controller
	{
	public:
		InteractiveController();
		~InteractiveController();

		void update(class Simulator& simulator) override;

	private:
		Command readCommand();

	private:
		u32 mInitialDelay;
		u32 mInterval;

		Command mLastCommand;
		u32 mLastDuration;
		bool mRepeat;
	};


	//===================================================================================
	//! @class AutoController
	//===================================================================================
	class AutoController : public Controller
	{
	public:
		AutoController();
		~AutoController();

		void update(class Simulator& simulator) override;

		void setCommand(const s3d::String& cmds);

		void play();
		void pause();
		void stop();

		bool isPlay() const { return mState == State::Play; }
		bool isStop() const { return mState == State::Stop; }

		const s3d::String& getNormalComamnds() const { return mNormalCommands; }

	private:
		enum class State {
			Reset,
			Play,
			Pause,
			Stop,
		};

	private:
		s3d::String mCommands;
		u32 mCommandPos;

		s3d::String mNormalCommands;

		State mState;
		bool mReset;
	};

}