//
// Simulator
//

#pragma once

namespace app
{

	// Forward declaration
	enum class Command;
	struct Map;

	//===================================================================================
	//! @class Simulator
	//===================================================================================
	class Simulator
	{
	public:
		Simulator();
		~Simulator();

		void clear(bool all = true);

		bool loadMap(const s3d::String& filepath);

		static bool loadMap(const s3d::String& filepath, struct MapInfo& mapInfo, struct Map& map);

		//-----------------------------------------------------------------------------
		void run(const s3d::String& cmds);

		bool step(s3d::wchar cmd);
		bool step(Command cmd);

		static bool step(Command cmd, struct Map& newMap);

		void reset();
		bool undo(u32 step = 1);
		bool redo(u32 step = 1);

		bool undoable() const;
		bool redoable() const;

		void setHistoryMax(s32 m = -1){ mHistoryMax = m; }

		//-----------------------------------------------------------------------------
		static bool loadAsset();

		void draw(const s3d::Vec2& pos = s3d::Vec2::Zero, f64 scale = 1.f) const;
		void drawShadow(const s3d::Vec2& pos = s3d::Vec2::Zero, f64 scale = 1.f) const;
		void drawTrail(s32 length, const s3d::Vec2& pos = s3d::Vec2::Zero, f64 scale = 1.f) const;

		s3d::RectF calcRect(const s3d::Vec2& pos = s3d::Vec2::Zero, f64 scale = 1.f) const;

		s3d::RectF drawMapInfo(const s3d::Vec2& pos = s3d::Vec2::Zero, const s3d::Color& = s3d::Palette::White) const;
		s3d::RectF drawCommands(const s3d::Vec2& pos = s3d::Vec2::Zero, const s3d::Color& = s3d::Palette::White) const;

		//-----------------------------------------------------------------------------
		const s3d::String& getFilePath() const { return mFilePath;  }
		const struct Map& getMap() const;
		bool isPlaying() const;
		u32 getHistoryNum() const { return mHistory.size(); }

	private:
		//! @name Auxiliary function
		//@{
		static bool updateRobot(Command cmd, struct Map& map);
		static bool moveRobot(Command cmd, struct Map& map);
		static u32  updateMap(struct Map& map);
		static bool updateFlooding(struct Map& map);
		static bool updateBeard(struct Map& map);
		//@}

		void pushHistory(s3d::wchar cmd, struct Map* pmap);
		bool resumeHistory();

	private:
		s3d::FilePath mFilePath;
		s3d::String mFileName;
		struct MapInfo* mpMapInfo;
		struct Map* mpInitialMap;
		struct Map* mpMap;

		s3d::String mCommands;
		u32 mCommandPos;
		std::vector<bool> mValids;
		std::deque<struct Map*> mHistory;
		u32 mHistoryPos;
		s32 mHistoryMax;
	};

} // namespace app