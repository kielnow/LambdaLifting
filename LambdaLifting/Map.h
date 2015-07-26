//
// Map
//

#pragma once

namespace app
{

	//===================================================================================
	//! @enum Command
	//===================================================================================
	enum class Command
	{
		None	= L'?',
		Up		= L'U',
		Down	= L'D',
		Left	= L'L',
		Right	= L'R',
		Wait	= L'W',
		Abort	= L'A',
		Shave	= L'S',
	};

	s3d::wchar charOfCommand(Command cmd);
	Command commandOfChar(s3d::wchar c);

	//===================================================================================
	// Cell
	//===================================================================================
	static const u16 CELL_TYPE_MASK		= 0x00FF;
	static const u16 CELL_LABEL_MASK 	= 0xFF00;
	static const u16 CELL_LABEL_SHIFT 	= 8;

	#define MAKE_LABELED_CELL(type, label)	((label << CELL_LABEL_SHIFT) | ((u32)type & CELL_TYPE_MASK))
	
	//! @enum Cell
	enum class Cell : u16
	{
		Empty,
		Robot,
		Rock,
		Lambda,
		Earth,
		Wall,
		ClosedLift,
		OpenLift,
		Trampoline,
		Target,
		Beard,
		Razor,
		HORock,

		// Labeled Trampoline
		TrampolineA		= MAKE_LABELED_CELL(Trampoline, 0),
		TrampolineB		= MAKE_LABELED_CELL(Trampoline, 1),
		TrampolineC		= MAKE_LABELED_CELL(Trampoline, 2),
		TrampolineD		= MAKE_LABELED_CELL(Trampoline, 3),
		TrampolineE		= MAKE_LABELED_CELL(Trampoline, 4),
		TrampolineF		= MAKE_LABELED_CELL(Trampoline, 5),
		TrampolineG		= MAKE_LABELED_CELL(Trampoline, 6),
		TrampolineH		= MAKE_LABELED_CELL(Trampoline, 7),
		TrampolineI		= MAKE_LABELED_CELL(Trampoline, 8),

		// Labeled Target
		Target1 		= MAKE_LABELED_CELL(Target, 0),
		Target2			= MAKE_LABELED_CELL(Target, 1),
		Target3			= MAKE_LABELED_CELL(Target, 2),
		Target4			= MAKE_LABELED_CELL(Target, 3),
		Target5			= MAKE_LABELED_CELL(Target, 4),
		Target6			= MAKE_LABELED_CELL(Target, 5),
		Target7			= MAKE_LABELED_CELL(Target, 6),
		Target8			= MAKE_LABELED_CELL(Target, 7),
		Target9			= MAKE_LABELED_CELL(Target, 8),
	};

	inline Cell cellType(Cell c){ return static_cast<Cell>( static_cast<u16>(c) & CELL_TYPE_MASK ); }
	inline u8 cellLabel(Cell c){ return (static_cast<u16>(c) & CELL_LABEL_MASK) >> CELL_LABEL_SHIFT; }

	const s3d::wchar* stringOfCell(Cell cell);

	//===================================================================================
	//! @enum Condition
	//===================================================================================
	enum class Condition : u8
	{
		Playing,
		Winning,
		Abort,
		Losing,
	};

	const s3d::wchar* stringOfCondition(Condition cond);

	using int2 = s3d::Vector2D<s32>;

	//===================================================================================
	//! @struct Map
	//===================================================================================
	struct Map
	{
		struct MapInfo* info;

		s3d::Grid<Cell> cell;
		int2 robotPos;
		u32	lambda;
		u32 lambdaCollected;
		u32 stepCount;
		s32 score;
		Condition condition;

		// Flooding
		u32 water;
		u32 floodingCount;
		u32 waterproofCount;

		// Beard
		u32 growthCount;
		u32 razor;
		u32 beard;

	public:
		Map(){ clear(); }

		void clear();

		void step(){ stepCount++; score--; }
	};

	//===================================================================================
	// Map Info
	//===================================================================================
	static const u32 MAX_TRAMPOLINE = 9;

	//! @struct Jump Table
	struct MapInfo
	{
		int2 liftPos;

		// Flooding
		u32 flooding;
		u32 waterproof;

		// Beard
		u32 growth;

		// Trampoline
		int2 trampolinePos[MAX_TRAMPOLINE];
		int2 targetPos[MAX_TRAMPOLINE];
		u8 jump[MAX_TRAMPOLINE];

	public:
		MapInfo(){ clear(); }

		void clear();
	};


#if 0
	//===================================================================================
	//! @class Map
	//===================================================================================
	class Map
	{
	public:
		using Vec2i = s3d::Vector2D<s32>;

		Map();
		~Map();

		bool load(const s3d::FilePath& filepath);
		bool load(s3d::TextReader& r);

		void clear();

		bool update(Command cmd);
		bool isUpdatable() const { return mCondition == Condition::Playing; }

		//-----------------------------------------------------------------------------
		//! @name •`‰æ
		//@{

		static const s3d::Vec2 CellSize;

		s3d::RectF draw(f64 scale = 1.f) const;
		s3d::RectF draw(f64 x, f64 y, f64 scale = 1.f) const;
		s3d::RectF draw(const s3d::Vec2& ofs, f64 scale = 1.f) const;

		s3d::RectF drawShadow(f64 scale = 1.f) const;
		s3d::RectF drawShadow(f64 x, f64 y, f64 scale = 1.f) const;
		s3d::RectF drawShadow(const s3d::Vec2& ofs, f64 scale = 1.f) const;

		void print() const;

		//@}

		//-----------------------------------------------------------------------------
		//! @name Accessors
		//@{

		u32 width() const { return mCell.width; }
		u32 height() const { return mCell.height; }

		const Vec2i& robotPos() const { return mRobotPos; }

		u32 lambda() const { return mLambda; }
		u32 lambdaCollected() const { return mLambdaCollected; }

		u32 stepCount() const { return mStepCount; }

		s32 score() const { return mScore; }

		Condition condition() const { return mCondition;  }

		Cell cell(Vec2i v) const { return cell(v.x, v.y); }
		Cell cell(s32 x, s32 y) const
		{
			if(0 <= y && y < mCell.height && 0 <= x && x < mCell.width){
				return mCell[x][y];
			}else{
				return Cell::Empty;
			}
		}

		u32 water() const { return mWater; }
		u32 flooding() const { return mFlooding; }
		u32 floodingCount() const { return mFloodingCount; }
		u32 waterproof() const { return mWaterproof; }
		u32 waterproofCount() const { return mWaterproofCount; }

		u32 growth() const { return mGrowth; }
		u32 growthCount() const { return mGrowthCount; }
		u32 razor() const { return mRazor; }
		u32 beard() const { return mBeard; }

		//@}

	private:
		void incrStepCount(){ mStepCount++; mScore--; }
		bool move(Command cmd);
		u32 updateMap();
		bool updateFlooding();
		bool updateBeard();

	private:
		s3d::Grid<Cell> mCell;
		Vec2i mRobotPos;
		Vec2i mLiftPos;
		u32 mLambda;
		u32 mLambdaCollected;
		u32 mStepCount;
		s32 mScore;
		Condition mCondition;

		u32 mWater;
		u32 mFlooding;
		u32 mFloodingCount;
		u32 mWaterproof;
		u32 mWaterproofCount;

		u8 mTrampolineTo[MAX_TRAMPOLINE];
		Vec2i mTrampolinePos[MAX_TRAMPOLINE];
		Vec2i mTargetPos[MAX_TRAMPOLINE];

		u32 mGrowth;
		u32 mGrowthCount;
		u32 mRazor;
		u32 mBeard;
	};
#endif

}