//
// Map
//

#pragma once

namespace app
{

	enum class Command;

	static const u16 CELL_TYPE_MASK		= 0x00FF;
	static const u16 CELL_LABEL_MASK 	= 0xFF00;
	static const u16 CELL_LABEL_SHIFT 	= 8;

	static const u32 MAX_TRAMPOLINE 	= 9;

	//===================================================================================
	//! @enum Cell
	//===================================================================================
	enum class Cell : u16
	{
		Empty			,
		Robot			,
		Rock			,
		Lambda			,
		Earth			,
		Wall			,
		ClosedLift		,
		OpenLift		,
		Trampoline		,
		Target			,
		Beard			,
		Razor			,
		HORock			,

		TrampolineA		= (0x00 << CELL_LABEL_SHIFT) | (u32)Trampoline,
		TrampolineB		= (0x01 << CELL_LABEL_SHIFT) | (u32)Trampoline,
		TrampolineC		= (0x02 << CELL_LABEL_SHIFT) | (u32)Trampoline,
		TrampolineD		= (0x03 << CELL_LABEL_SHIFT) | (u32)Trampoline,
		TrampolineE		= (0x04 << CELL_LABEL_SHIFT) | (u32)Trampoline,
		TrampolineF		= (0x05 << CELL_LABEL_SHIFT) | (u32)Trampoline,
		TrampolineG		= (0x06 << CELL_LABEL_SHIFT) | (u32)Trampoline,
		TrampolineH		= (0x07 << CELL_LABEL_SHIFT) | (u32)Trampoline,
		TrampolineI		= (0x08 << CELL_LABEL_SHIFT) | (u32)Trampoline,

		Target1			= (0x00 << CELL_LABEL_SHIFT) | (u32)Target,
		Target2			= (0x01 << CELL_LABEL_SHIFT) | (u32)Target,
		Target3			= (0x02 << CELL_LABEL_SHIFT) | (u32)Target,
		Target4			= (0x03 << CELL_LABEL_SHIFT) | (u32)Target,
		Target5			= (0x04 << CELL_LABEL_SHIFT) | (u32)Target,
		Target6			= (0x05 << CELL_LABEL_SHIFT) | (u32)Target,
		Target7			= (0x06 << CELL_LABEL_SHIFT) | (u32)Target,
		Target8			= (0x07 << CELL_LABEL_SHIFT) | (u32)Target,
		Target9			= (0x08 << CELL_LABEL_SHIFT) | (u32)Target,
	};

	inline u8 cellLabel(Cell c){ return (static_cast<u32>(c) & CELL_LABEL_MASK) >> CELL_LABEL_SHIFT; }
	inline Cell cellType(Cell c){ return static_cast<Cell>(static_cast<u32>(c) & CELL_TYPE_MASK); }

	//! Cell‚ð•¶Žš—ñ‚É
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

	//! Condition‚ð•¶Žš—ñ‚É
	const s3d::wchar* stringOfCondition(Condition cond);


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

}