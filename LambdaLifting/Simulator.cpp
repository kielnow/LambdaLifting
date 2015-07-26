//
// Simulator
//

#include "stdafx.h"
#include "Simulator.h"

#include "Map.h"

#define CHECK_REGISTER(x)	if(!x){return false;}

namespace {

	const s3d::Vec2 kCellSize{ 32, 32 };

} // unnamed namespace


namespace app
{

	//-----------------------------------------------------------------------------
	//! ctor
	//-----------------------------------------------------------------------------
	Simulator::Simulator()
		: mpMapInfo(nullptr)
		, mpInitialMap(nullptr)
		, mHistoryPos(0)
	{
		mpMapInfo = new MapInfo;

		mpInitialMap = new Map;
		mpInitialMap->info = mpMapInfo;

		mpMap = new Map;
		mpMap->info = mpMapInfo;
	}

	//-----------------------------------------------------------------------------
	//! dtor
	//-----------------------------------------------------------------------------
	Simulator::~Simulator()
	{
		clear();

		delete mpMap;
		delete mpInitialMap;
		delete mpMapInfo;
	}

	//-----------------------------------------------------------------------------
	//! クリア
	//-----------------------------------------------------------------------------
	void Simulator::clear(bool all)
	{
		if (all) {
			mFilePath.clear();
			mFileName.clear();
			mpMapInfo->clear();
			mpInitialMap->clear();
		}

		mpMap->clear();

		mCommands.clear();
		mValids.clear();
		mCommandPos = 0;

		for (auto* p : mHistory)
			delete p;
		mHistory.clear();
		mHistoryPos = 0;
	}

	//-----------------------------------------------------------------------------
	//! マップを読み込む
	//-----------------------------------------------------------------------------
	bool Simulator::loadMap(const s3d::String& filepath)
	{
		clear();
		mFilePath = filepath;
		mFileName = s3d::FileSystem::BaseName(filepath);

		if (!loadMap(filepath, *mpMapInfo, *mpInitialMap)) {
			clear();
			return false;
		}

		reset();
		return true;
	}

	//-----------------------------------------------------------------------------
	//! マップを読み込む
	//-----------------------------------------------------------------------------
	bool Simulator::loadMap(const s3d::String& filepath, struct MapInfo& mapInfo, struct Map& map)
	{
		s3d::TextReader r(filepath);
		if (!r.isOpened())
			return false;

		const s3d::String s = r.readContents();

		u32 w = 0, h = 0;
		bool bend = false;
		for (const auto& line : s.split(L'\n')) {
			// マップサイズを調べる
			if (!bend) {
				if (line[0] == L'\0' || line.length == 1) {
					bend = true;
					continue;
				}
				w = std::max(w, line.length);
				h++;
				continue;
			}

			// マップ属性を調べる
			if (line.startsWith(L"Water "))
			{
				map.water = s3d::Parse<u32>(line.split(' ').at(1));
			} else if (line.startsWith(L"Flooding "))
			{
				const u32 f = s3d::Parse<u32>(line.split(' ').at(1));
				mapInfo.flooding = f;
				map.floodingCount = f - 1;
			} else if (line.startsWith(L"Waterproof "))
			{
				const u32 w = s3d::Parse<u32>(line.split(' ').at(1));

				mapInfo.waterproof = w;
				map.waterproofCount = w;
			} else if (line.startsWith(L"Trampoline "))
			{
				auto words = line.split(' ');
				const u8 from = static_cast<u8>(s3d::Parse<s3d::wchar>(words.at(1)) - L'A');
				const u8 to = static_cast<u8>(s3d::Parse<s3d::wchar>(words.at(3)) - L'1');
				mapInfo.jump[from] = to;
			} else if (line.startsWith(L"Growth "))
			{
				const u32 g = s3d::Parse<u32>(line.split(' ').at(1));
				mapInfo.growth = g;
				map.growthCount = g - 1;
			} else if (line.startsWith(L"Razor "))
			{
				map.razor = s3d::Parse<u32>(line.split(' ').at(1));
			}
		}

		if (!w && !h) {
			// マップサイズが0なら読み込み失敗
			return false;
		}

		map.cell.resize(w, h, Cell::Empty);

		u32 x = 0, y = 0;
		for (const auto c : s) {
			if (c == L'\n') {
				if (x == 0)
					break;
				x = 0;
				y++;
				continue;
			}

			Cell& cell = map.cell[y][x];
			switch (c) {
			case L' ':
				cell = Cell::Empty;
				break;
			case L'R':
				cell = Cell::Robot;
				map.robotPos.set(x, y);
				break;
			case L'*':
				cell = Cell::Rock;
				break;
			case L'\\':
				cell = Cell::Lambda;
				map.lambda++;
				break;
			case L'.':
				cell = Cell::Earth;
				break;
			case L'#':
				cell = Cell::Wall;
				break;
			case L'L':
				cell = Cell::ClosedLift;
				mapInfo.liftPos.set(x, y);
				break;
			case L'O':
				cell = Cell::OpenLift;
				mapInfo.liftPos.set(x, y);
				break;
			case L'W':
				cell = Cell::Beard;
				map.beard++;
				break;
			case L'!':
				cell = Cell::Razor;
				break;
			case L'@':
				cell = Cell::HORock;
				map.lambda++;
				break;
			default:
				if (L'A' <= c && c <= L'I')
				{
					const u32 label = c - L'A';
					cell = static_cast<Cell>(MAKE_LABELED_CELL(Cell::Trampoline, label));
					mapInfo.trampolinePos[label].set(x, y);
				}
				else if (L'1' <= c && c <= L'9')
				{
					const u32 label = c - L'1';
					cell = static_cast<Cell>(MAKE_LABELED_CELL(Cell::Target, label));
					mapInfo.targetPos[label].set(x, y);
				}
				else
				{
					// 想定外の文字があった場合は読み込み失敗
					return false;
				}
				break;
			}

			x++;
		}

		return true;
	}


	//-----------------------------------------------------------------------------
	//! マップを取得
	//-----------------------------------------------------------------------------
	const struct Map& Simulator::getMap() const
	{
		return mHistoryPos == mHistory.size() ? *mpMap : *mHistory[mHistoryPos - 1];
	}

	//-----------------------------------------------------------------------------
	//! プレイ中か
	//-----------------------------------------------------------------------------
	bool Simulator::isPlaying() const
	{
		return getMap().condition == Condition::Playing;
	}


#pragma region Update Operation

	//-----------------------------------------------------------------------------
	//! リセット
	//-----------------------------------------------------------------------------
	void Simulator::reset()
	{
		clear(false);
		*mpMap = *mpInitialMap;
		mHistory.push_back(new Map(*mpInitialMap));
		mHistoryPos++;
	}

	//-----------------------------------------------------------------------------
	//! Undo
	//-----------------------------------------------------------------------------
	bool Simulator::undo(u32 step)
	{
		if (mCommandPos > 0 && mHistoryPos > 0) {
			step = std::max(1u, step);

			for (s32 i = mCommandPos - 1; i >= 0; --i) {
				mCommandPos--;
				if (mValids[i]) {
					mHistoryPos--;
				}
				if (--step == 0)
					break;
			}
			return true;
		}
		return false;
	}

	//-----------------------------------------------------------------------------
	//! Redo
	//-----------------------------------------------------------------------------
	bool Simulator::redo(u32 step)
	{
		if (mCommandPos < mCommands.length) {
			step = std::min(step, mCommands.length);

			for (u32 i = mCommandPos; i < mCommands.length; ++i) {
				mCommandPos++;
				if (mValids[i]) {
					mHistoryPos++;
				}
				if (--step == 0)
					break;
			}
			return true;
		}
		return false;
	}

	//-----------------------------------------------------------------------------
	//! Undo可能か
	//-----------------------------------------------------------------------------
	bool Simulator::undoable() const
	{
		return mCommandPos > 0;
	}

	//-----------------------------------------------------------------------------
	//! Redo可能か
	//-----------------------------------------------------------------------------
	bool Simulator::redoable() const
	{
		return mCommandPos < mCommands.length;
	}

	//-----------------------------------------------------------------------------
	//! 履歴をプッシュ
	//-----------------------------------------------------------------------------
	void Simulator::pushHistory(s3d::wchar cmd, struct Map* pmap)
	{
		mCommands += cmd;
		mValids.push_back(pmap != nullptr);
		mCommandPos++;
		if (pmap) {
			mHistory.push_back(new Map(*pmap));
			mHistoryPos++;
		}
	}

	//-----------------------------------------------------------------------------
	//! 履歴から再開
	//-----------------------------------------------------------------------------
	bool Simulator::resumeHistory()
	{
		if (mCommandPos != mCommands.length) {
			mCommands.resize(mCommandPos);
			mValids.resize(mCommandPos);

			if (mHistoryPos != mHistory.size()) {
				*mpMap = *mHistory[mHistoryPos - 1];

				for (u32 i = mHistoryPos; i < mHistory.size(); ++i)
					delete mHistory[i];
				mHistory.resize(mHistoryPos);
				return true;
			}
		}
		return false;
	}

	//-----------------------------------------------------------------------------
	//! 連続実行
	//-----------------------------------------------------------------------------
	void Simulator::run(const s3d::String& cmds)
	{
		for (auto c : cmds) {
			step(c);
		}
	}

	//-----------------------------------------------------------------------------
	//! ステップ実行
	//-----------------------------------------------------------------------------
	bool Simulator::step(s3d::wchar c)
	{
		resumeHistory();

		const Command cmd = commandOfChar(c);
		if (cmd == Command::None) {
			pushHistory(c, nullptr);
			return false;
		}

		bool result = step(cmd, *mpMap);
		pushHistory(c, result ? mpMap : nullptr);
		return result;
	}

	//-----------------------------------------------------------------------------
	//! ステップ実行
	//-----------------------------------------------------------------------------
	bool Simulator::step(Command cmd)
	{
		return step(charOfCommand(cmd));
	}

	//-----------------------------------------------------------------------------
	//! ステップ実行
	//-----------------------------------------------------------------------------
	bool Simulator::step(Command cmd, struct Map& map)
	{
		if (map.condition != Condition::Playing)
			return false;

		// ロボット更新
		bool result = updateRobot(cmd, map);

		// マップ更新
		const u32 count = updateMap(map);
		if (cmd == Command::Wait) {
			result = count > 0;
		} else {
			result |= count > 0;
		}

		// 洪水更新
		result |= updateFlooding(map);

		// 髭更新
		result |= updateBeard(map);

		return result;
	}


	//-----------------------------------------------------------------------------
	//! ロボット更新
	//-----------------------------------------------------------------------------
	bool Simulator::updateRobot(Command cmd, struct Map& map)
	{
		bool result = false;

		switch (cmd) {
		case Command::Up:
		case Command::Down:
		case Command::Left:
		case Command::Right:
			result = moveRobot(cmd, map);
			break;
		case Command::Wait:
			result = true;
			break;
		case Command::Abort:
			map.condition = Condition::Abort;
			map.score += 25 * map.lambdaCollected;
			result = true;
			break;
		case Command::Shave:
			if (map.razor > 0) {
				const s3d::Rect rect{ 0, 0, map.cell.width, map.cell.height };

				for (s32 dy : {-1, 0, 1}) {
					for (s32 dx : {-1, 0, 1}) {
						if (!dx && !dy)
							continue;

						const int2 adjPos{ map.robotPos.movedBy(dx, dy) };

						// マップ範囲外チェック
						if (!adjPos.intersects(rect))
							continue;

						if (map.cell[adjPos.y][adjPos.x] == Cell::Beard) {
							map.cell[adjPos.y][adjPos.x] = Cell::Empty;
							map.beard--;
						}
					}
				}

				// 髭がすべて無くなったらgrowthCountを0に
				if (map.beard == 0) {
					map.growthCount = 0;
				}

				map.razor--;
				map.step();
				result = false;
			}
			break;
		default:
			break;
		}

		return result;
	}

	//-----------------------------------------------------------------------------
	//! ロボット移動
	//-----------------------------------------------------------------------------
	bool Simulator::moveRobot(Command cmd, struct Map& map)
	{
		const s3d::Rect rect{ 0, 0, map.cell.width, map.cell.height };

		int2 newPos{ map.robotPos };
		switch (cmd) {
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
		if (!newPos.intersects(rect))
			return false;

		// 移動できたか
		bool valid = false;

		const Cell lc = map.cell[newPos.y][newPos.x];
		const Cell c = cellType(lc);
		if (c == Cell::Empty || c == Cell::Earth || c == Cell::Lambda || c == Cell::OpenLift || c == Cell::Trampoline || c == Cell::Razor) {
			map.cell[map.robotPos.y][map.robotPos.x] = Cell::Empty;
			map.cell[newPos.y][newPos.x] = Cell::Robot;
			map.robotPos = newPos;
			map.step();
			valid = true;

			switch (c) {
			case Cell::Lambda:
				map.lambdaCollected++;
				map.score += 25;
				break;
			case Cell::OpenLift:
				map.condition = Condition::Winning;
				map.score += 50 * map.lambdaCollected;
				break;
			case Cell::Trampoline:
			{
				const u8 label = cellLabel(lc);
				const u8 target = map.info->jump[label];
				const int2 jumpPos{ map.info->targetPos[target] };

				map.cell[newPos.y][newPos.x] = Cell::Empty;
				map.cell[jumpPos.y][jumpPos.x] = Cell::Robot;
				map.robotPos = jumpPos;

				// ターゲットに関連付けられていたトランポリンを消去
				for (u32 i = 0; i < MAX_TRAMPOLINE; ++i) {
					if (map.info->jump[i] == target) {
						const auto& pos = map.info->trampolinePos[i];
						map.cell[pos.y][pos.x] = Cell::Empty;
					}
				}
			}
			break;
			case Cell::Razor:
				map.razor++;
				break;
			}
		} else if ((c == Cell::Rock || c == Cell::HORock) && (cmd == Command::Left || cmd == Command::Right)) {
			const int2 nextPos{ newPos.movedBy(cmd == Command::Left ? -1 : 1, 0) };

			// マップ範囲外チェック
			if (!nextPos.intersects(rect))
				return false;

			if (map.cell[nextPos.y][nextPos.x] == Cell::Empty) {
				map.cell[map.robotPos.y][map.robotPos.x] = Cell::Empty;
				map.cell[newPos.y][newPos.x] = Cell::Robot;
				map.cell[nextPos.y][nextPos.x] = c;
				map.robotPos = newPos;
				map.step();
				valid = true;
			}
		}

		return valid;
	}

	//-----------------------------------------------------------------------------
	//! マップ更新
	//-----------------------------------------------------------------------------
	u32 Simulator::updateMap(struct Map& map)
	{
		const s3d::Grid<Cell> old{ map.cell };

		u32 count = 0;

		const s32 w = old.width, h = old.height;
		for (auto y : s3d::step_backward(h)) {
			for (auto x : s3d::step(w)) {
				const Cell lc = old[y][x];
				const Cell c = cellType(lc);

				// リフトの場合
				if (c == Cell::ClosedLift && map.lambdaCollected == map.lambda) {
					map.cell[y][x] = Cell::OpenLift;
					count++;
				}
				// 岩の場合
				else if (y + 1 < h && (c == Cell::Rock || c == Cell::HORock)) {
					int2 newPos{ x, y };
					switch (old[y + 1][x]) {
					case Cell::Empty:
						newPos.set(x, y + 1);
						break;
					case Cell::Rock:
					case Cell::HORock:
						if (x + 1 < w && old[y][x + 1] == Cell::Empty && old[y + 1][x + 1] == Cell::Empty) {
							newPos.set(x + 1, y + 1);
						} else if (0 <= x - 1 && old[y][x - 1] == Cell::Empty && old[y + 1][x - 1] == Cell::Empty) {
							newPos.set(x - 1, y + 1);
						}
						break;
					case Cell::Lambda:
						if (x + 1 < w && old[y][x + 1] == Cell::Empty && old[y + 1][x + 1] == Cell::Empty) {
							newPos.set(x + 1, y + 1);
						}
						break;
					default:
						// nop
						break;
					}

					// 岩が落下したか
					if (x != newPos.x || y != newPos.y) {
						// ラムダ消滅チェック
						if (map.cell[newPos.y][newPos.x] == Cell::Lambda) {
							map.lambda--;
						}

						map.cell[y][x] = Cell::Empty;
						map.cell[newPos.y][newPos.x] = c;
						count++;

						if (c == Cell::HORock && newPos.y + 1 < h && old[newPos.y + 1][newPos.x] != Cell::Empty) {
							map.cell[newPos.y][newPos.x] = Cell::Lambda;
						}
					}
				}
				// 髭の場合
				else if (old[y][x] == Cell::Beard) {
					if (map.growthCount == 0) {
						const s3d::Rect rect{ 0, 0, w, h };

						for (auto dy : { -1, 0, 1 }) {
							for (auto dx : { -1, 0, 1 }) {
								if (!dx && !dy)
									continue;

								const int2 adjPos{ x + dx, y + dy };

								// マップ範囲外チェック
								if (!adjPos.intersects(rect))
									continue;

								if (old[adjPos.y][adjPos.x] == Cell::Empty) {
									// ラムダ消滅チェック
									if (map.cell[adjPos.y][adjPos.x] == Cell::Lambda) {
										map.lambda--;
									}

									map.cell[adjPos.y][adjPos.x] = Cell::Beard;
									map.beard++;
									count++;
								}
							}
						}
					}
				}
			}
		}

		// ロボットが破壊されたかチェック
		const int2 robotPos{ map.robotPos };
		if (map.condition == Condition::Playing &&
			robotPos.y - 1 >= 0 &&
			old[robotPos.y - 1][robotPos.x] != Cell::Rock &&
			map.cell[robotPos.y - 1][robotPos.x] == Cell::Rock)
		{
			map.condition = Condition::Losing;
		}

		return count;
	}

	//-----------------------------------------------------------------------------
	//! 洪水更新
	//-----------------------------------------------------------------------------
	bool Simulator::updateFlooding(struct Map& map)
	{
		bool result = false;

		if (map.info->flooding > 0) {
			if (map.floodingCount == 0) {
				map.floodingCount = map.info->flooding;
				map.water = std::min(map.water + 1, (u32)map.cell.height);
			}
			map.floodingCount--;
			result = true;
		}

		// ロボットが水没したかチェック
		if (map.condition == Condition::Playing) {
			if (map.cell.height - (s32)map.water <= map.robotPos.y) {
				if (map.waterproofCount == 0) {
					map.condition = Condition::Losing;
				} else {
					map.waterproofCount--;
				}
				result = true;
			} else if (map.waterproofCount < map.info->waterproof) {
				map.waterproofCount = map.info->waterproof;
				result = true;
			}
		}

		return result;
	}

	//-----------------------------------------------------------------------------
	//! 髭更新
	//-----------------------------------------------------------------------------
	bool Simulator::updateBeard(struct Map& map)
	{
		if (map.beard > 0) {
			if (map.growthCount == 0) {
				map.growthCount = map.info->growth;
			}
			map.growthCount--;
			return true;
		}
		return false;
	}

#pragma endregion

#pragma region Draw Operation

	//-----------------------------------------------------------------------------
	//! アセットをロード
	//-----------------------------------------------------------------------------
	bool Simulator::loadAsset()
	{
		CHECK_REGISTER(s3d::TextureAsset::Register(L"cell", L"assets\\cell.png"));
		CHECK_REGISTER(s3d::FontAsset::Register(L"font", 8, L"Consolas", s3d::FontStyle::Bitmap));
		return true;
	}

	//-----------------------------------------------------------------------------
	//! 描画
	//-----------------------------------------------------------------------------
	void Simulator::draw(const s3d::Vec2& pos, f64 scale) const
	{
		const s3d::Texture tex{ s3d::TextureAsset(L"cell") };

		const Map& map{ getMap() };
		const s3d::Vec2 sz{ s3d::Math::Round(kCellSize * scale) };

		// ポイントサンプリングに
		s3d::Graphics2D::SetSamplerState(s3d::SamplerState::WrapPoint);

		s3d::RectF rect{ kCellSize.asPoint() };
		for (auto y : s3d::step(map.cell.height)) {
			for (auto x : s3d::step(map.cell.width)) {
				const Cell lc = map.cell[y][x];
				const Cell c = cellType(lc);

				switch (c) {
				case Cell::Empty:
					break;

				case Cell::Robot:
					if (map.robotPos == map.info->liftPos) {
						rect.setPos(kCellSize.x * static_cast<u32>(Cell::OpenLift), 0);
						tex(rect).resize(sz).draw(pos + sz * s3d::Vec2{ x, y });
					}

					rect.setPos(kCellSize.x * static_cast<u32>(map.condition), kCellSize.y);
					tex(rect).resize(sz).draw(pos + sz * s3d::Vec2{ x, y });
					break;

				default:
					rect.setPos(kCellSize.x * static_cast<u32>(c), 0);
					tex(rect).resize(sz).draw(pos + sz * s3d::Vec2{ x, y });

					if (c == Cell::Trampoline)
					{
						const u8 label = cellLabel(lc);
						rect.setPos(kCellSize.x * label, kCellSize.y * 2);
						tex(rect).resize(sz).draw(pos + sz * s3d::Vec2{ x, y });
						const u8 target = map.info->jump[label];
						rect.setPos(kCellSize.x * target, kCellSize.y * 3);
						tex(rect).resize(sz).draw(pos + sz * s3d::Vec2{ x, y });
					} else if (c == Cell::Target)
					{
						const u8 label = cellLabel(lc);
						rect.setPos(kCellSize.x * label, kCellSize.y * 3);
						tex(rect).resize(sz).draw(pos + sz * s3d::Vec2{ x, y });
					}
					break;
				}
			}
		}

		// リニアサンプリングに戻す
		s3d::Graphics2D::SetSamplerState(s3d::SamplerState::Default2D);

		// 水を描画
		s3d::Color waterColor{ s3d::Palette::Blue };
		waterColor.a = 96;
		s3d::RectF{ pos.x, pos.y + sz.y * (map.cell.height - map.water), sz.x * map.cell.width, sz.y * map.water }.draw(waterColor);
	}

	//-----------------------------------------------------------------------------
	//! 影を描画
	//-----------------------------------------------------------------------------
	void Simulator::drawShadow(const s3d::Vec2& pos, f64 scale) const
	{
		const Map& map{ getMap() };
		const s3d::Vec2 sz{ s3d::Math::Round(kCellSize * scale) };
		const f64 blur = 6.0 * scale;

		for (auto y : s3d::step(map.cell.height)){
			for (auto x : s3d::step(map.cell.width)){
				Cell c = cellType(map.cell[y][x]);

				if (c == Cell::Robot && map.robotPos == map.info->liftPos) {
					c = Cell::OpenLift;
				}

				switch (c) {
				case Cell::Empty:
					break;

				case Cell::Earth:
				case Cell::Wall:
				case Cell::ClosedLift:
				case Cell::OpenLift:
					s3d::RectF{ pos + sz * s3d::Vec2{ x, y }, sz }.drawShadow({ blur, blur }, blur * 2);

				default:
					s3d::Circle{ pos + sz * s3d::Vec2{ x + 0.5, y + 0.5 }, sz.x * 0.5 }.drawShadow({ 0, 0 }, blur * 5.0);
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	//! 軌跡を描画
	//-----------------------------------------------------------------------------
	void Simulator::drawTrail(s32 length, const s3d::Vec2& pos, f64 scale) const
	{
		const s3d::Vec2 half{ s3d::Math::Round(kCellSize * 0.5 * scale) };
		length = length < 0 ? mHistoryPos - 1 : std::min((u32)length, mHistoryPos - 1);

		s3d::Graphics2D::SetBlendState(s3d::BlendState::Additive);

		s3d::Vec2 pos1{ getMap().robotPos };
		s3d::Vec2 pos2{ pos1 };
		for(s32 i = 0; i < length; ++i){
			// 履歴からロボットの位置の取得
			pos2 = mHistory[mHistoryPos - 2 - i]->robotPos;

			s3d::Color color{ s3d::Math::Lerp(s3d::Palette::Red, s3d::Palette::Yellow, (f64)i / length) };
			color.a = 255 * s3d::Math::Lerp(1.0, 0.5, (f64)i / length);
			s3d::Line(pos + half + pos2 * kCellSize * scale, pos + half + pos1 * kCellSize * scale).drawArrow(2, { 8, 16 }, color);
			pos1 = pos2;
		}

		s3d::Graphics2D::SetBlendState(s3d::BlendState::Default);
	}

	//-----------------------------------------------------------------------------
	//! 描画矩形を取得
	//-----------------------------------------------------------------------------
	s3d::RectF Simulator::calcRect(const s3d::Vec2& pos, f64 scale) const
	{
		const Map& map{ getMap() };
		const s3d::Vec2 sz{ s3d::Math::Round(kCellSize * scale) };
		return s3d::RectF{ pos.x, pos.y, sz.x * map.cell.width, sz.y * map.cell.height };
	}

	//-----------------------------------------------------------------------------
	//! マップ情報を描画
	//-----------------------------------------------------------------------------
	s3d::RectF Simulator::drawMapInfo(const s3d::Vec2& pos, const s3d::Color& color) const
	{
		const s3d::Font font = s3d::FontAsset(L"font");
		const Map& map{ getMap() };

		s3d::String s;
		s += s3d::Format(s3d::PyFmt, L"Map: {}  ", mFileName);
		s += s3d::Format(s3d::PyFmt, L"Size: ({},{})  ", map.cell.width, map.cell.height);
		s += s3d::Format(s3d::PyFmt, L"Robot: ({},{})  ", map.robotPos.x, map.cell.height - map.robotPos.y);
		s += s3d::Format(s3d::PyFmt, L"Lambda: {1}/{0}  ", map.lambda, map.lambdaCollected);
		s += s3d::Format(s3d::PyFmt, L"StepCount: {}  ", map.stepCount);
		s += s3d::Format(s3d::PyFmt, L"Score: {}  ", map.score);
		s += s3d::Format(s3d::PyFmt, L"Condition: {}\n", stringOfCondition(map.condition));
		s += s3d::Format(s3d::PyFmt, L"Water: {}  ", map.water);
		s += s3d::Format(s3d::PyFmt, L"Flooding: {1}/{0}  ", map.info->flooding, map.floodingCount);
		s += s3d::Format(s3d::PyFmt, L"Waterproof: {1}/{0}\n", map.info->waterproof, map.waterproofCount);
		s += s3d::Format(s3d::PyFmt, L"Growth: {1}/{0}  ", map.info->growth, map.growthCount);
		s += s3d::Format(s3d::PyFmt, L"Razor: {}  ", map.razor);
		s += s3d::Format(s3d::PyFmt, L"Beard: {}  ", map.beard);

		return font.draw(s, pos, color);
	}

	//-----------------------------------------------------------------------------
	//! コマンドを描画
	//-----------------------------------------------------------------------------
	s3d::RectF Simulator::drawCommands(const s3d::Vec2& pos, const s3d::Color& _color) const
	{
		const s3d::Font font = s3d::FontAsset(L"font");
		const s32 d = mHistory.size() - mHistoryPos;
		const f64 w = font.size, h = font.size * 2;

		s3d::RectF rect;

		const s3d::String s{ s3d::Format(s3d::PyFmt, L"CommandCount: {}  ", mCommands.length - d) };
		rect = font.draw(s, pos, _color);

		s3d::wchar wstr[2] = {};
		s3d::Color color{ _color };
		u32 x = 0, y = 0;
		for (u32 i = 0; i < mCommands.length; ++i) {
			wstr[0] = mCommands[i];

			if (i >= mCommands.length - d) {
				color = s3d::Palette::Dimgray;
			} else {
				color = mValids[i] ? _color : s3d::Palette::Red;
			}

			font.draw(wstr, s3d::Vec2(rect.x + w * x, rect.y + rect.h + h * y), color);

			if(i % 100 == 99){
				x = 0;
				y++;
			}
			else{
				x++;
			}
		}

		u32 cw = mCommands.length % 100, ch = (mCommands.length + 99) / 100;
		return s3d::RectF{ rect.x, rect.y, std::max(rect.w, w * cw), rect.h + h * ch };
	}

#pragma endregion

} // namespace app