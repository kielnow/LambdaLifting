//
// Solver
//

#include "stdafx.h"
#include "Solver.h"

#include "Map.h"
#include "Simulator.h"

#include <queue>

namespace {

	const s3d::wchar* TAG = L"[LambdaLifting]";

	template <typename T, size_t N>
	size_t array_length(const T(&)[N]){ return N; }

	const app::Command commands[] = {
		app::Command::Wait,
		app::Command::Up,
		app::Command::Down,
		app::Command::Left,
		app::Command::Right,
		//app::Command::Abort,
		app::Command::Shave,
	};

	const app::Command getCommandInv(app::Command cmd)
	{
		switch(cmd){
		case app::Command::Wait:	return app::Command::None;
		case app::Command::Up:		return app::Command::Down;
		case app::Command::Down:	return app::Command::Up;
		case app::Command::Left:	return app::Command::Right;
		case app::Command::Right:	return app::Command::Left;
		case app::Command::Abort:	return app::Command::None;
		case app::Command::Shave:	return app::Command::None;
		}
		return app::Command::None;
	};

} // unnamed namespace


namespace app
{

	//===================================================================================
	//! @class State
	//===================================================================================
	struct State
	{
	public:
		Simulator simulator;
		s3d::Grid<bool> pass;

		State(const State& state)
			: simulator(state.simulator), pass(state.pass)
		{
			const auto& map = simulator.getMap();
			pass[map.robotPos.y][map.robotPos.x] = true;
		}

		State(const Simulator& _simulator)
			: simulator(_simulator)
		{
			const auto& map = simulator.getMap();
			pass.resize(map.cell.width, map.cell.height, false);
			pass[map.robotPos.y][map.robotPos.x] = true;
		}
	};



	//-----------------------------------------------------------------------------
	//! ctor
	//-----------------------------------------------------------------------------
	Solver::Solver()
		: mpSimulator(new Simulator)
	{
	}

	//-----------------------------------------------------------------------------
	//! dtor
	//-----------------------------------------------------------------------------
	Solver::~Solver()
	{
	}

	//-----------------------------------------------------------------------------
	//! マップを読み込む
	//-----------------------------------------------------------------------------
	bool Solver::loadMap(const s3d::String& filepath)
	{
		if (mpSimulator->loadMap(filepath)) {
			return true;
		}
		LOG(TAG, L"マップファイルの読み込みに失敗しました。", filepath);
		return false;

	}

	//-----------------------------------------------------------------------------
	//! Solve
	//-----------------------------------------------------------------------------
	s3d::String Solver::solve(const s3d::String& filepath)
	{
		if (!loadMap(filepath)) {
			return L"";
		}

		return bfs(*mpSimulator);
	}

	s3d::String Solver::bfs(class Simulator& simulator)
	{
		s3d::String cmds;
		s32 highScore = 0;

		State* pstate{ new State(simulator) };
		std::queue<State*> q;
		q.push(pstate);

		while (!q.empty())
		{
			State* ps{ q.front() };
			q.pop();

			if (ps->simulator.getMap().condition == Condition::Winning) {
				cmds = ps->simulator.getCommands();
				break;
			}

			// ステートの数が爆発しないように
			while (q.size() > 256) {
				delete q.front();
				q.pop();
			}

			std::queue<State*> newq;

			for (Command cmd : commands)
			{
				const u32 lambdaCollected = ps->simulator.getMap().lambdaCollected;

				const bool result = ps->simulator.step(cmd);
				const auto& map = ps->simulator.getMap();

				// その時点の最高得点のコマンドを覚えておく
				if (map.score > highScore) {
					highScore = map.score;
					cmds = ps->simulator.getCommands() + L'A';
				}

				if (result && 
					map.condition != Condition::Losing &&
					(/*cmd == Command::Wait || */!ps->pass[map.robotPos.y][map.robotPos.x]))
				{
					State* const pnew = new State(*ps);

					if (map.lambdaCollected > lambdaCollected)
					{
						pnew->pass.fill(false);
						pnew->pass[map.robotPos.y][map.robotPos.x] = true;

#if 0
						while (!q.empty()) {
							delete q.front();
							q.pop();
						}
#endif

						q.push(pnew);
						break;
					}
					else
					{
						pnew->pass[map.robotPos.y][map.robotPos.x] = true;
						q.push(pnew);
					}
				}
				ps->simulator.undo();
			}

			delete ps;
		}

		while (!q.empty()) {
			delete q.front();
			q.pop();
		}

		return cmds;
	}

} // namespace app