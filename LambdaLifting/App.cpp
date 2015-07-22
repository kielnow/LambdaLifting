//
// App
//

#include "stdafx.h"
#include "App.h"

#include "Map.h"
#include "Command.h"

namespace {

	const s3d::wchar* CONFIG_FILE = L"config.ini";

}


namespace app
{

	//! ctor
	App::App()
		: mInitialMapPtr(new Map)
		, mMapPtr(new Map)
		, mUndoPos(0)
		, mInputPtr(new InputCommand)
		, mState(State::Stop)
	{
	}

	//! dtor
	App::~App()
	{
	}

	//-----------------------------------------------------------------------------
	//! Initialize
	//-----------------------------------------------------------------------------
	void App::initialize()
	{
		// ウィンドウ設定
		s3d::Window::SetTitle(L"Lambda Lifting");
		s3d::Window::SetStyle(s3d::WindowStyle::Sizeable);
		s3d::Window::Resize(1600, 900);

		//s3d::Graphics::SetBackground(s3d::Palette::White);

		// アセット登録
		s3d::TextureAsset::Register(L"cell", L"assets\\cell.png");
		s3d::FontAsset::Register(L"font", 8, L"Consolas", s3d::FontStyle::Bitmap);

		// GUIをセットアップ
		mGUIPtr = new s3d::GUI(s3d::GUIStyle::Default);
		mGUIPtr->setPos(900, 16);
		mGUIPtr->setTitle(L"Settings");

		mGUIPtr->addln(L"openMap", s3d::GUIButton::Create(L"Open Map"));
		mGUIPtr->addln(L"reset", s3d::GUIButton::Create(L"Reset"));

		mGUIPtr->add(L"hr1", s3d::GUIHorizontalLine::Create(1));
		mGUIPtr->horizontalLine(L"hr1").style.color = s3d::Palette::Gray;

		mGUIPtr->add(L"textScale", s3d::GUIText::Create(L"Scale"));
		mGUIPtr->addln(L"scale", s3d::GUISlider::Create(0.5, 1.5, 1.0));

		mGUIPtr->addln(L"dropShadow", s3d::GUICheckBox::Create({ L"Drop Shadow" }, { 0u }));

		mGUIPtr->addln(L"trail", s3d::GUICheckBox::Create({ L"Show Trail" }));
		mGUIPtr->add(L"textTrailLength", s3d::GUIText::Create(L"Trail Length"));
		mGUIPtr->addln(L"trailLength", s3d::GUISlider::Create(1.0, MAX_HISTORY, 100.0));

		mGUIPtr->add(L"hr2", s3d::GUIHorizontalLine::Create(1));
		mGUIPtr->horizontalLine(L"hr2").style.color = s3d::Palette::Gray;

		mGUIPtr->addln(L"textCommands", s3d::GUIText::Create(L"Commands:0"));
		//mGUIPtr->addln(L"commands", s3d::GUITextArea::Create(4, 20));
		mGUIPtr->addln(L"commands", s3d::GUITextArea::Create(4, 20));
		mGUIPtr->add(L"delete", s3d::GUIButton::Create(L"Delete"));
		mGUIPtr->add(L"copy", s3d::GUIButton::Create(L"Copy"));
		mGUIPtr->add(L"save", s3d::GUIButton::Create(L"Save"));
		mGUIPtr->addln(L"load", s3d::GUIButton::Create(L"Load"));

		mGUIPtr->add(L"play", s3d::GUIButton::Create(L"Play"));
		mGUIPtr->add(L"stop", s3d::GUIButton::Create(L"Stop", false));

		// INIファイルを読み込む
		s3d::INIReader ini(CONFIG_FILE);

		const s3d::String filepath{ ini.getOr<s3d::String>(L"Map.filepath", s3d::String{ L"map\\map1.txt" }) };
		load(filepath);

		loadCommands();
	}

	//-----------------------------------------------------------------------------
	//! Finalize
	//-----------------------------------------------------------------------------
	void App::finalize()
	{
		save();
	}

	//-----------------------------------------------------------------------------
	//! Load
	//-----------------------------------------------------------------------------
	bool App::load(const s3d::FilePath& filepath)
	{
		const bool result = mInitialMapPtr->load(filepath);
		mFilepath = filepath;
		mFilename = s3d::FileSystem::BaseName(filepath);
		reset();
		return result;
	}

	//-----------------------------------------------------------------------------
	//! Undo
	//-----------------------------------------------------------------------------
	void App::undo()
	{
		mUndoPos = std::min(mUndoPos + 1, (s32)mHistory.size() - 1);
		*mMapPtr = mHistory.at(mHistory.size() - 1 - mUndoPos);
	}

	//-----------------------------------------------------------------------------
	//! Redo
	//-----------------------------------------------------------------------------
	void App::redo()
	{
		mUndoPos = std::max(0, mUndoPos - 1);
		*mMapPtr = mHistory.at(mHistory.size() - 1 - mUndoPos);
	}

	//-----------------------------------------------------------------------------
	//! Reset
	//-----------------------------------------------------------------------------
	void App::reset()
	{
		*mMapPtr = *mInitialMapPtr;

		mCommands.clear();
		mValids.clear();

		mHistory.clear();
		mHistory.push_back(*mMapPtr);
		mUndoPos = 0;
	}

	//-----------------------------------------------------------------------------
	//! Update
	//-----------------------------------------------------------------------------
	void App::update()
	{
		updateGUI();

		Command cmd = Command::None;
		s3d::wchar wc = '?';

		if(isOperational()){
			// 履歴を操作
			if(s3d::Input::KeyControl.pressed){
				if(s3d::Input::KeyZ.clicked){
					undo();
				}
				else if(s3d::Input::KeyY.clicked){
					redo();
				}
			}
		}

		if(mState == State::Stop && isOperational()){
			// コマンド入力
			cmd = mInputPtr->command();
			wc = charOfCommand(cmd);
		}
		else if(mState == State::Play){
			if(mInputCommands.length - mCommands.length > 0){
				wc = mInputCommands[mCommands.length];
				cmd = commandOfChar(wc);
			}
			else{
				stop();
			}
		}

		bool valid = mMapPtr->update(cmd);

		bool bupdate = false;
		switch(mState){
		case State::Play:
			bupdate = true;
			break;
		case State::Stop:
		case State::Pause:
			bupdate = cmd != Command::None && valid;
			break;
		}

		if(bupdate){
			// 履歴をリサイズ
			if(mUndoPos > 0){
				mCommands.resize(mCommands.length - mUndoPos);
				mValids.resize(mValids.size() - mUndoPos);
				mHistory.resize(mHistory.size() - mUndoPos);
				mUndoPos = 0;
			}

			// コマンドを保存
			mCommands.push_back(wc);
			mValids.push_back(valid);

			// 履歴を保存
			mHistory.push_back(*mMapPtr);
			if(mHistory.size() > MAX_HISTORY){
				mHistory.pop_front();
			}
		}
	}

	//-----------------------------------------------------------------------------
	//! 操作可能か
	//-----------------------------------------------------------------------------
	bool App::isOperational() const
	{
		return !mGUIPtr->textArea(L"commands").active && (mState == State::Stop || mState == State::Pause);
	}

	//-----------------------------------------------------------------------------
	//! Update GUI
	//-----------------------------------------------------------------------------
	void App::updateGUI()
	{
		f64 scale = mGUIPtr->slider(L"scale").value;
		scale = static_cast<f64>(static_cast<s32>(scale * 10.0)) * 0.1f;
		mGUIPtr->slider(L"scale").setValue(scale);
		mGUIPtr->text(L"textScale").text = s3d::Format(s3d::PyFmt, L"Scale:{:.0f}%", scale * 100.0);

		f64 trailLength = mGUIPtr->slider(L"trailLength").value;
		trailLength = static_cast<f64>(static_cast<s32>(trailLength));
		mGUIPtr->slider(L"trailLength").setRightValue(static_cast<f64>((mCommands.length + 200) / 200 * 200));
		mGUIPtr->slider(L"trailLength").setValue(trailLength);
		mGUIPtr->text(L"textTrailLength").text = s3d::Format(s3d::PyFmt, L"Trail Length:{:.0f}", trailLength);

		if(mGUIPtr->button(L"openMap").pushed){
			stop();
			if(const auto open = s3d::Dialog::GetOpen({{ L"マップファイル (*.txt;*.map)", L"*.txt;*.map" }})){
				load(open.value());
			}
		}
		if(mGUIPtr->button(L"reset").pushed){
			stop();
			reset();
		}

		if(mGUIPtr->textArea(L"commands").hasChanged){
			mGUIPtr->text(L"textCommands").text = s3d::Format(s3d::PyFmt, L"Commands:{}", mGUIPtr->textArea(L"commands").text.length);
		}

		if(mGUIPtr->button(L"delete").pushed){
			mGUIPtr->textArea(L"commands").setText(L"");
			mGUIPtr->text(L"textCommands").text = L"Commands:0";
		}
		if(mGUIPtr->button(L"copy").pushed){
			s3d::Clipboard::SetText(mGUIPtr->textArea(L"commands").text);
		}
		if(mGUIPtr->button(L"save").pushed){
			save();
		}
		if(mGUIPtr->button(L"load").pushed){
			loadCommands();
		}

		if(mGUIPtr->button(L"play").pushed){
			switch(mState){
			case State::Play:
				pause();
				break;
			case State::Pause:
				resume();
				break;
			case State::Stop:
				play();
				break;
			}
		}

		if(mGUIPtr->button(L"stop").pushed){
			stop();
		}
	}

	//-----------------------------------------------------------------------------
	//! Play
	//-----------------------------------------------------------------------------
	void App::play()
	{
		s3d::GUITextAreaWrapper textArea = mGUIPtr->textArea(L"commands");

		mGUIPtr->button(L"play").text = L"Pause";

		textArea.enabled = false;
		mGUIPtr->button(L"delete").enabled = false;
		mGUIPtr->button(L"stop").enabled = true;

		mInputCommands = textArea.text;
		reset();

		mState = State::Play;
	}

	//-----------------------------------------------------------------------------
	//! Pause
	//-----------------------------------------------------------------------------
	void App::pause()
	{
		mGUIPtr->button(L"play").text = L"Resume";
		mState = State::Pause;
	}

	//-----------------------------------------------------------------------------
	//! Resume
	//-----------------------------------------------------------------------------
	void App::resume()
	{
		mGUIPtr->button(L"play").text = L"Pause";
		mState = State::Play;
	}

	//-----------------------------------------------------------------------------
	//! Stop
	//-----------------------------------------------------------------------------
	void App::stop()
	{
		if(mState == State::Stop){
			return;
		}

		mGUIPtr->button(L"play").text = L"Play";

		mGUIPtr->textArea(L"commands").enabled = true;
		mGUIPtr->button(L"delete").enabled = true;
		mGUIPtr->button(L"stop").enabled = false;
		mState = State::Stop;
	}

	//-----------------------------------------------------------------------------
	//! Save Commands
	//-----------------------------------------------------------------------------
	void App::save()
	{
		s3d::INIWriter ini(CONFIG_FILE);

		ini.write(L"Map", L"filepath", mFilepath);

		ini.write(L"GUI", L"commands", mGUIPtr->textArea(L"commands").text);
	}

	//-----------------------------------------------------------------------------
	//! Load Commands
	//-----------------------------------------------------------------------------
	void App::loadCommands()
	{
		s3d::INIReader ini(CONFIG_FILE);

		const auto commands = ini.getOpt<s3d::String>(L"GUI.commands");
		if(commands){
			mGUIPtr->textArea(L"commands").setText(commands.value());
		}
	}

	//-----------------------------------------------------------------------------
	//! Draw
	//-----------------------------------------------------------------------------
	void App::draw()
	{
		const s3d::Font font = s3d::FontAsset(L"font");

		s3d::RectF rect{ 0, 0, 0, 0 };

		// 情報を描画
		s3d::String s;
		s += s3d::Format(s3d::PyFmt, L"Map: {}  ", mFilename);
		s += s3d::Format(s3d::PyFmt, L"Size: ({},{})  ", mMapPtr->width(), mMapPtr->height());
		s += s3d::Format(s3d::PyFmt, L"Robot: ({},{})  ", mMapPtr->robotPos().x, mMapPtr->height() - mMapPtr->robotPos().y);
		s += s3d::Format(s3d::PyFmt, L"Lambda: {1}/{0}  ", mMapPtr->lambda(), mMapPtr->lambdaCollected());
		s += s3d::Format(s3d::PyFmt, L"StepCount: {}  ", mMapPtr->stepCount());
		s += s3d::Format(s3d::PyFmt, L"Score: {}  ", mMapPtr->score());
		s += s3d::Format(s3d::PyFmt, L"Condition: {}\n", stringOfCondition(mMapPtr->condition()));
		s += s3d::Format(s3d::PyFmt, L"Water: {}  ", mMapPtr->water());
		s += s3d::Format(s3d::PyFmt, L"Flooding: {1}/{0}  ", mMapPtr->flooding(), mMapPtr->floodingCount());
		s += s3d::Format(s3d::PyFmt, L"Waterproof: {1}/{0}\n", mMapPtr->waterproof(), mMapPtr->waterproofCount());
		s += s3d::Format(s3d::PyFmt, L"Growth: {1}/{0}  ", mMapPtr->growth(), mMapPtr->growthCount());
		s += s3d::Format(s3d::PyFmt, L"Razor: {}  ", mMapPtr->razor());
		s += s3d::Format(s3d::PyFmt, L"Beard: {}  ", mMapPtr->beard());
		rect = font.draw(s);

		// マップを描画
		const s3d::Vec2 ofs = rect.bl;
		const f64 scale = mGUIPtr->slider(L"scale").value;
		// ドロップシャドウを描画
		if(mGUIPtr->checkBox(L"dropShadow").checked(0))
		{
			mMapPtr->drawShadow(ofs, scale);
		}
		rect = mMapPtr->draw(ofs, scale);

		// 軌跡を描画
		if(mGUIPtr->checkBox(L"trail").checked(0)){
			s3d::Graphics2D::SetBlendState(s3d::BlendState::Additive);

			const s32 length = static_cast<s32>(mGUIPtr->slider(L"trailLength").value);
			const s3d::Vec2 half{ s3d::Math::Round(Map::CellSize * 0.5 * scale) };
			s3d::Vec2 pos1{ mMapPtr->robotPos() };
			s3d::Vec2 pos2{ pos1 };
			for(s32 i = 0; i < length; ++i){
				// 履歴からロボットの位置の取得
				s32 index = mHistory.size() - 1 - mUndoPos - i;
				if(index < 0){
					break;
				}
				pos2 = mHistory[index].robotPos();

				const s3d::Color color{ s3d::Math::Lerp(s3d::Palette::Red, s3d::Palette::Yellow, (f32)i / length) };
				s3d::Line(ofs + half + pos2 * Map::CellSize * scale, ofs + half + pos1 * Map::CellSize * scale).drawArrow(2, { 8, 16 }, color);
				pos1 = pos2;
			}

			s3d::Graphics2D::SetBlendState(s3d::BlendState::Default);
		}

		// コマンドを描画
		{
			s = s3d::Format(s3d::PyFmt, L"CommandCount: {}  ", mCommands.length - mUndoPos);
			rect = font.draw(s, rect.bl);

			s3d::wchar wstr[2] = {};
			s3d::Color color{ s3d::Palette::White };
			u32 x = 0, y = 0;
			for(u32 i = 0; i < mCommands.length; ++i){
				wstr[0] = mCommands[i];

				if(i >= mCommands.length - mUndoPos){
					color = s3d::Palette::Dimgray;
				}
				else{
					color = mValids[i] ? s3d::Palette::White : s3d::Palette::Red;
				}
				font.draw(wstr, s3d::Vec2(rect.x + (f64)font.size * x, rect.y + rect.h + (f64)font.size * 2.0 * y), color);
				if(i % 100 == 99){
					x = 0;
					y++;
				}
				else{
					x++;
				}
			}
		}

		//s3d::Circle(s3d::Mouse::Pos(), 50).draw({ 255, 0, 0, 127 });
	}
}