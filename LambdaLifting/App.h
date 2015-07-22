//
// App
//

#pragma once

#include "Map.h"

namespace app
{

	//===================================================================================
	//! @class App
	//===================================================================================
	class App
	{
	public:
		App();
		~App();

		void initialize();
		void finalize();

		void update();
		void draw();

	private:
		void updateGUI();

		bool load(const s3d::FilePath& filepath);

		void undo();
		void redo();
		void reset();

		void play();
		void pause();
		void resume();
		void stop();

		void save();
		void loadCommands();

		bool isOperational() const;

		static const u32 MAX_HISTORY = 1024;

		enum class State {
			Play,
			Pause,
			Stop,
		};

	private:
		s3d::FilePath mFilepath;
		s3d::String mFilename;
		std::unique_ptr<class Map> mInitialMapPtr;
		std::unique_ptr<class Map> mMapPtr;

		s3d::String mCommands;
		std::deque<class Map> mHistory;
		s32 mUndoPos;

		std::vector<bool> mValids;

		s3d::String mInputCommands;

		std::unique_ptr<class InputCommand> mInputPtr;
		class s3d::GUI* mGUIPtr;

		State mState;
	};

}