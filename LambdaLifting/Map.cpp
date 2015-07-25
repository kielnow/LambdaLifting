//
// Map
//

#include "stdafx.h"
#include "Map.h"

namespace app
{

	//-----------------------------------------------------------------------------
	//! Commandを文字に
	//-----------------------------------------------------------------------------
	s3d::wchar charOfCommand(Command cmd)
	{
		return static_cast<s3d::wchar>(cmd);
	}

	//-----------------------------------------------------------------------------
	//! 文字をCommandに
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
	//! Cellを文字列に
	//-----------------------------------------------------------------------------
	const s3d::wchar* stringOfCell(Cell cell)
	{
		switch (cell) {
		case Cell::Empty:		return L"　";
		case Cell::Robot:		return L"Ｒ";
		case Cell::Rock:		return L"＊";
		case Cell::Lambda:		return L"＼";
		case Cell::Earth:		return L"・";
		case Cell::Wall:		return L"＃";
		case Cell::ClosedLift:	return L"Ｌ";
		case Cell::OpenLift:	return L"Ｏ";

		case Cell::TrampolineA:	return L"Ａ";
		case Cell::TrampolineB:	return L"Ｂ";
		case Cell::TrampolineC:	return L"Ｃ";
		case Cell::TrampolineD:	return L"Ｄ";
		case Cell::TrampolineE:	return L"Ｅ";
		case Cell::TrampolineF:	return L"Ｆ";
		case Cell::TrampolineG:	return L"Ｇ";
		case Cell::TrampolineH:	return L"Ｈ";
		case Cell::TrampolineI:	return L"Ｉ";

		case Cell::Target1:		return L"１";
		case Cell::Target2:		return L"２";
		case Cell::Target3:		return L"３";
		case Cell::Target4:		return L"４";
		case Cell::Target5:		return L"５";
		case Cell::Target6:		return L"６";
		case Cell::Target7:		return L"７";
		case Cell::Target8:		return L"８";
		case Cell::Target9:		return L"９";

		case Cell::Beard:		return L"Ｗ";
		case Cell::Razor:		return L"！";

		case Cell::HORock:		return L"＠";
		}
		return L"？";
	}

	//-----------------------------------------------------------------------------
	//! Conditionを文字列に
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
	//! クリア
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
	//! クリア
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

#if 0
	//===================================================================================
	// Map
	//===================================================================================

	//! Cellのサイズ
	const s3d::Vec2 Map::CellSize{ 32, 32 };

	//-----------------------------------------------------------------------------
	//! ctor
	//-----------------------------------------------------------------------------
	Map::Map()
		: mRobotPos(0, 0)
		, mLiftPos(0, 0)
		, mLambda(0)
		, mLambdaCollected(0)
		, mStepCount(0)
		, mScore(0)
		, mCondition(Condition::Playing)

		, mWater(0)
		, mFlooding(0)
		, mFloodingCount(0)
		, mWaterproof(10)
		, mWaterproofCount(10)

		, mGrowth(25)
		, mGrowthCount(0)
		, mRazor(0)
		, mBeard(0)
	{
	}

	//-----------------------------------------------------------------------------
	//! dtor
	//-----------------------------------------------------------------------------
	Map::~Map()
	{
	}

	//-----------------------------------------------------------------------------
	//! Load
	//-----------------------------------------------------------------------------
	bool Map::load(const s3d::FilePath& filepath)
	{
		s3d::TextReader r(filepath);
		return load(r);
	}

	//-----------------------------------------------------------------------------
	//! Load
	//-----------------------------------------------------------------------------
	bool Map::load(s3d::TextReader& r)
	{
		if(!r.isOpened()){
			return false;
		}

		// クリア
		clear();

		const s3d::String s = r.readContents();

		u32 w = 0, h = 0;
		bool bend = false;
		for(const auto& line : s.split(L'\n')){
			if(!bend){
				if(line[0] == L'\0' || line.length == 1){
					bend = true;
					continue;
				}
				w = std::max(w, line.length);
				h++;
				continue;
			}

			if(line.startsWith(L"Water ")){
				mWater = s3d::Parse<u32>(line.split(' ').at(1));
			}
			else if(line.startsWith(L"Flooding ")){
				mFlooding = s3d::Parse<u32>(line.split(' ').at(1));
				mFloodingCount = mFlooding - 1;
			}
			else if(line.startsWith(L"Waterproof ")){
				mWaterproof = s3d::Parse<u32>(line.split(' ').at(1));
				mWaterproofCount = mWaterproof;
			}
			else if(line.startsWith(L"Trampoline ")){
				auto words = line.split(' ');
				const u8 from = static_cast<u8>( s3d::Parse<s3d::wchar>(words.at(1)) - L'A' );
				const u8 to   = static_cast<u8>( s3d::Parse<s3d::wchar>(words.at(3)) - L'1' );
				mTrampolineTo[from] = to;
			}
			else if(line.startsWith(L"Growth ")){
				mGrowth = s3d::Parse<u32>(line.split(' ').at(1));
				mGrowthCount = mGrowth - 1;
			}
			else if(line.startsWith(L"Razor ")){
				mRazor = s3d::Parse<u32>(line.split(' ').at(1));
			}
		}

		mCell.resize(w, h, Cell::Empty);

		int x = 0, y = 0;
		for(const auto wc : s){
			if(wc == L'\n'){
				if(x == 0){
					break;
				}
				x = 0;
				y++;
				continue;
			}

			Cell cell = Cell::Empty;
			switch(wc){
			case L' ':
				cell = Cell::Empty;
				break;
			case L'R':
				cell = Cell::Robot;
				mRobotPos.set(x, y);
				break;
			case L'*':
				cell = Cell::Rock;
				break;
			case L'\\':
				cell = Cell::Lambda;
				mLambda++;
				break;
			case L'.':
				cell = Cell::Earth;
				break;
			case L'#':
				cell = Cell::Wall;
				break;
			case L'L':
				cell = Cell::ClosedLift;
				mLiftPos.set(x, y);
				break;
			case L'O':
				cell = Cell::OpenLift;
				mLiftPos.set(x, y);
				break;
			case L'W':
				cell = Cell::Beard;
				mBeard++;
				break;
			case L'!':
				cell = Cell::Razor;
				break;
			case L'@':
				cell = Cell::HORock;
				mLambda++;
				break;
			default:
				if(L'A' <= wc && wc <= L'I'){
					const u32 label = wc - L'A';
					cell = static_cast<Cell>((u32)Cell::Trampoline | (label << CELL_LABEL_SHIFT));
					mTrampolinePos[label].set(x, y);
				}
				else if(L'1' <= wc && wc <= L'9'){
					const u32 label = wc - L'1';
					cell = static_cast<Cell>((u32)Cell::Target | (label << CELL_LABEL_SHIFT));
					mTargetPos[label].set(x, y);
				}
				else{
					// 想定外の文字があった場合は読み込み失敗
					clear();
					return false;
				}
				break;
			}

			mCell[y][x] = cell;
			x++;
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	//! Clear
	//-----------------------------------------------------------------------------
	void Map::clear()
	{
		mCell.clear();
		mRobotPos.set(0, 0);
		mLiftPos.set(0, 0);
		mLambda = 0;
		mLambdaCollected = 0;
		mStepCount = 0;
		mCondition = Condition::Playing;

		mWater = 0;
		mFlooding = 0;
		mFloodingCount = 0;
		mWaterproof = 10;
		mWaterproofCount = mWaterproof;

		std::fill(mTrampolineTo, mTrampolineTo + MAX_TRAMPOLINE, 0xFF);
		memset(mTrampolinePos, 0, sizeof(mTrampolinePos));
		memset(mTargetPos, 0, sizeof(mTargetPos));

		mGrowth = 25;
		mGrowthCount = 0;
		mRazor = 0;
		mBeard = 0;
	}

	//-----------------------------------------------------------------------------
	//! Update
	//-----------------------------------------------------------------------------
	bool Map::update(Command cmd)
	{
		if(cmd == Command::None || !isUpdatable()){
			return false;
		}

		bool bupdate = true;

		// ロボット更新
		switch(cmd){
		case Command::Up:
		case Command::Down:
		case Command::Left:
		case Command::Right:
			bupdate = move(cmd);
			break;
		case Command::Wait:
			break;
		case Command::Abort:
			mCondition = Condition::Abort;
			mScore += 25 * mLambdaCollected;
			break;
		case Command::Shave:
			if(mRazor > 0){
				const s3d::Rect rect{ 0, 0, mCell.width, mCell.height };

				static const s32 d[] = { -1, 0, 1 };
				for(u32 dy = 0; dy < 3; ++dy){
					for(u32 dx = 0; dx < 3; ++dx){
						if(!dx && !dy){
							continue;
						}

						const s32 ax = mRobotPos.x + d[dx], ay = mRobotPos.y + d[dy];

						// マップ範囲外チェック
						if(!Vec2i{ ax, ay }.intersects(rect)){
							continue;
						}

						if(mCell[ay][ax] == Cell::Beard){
							mCell[ay][ax] = Cell::Empty;
							mBeard--;
						}
					}
				}

				// 髭がすべて無くなったらmGrowthCountを0にしておく
				if(mBeard == 0){
					mGrowthCount = 0;
				}

				mRazor--;
				incrStepCount();
			}
			else{
				bupdate = false;
			}
			break;
		default:
			break;
		}

		// マップ更新
		const u32 count = updateMap();

		if(cmd == Command::Wait){
			// 待機した場合はマップの更新があったかを調べる
			bupdate = count > 0;
		}

		// 洪水更新
		bupdate |= updateFlooding();

		// 髭更新
		bupdate |= updateBeard();

		return bupdate;
	}

	//-----------------------------------------------------------------------------
	//! ロボット移動
	//-----------------------------------------------------------------------------
	bool Map::move(Command cmd)
	{
		const s3d::Rect rect{ 0, 0, mCell.width, mCell.height };

		Vec2i newPos{ mRobotPos };
		switch(cmd){
		case Command::Up:
			newPos.y--;
			break;
		case Command::Down:
			newPos.y++;
			break;
		case Command::Left:
			newPos.x--;
			break;
		case Command::Right:
			newPos.x++;
			break;
		}

		// マップ範囲外チェック
		if(!newPos.intersects(rect)){
			return false;
		}

		// 移動できたか
		bool valid = false;

		const Cell c = mCell[newPos.y][newPos.x];
		if(c == Cell::Empty || c == Cell::Earth || c == Cell::Lambda || c == Cell::OpenLift || c == Cell::Razor || cellType(c) == Cell::Trampoline){
			mCell[mRobotPos.y][mRobotPos.x] 	= Cell::Empty;
			mCell[newPos.y][newPos.x] 			= Cell::Robot;
			mRobotPos = newPos;
			incrStepCount();
			valid = true;

			if(c == Cell::Lambda){
				mLambdaCollected++;
				mScore += 25;
			}
			else if(c == Cell::OpenLift){
				mCondition = Condition::Winning;
				mScore += 50 * mLambdaCollected;
			}
			else if(c == Cell::Razor){
				mRazor++;
			}
			else if(cellType(c) == Cell::Trampoline){
				const u8 from = cellLabel(c);
				const u8 to = mTrampolineTo[from];
				const Vec2i jumpPos{ mTargetPos[to] };

				mCell[newPos.y][newPos.x] 		= Cell::Empty;
				mCell[jumpPos.y][jumpPos.x] 	= Cell::Robot;
				mRobotPos = jumpPos;

				// ターゲットに関連付けられていたトランポリンを消去
				for(u32 i = 0; i < MAX_TRAMPOLINE; ++i){
					if(mTrampolineTo[i] == to){
						mCell[mTrampolinePos[i].y][mTrampolinePos[i].x] = Cell::Empty;
					}
				}
			}
		}
		else if((c == Cell::Rock || c == Cell::HORock) && (cmd == Command::Left || cmd == Command::Right)){
			const Vec2i nextPos{ newPos.movedBy(cmd == Command::Left ? -1 : 1, 0) };

			// マップ範囲外チェック
			if(!nextPos.intersects(rect)){
				return false;
			}

			if(mCell[nextPos.y][nextPos.x] == Cell::Empty){
				mCell[mRobotPos.y][mRobotPos.x] = Cell::Empty;
				mCell[newPos.y][newPos.x] 		= Cell::Robot;
				mCell[nextPos.y][nextPos.x] 	= c;
				mRobotPos = newPos;
				incrStepCount();
				valid = true;
			}
		}

		return valid;
	}

	//-----------------------------------------------------------------------------
	//! マップ更新
	//-----------------------------------------------------------------------------
	u32 Map::updateMap()
	{
		const s3d::Grid<Cell> old{ mCell };

		u32 count = 0;
		for(auto y : s3d::step_backward(mCell.height)){
			for(auto x : s3d::step(mCell.width)){
				const Cell c = old[y][x];
				if(c == Cell::ClosedLift && mLambdaCollected == mLambda){
					mCell[y][x] = Cell::OpenLift;
					count++;
				}
				else if(y + 1 < mCell.height && (c == Cell::Rock || c == Cell::HORock)){
					Vec2i newPos{ x, y };
					switch(old[y + 1][x]){
					case Cell::Empty:
						newPos.set(x, y + 1);
						break;
					case Cell::Rock:
					case Cell::HORock:
						if(x + 1 < mCell.width && old[y][x + 1] == Cell::Empty && old[y + 1][x + 1] == Cell::Empty){
							newPos.set(x + 1, y + 1);
						}
						else if(0 <= x - 1 && old[y][x - 1] == Cell::Empty && old[y + 1][x - 1] == Cell::Empty){
							newPos.set(x - 1, y + 1);
						}
						break;
					case Cell::Lambda:
						if(x + 1 < mCell.width && old[y][x + 1] == Cell::Empty && old[y + 1][x + 1] == Cell::Empty){
							newPos.set(x + 1, y + 1);
						}
						break;
					default:
						// nop
						break;
					}

					// 岩が落下したか
					if(x != newPos.x || y != newPos.y){
						// ラムダ消滅チェック
						if(mCell[newPos.y][newPos.x] == Cell::Lambda){
							mLambda--;
						}

						mCell[y][x] = Cell::Empty;
						mCell[newPos.y][newPos.x] = c;
						count++;

						if(c == Cell::HORock && newPos.y + 1 < mCell.height && old[newPos.y + 1][newPos.x] != Cell::Empty){
							mCell[newPos.y][newPos.x] = Cell::Lambda;
						}
					}
				}
				else if(old[y][x] == Cell::Beard){
					if(mGrowthCount == 0){
						const s3d::Rect rect{ 0, 0, mCell.width, mCell.height };

						static const s32 d[] = { -1, 0, 1 };
						for(u32 dy = 0; dy < 3; ++dy){
							for(u32 dx = 0; dx < 3; ++dx){
								if(!dx && !dy){
									continue;
								}

								const s32 ax = x + d[dx], ay = y + d[dy];

								// マップ範囲外チェック
								if(!Vec2i{ ax, ay }.intersects(rect)){
									continue;
								}

								if(old[ay][ax] == Cell::Empty){
									// ラムダ消滅チェック
									if(mCell[ay][ax] == Cell::Lambda){
										mLambda--;
									}

									mCell[ay][ax] = Cell::Beard;
									mBeard++;
								}
							}
						}
					}
				}
			}
		}

		// ロボットが破壊されたかチェック
		if(mCondition == Condition::Playing	&& mRobotPos.y - 1 >= 0 && old[mRobotPos.y - 1][mRobotPos.x] != Cell::Rock && mCell[mRobotPos.y - 1][mRobotPos.x] == Cell::Rock){
			mCondition = Condition::Losing;
		}

		return count;
	}

	//-----------------------------------------------------------------------------
	//! 洪水更新
	//-----------------------------------------------------------------------------
	bool Map::updateFlooding()
	{
		// 更新があったか
		bool bupdate = false;

		if(mFlooding > 0){
			if(mFloodingCount == 0){
				mFloodingCount = mFlooding;
				mWater = std::min(mWater + 1, (u32)mCell.height);
			}
			mFloodingCount--;
			bupdate = true;
		}

		// ロボットが水没したかチェック
		if(mCondition == Condition::Playing){
			if(mCell.height - (s32)mWater <= mRobotPos.y){
				if(mWaterproofCount == 0){
					mCondition = Condition::Losing;
				}
				else{
					mWaterproofCount--;
				}
				bupdate = true;
			}
			else{
				if(mWaterproofCount < mWaterproof){
					mWaterproofCount = mWaterproof;
					bupdate = true;
				}
			}
		}

		return bupdate;
	}

	//-----------------------------------------------------------------------------
	//! 髭更新
	//-----------------------------------------------------------------------------
	bool Map::updateBeard()
	{
		bool bupdate = false;

		if(mBeard > 0){
			if(mGrowthCount == 0){
				mGrowthCount = mGrowth;
			}
			mGrowthCount--;
			bupdate = true;
		}

		return bupdate;
	}

	//-----------------------------------------------------------------------------
	//! Draw
	//-----------------------------------------------------------------------------
	s3d::RectF Map::draw(f64 scale) const
	{
		return draw(s3d::Vec2::Zero, scale);
	}

	//-----------------------------------------------------------------------------
	//! Draw
	//-----------------------------------------------------------------------------
	s3d::RectF Map::draw(f64 x, f64 y, f64 scale) const
	{
		return draw(s3d::Vec2{ x, y }, scale);
	}

	//-----------------------------------------------------------------------------
	//! Draw
	//-----------------------------------------------------------------------------
	s3d::RectF Map::draw(const s3d::Vec2& ofs, f64 scale) const
	{
		const s3d::Texture tex = s3d::TextureAsset(L"cell");
		const s3d::Font font = s3d::FontAsset(L"font");
		const s3d::Vec2 sz{ s3d::Math::Round(CellSize * scale) };

		s3d::Graphics2D::SetSamplerState(s3d::SamplerState::WrapPoint);

		for(auto y : s3d::step(mCell.height)){
			for(auto x : s3d::step(mCell.width)){
				const Cell c = mCell[y][x];
				const Cell t = cellType(c);
				if(c == Cell::Empty){
					continue;
				}
				else if(c == Cell::Robot){
					if(mRobotPos == mLiftPos){
						// OpenLiftを描画
						tex(CellSize.x * static_cast<u32>(Cell::OpenLift), 0, CellSize.x, CellSize.y).resize(sz).draw(ofs + sz * s3d::Vec2{ x, y });
					}

					// ロボットを描画
					tex(CellSize.x * static_cast<u32>(mCondition), CellSize.y, CellSize.x, CellSize.y).resize(sz).draw(ofs + sz * s3d::Vec2{ mRobotPos.x, mRobotPos.y });
				}
				else{
					tex(CellSize.x * static_cast<u32>(t), 0, CellSize.x, CellSize.y).resize(sz).draw(ofs + sz * s3d::Vec2{ x, y });

					if(t == Cell::Trampoline){
						const u32 label = cellLabel(c);
						tex(CellSize.x * label, CellSize.y * 2, CellSize.x, CellSize.y).resize(sz).draw(ofs + sz * s3d::Vec2{ x, y });
						const u32 to = mTrampolineTo[label];
						tex(CellSize.x * to, CellSize.y * 3, CellSize.x, CellSize.y).resize(sz).draw(ofs + sz * s3d::Vec2{ x, y });
					}
					else if(t == Cell::Target){
						const u32 label = cellLabel(c);
						tex(CellSize.x * label, CellSize.y * 3, CellSize.x, CellSize.y).resize(sz).draw(ofs + sz * s3d::Vec2{ x, y });
					}
				}
			}
		}

		s3d::Graphics2D::SetSamplerState(s3d::SamplerState::Default2D);

		// 水を描画
		s3d::Color water{ s3d::Palette::Blue };
		water.a = 96;
		s3d::RectF{ ofs.x, ofs.y + sz.y * (mCell.height - mWater), sz.x * mCell.width, sz.y * mWater }.draw(water);

		return s3d::RectF{ ofs.x, ofs.y, sz.x * mCell.width, sz.y * mCell.height };
	}

	//-----------------------------------------------------------------------------
	//! Draw Shadow
	//-----------------------------------------------------------------------------
	s3d::RectF Map::drawShadow(f64 scale) const
	{
		return drawShadow(s3d::Vec2::Zero, scale);
	}

	//-----------------------------------------------------------------------------
	//! Draw Shadow
	//-----------------------------------------------------------------------------
	s3d::RectF Map::drawShadow(f64 x, f64 y, f64 scale) const
	{
		return drawShadow(s3d::Vec2{ x, y }, scale);
	}

	//-----------------------------------------------------------------------------
	//! Draw Shadow
	//-----------------------------------------------------------------------------
	s3d::RectF Map::drawShadow(const s3d::Vec2& ofs, f64 scale) const
	{
		const s3d::Vec2 sz{ s3d::Math::Round(CellSize * scale) };
		const f64 blur = 6.0 * scale;

		for(auto y : s3d::step(mCell.height)){
			for(auto x : s3d::step(mCell.width)){
				const Cell c = cellType(mCell[y][x]);
				if(c == Cell::Empty){
					continue;
				}
				else if(c == Cell::Earth || c == Cell::Wall || c == Cell::ClosedLift || c == Cell::OpenLift || (c == Cell::Robot && mRobotPos == mLiftPos)){
					s3d::RectF{ ofs + sz * s3d::Vec2{ x, y }, sz }.drawShadow({ blur, blur }, blur * 2.0);
				}
				else{
					s3d::Circle{ ofs + sz * s3d::Vec2{ x + 0.5, y + 0.5 }, sz.x * 0.5 }.drawShadow({ 0, 0 }, blur * 5.0);
				}
			}
		}

		return s3d::RectF{ ofs.x, ofs.y, sz.x * mCell.width, sz.y * mCell.height };
	}

	//-----------------------------------------------------------------------------
	//! Print
	//-----------------------------------------------------------------------------
	void Map::print() const
	{
		for(auto y : s3d::step(mCell.height)){
			for(auto x : s3d::step(mCell.width)){
				s3d::Print(stringOfCell(mCell[y][x]));
			}
			s3d::Print(L"\n");
		}
	}
#endif

}
