#pragma once
#include <array>
namespace MyUtil
{
	class Input
	{
	private:
		std::array<bool, 256> mKeys;
		int mCursorX, mCursorY;

	public:
		void Initialize();

		void SetKeyDown(unsigned int key);
		void SetKeyUP(unsigned int key);
		void SetCursor(int x, int y);

		bool IsKeyDown(unsigned int key);
		void GetCursor(int& x, int& y);
	};
}


