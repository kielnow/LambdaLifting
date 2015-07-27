//
// Solver
//

#pragma once

namespace app
{

	//===================================================================================
	//! @class Solver
	//===================================================================================
	class Solver
	{
	public:
		Solver();
		~Solver();

		s3d::String solve(const s3d::String& filepath);

	private:
		bool loadMap(const s3d::String& filepath);

		s3d::String bfs(class Simulator& simulator);

	private:
		std::unique_ptr<class Simulator> mpSimulator;
	};

} // namespace app