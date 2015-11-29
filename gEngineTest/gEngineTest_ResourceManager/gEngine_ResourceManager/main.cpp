
#include "gEngine\System\Handles\gEntryContainer.h"
#include <array>
#include <memory>
#include <vector>
#undef main

struct Foo
{
	int val_1;
	int val_2;

	Foo(int a, int b) : val_1(a), val_2(b) { DBGINIT; };
	~Foo(){ DBGDEST; };
};

struct Bar
{
	int val_3;

	Bar(int a) : val_3(a) { DBGINIT; };
	~Bar(){ DBGDEST; };
};

struct FooBar : public Foo, public Bar
{
	FooBar(Foo a) : Foo(a.val_1, a.val_2), Bar(0) { DBGINIT; };
	FooBar(Bar a) : Foo(0, 0), Bar(a.val_3) { DBGINIT; };
	FooBar(int a, int b) : Foo(a, b), Bar(0) { DBGINIT; };
	FooBar(int a, int b, int c) : Foo(a, b), Bar(c) { DBGINIT; };
	~FooBar(){ DBGDEST; };
};

int main(int argc, char* args[])
{	
	GENG::gEntryManager resManager;
	std::vector<GENG::gHandle> handles;

	resManager.addEntryType<Foo, 10>();
	resManager.addEntryType<Bar, 10>();
	resManager.addEntryType<FooBar, 10>();

	for (int i = 0; i < 10 ; i++)
	{
		int a = rand() % 10;
		int b = rand() % 10;
		int c = rand() % 10;
		handles.push_back(resManager.addEntry<Foo>(a, b));
		handles.push_back(resManager.addEntry<Bar>(c));
		handles.push_back(resManager.addEntry<FooBar>(a, b, c));
	}
	
	for (auto handle : handles)
	{
		auto pFoo = resManager.getSharedEntry<Foo>(handle);
		auto pBar = resManager.getSharedEntry<Bar>(handle);
		auto pFooBar = resManager.getSharedEntry<FooBar>(handle);
		if (pFoo != nullptr)
		{
			DBG("Foo " << pFoo->val_1 << " " << pFoo->val_2);
		}
		else if (pBar != nullptr)
		{
			DBG("Bar " << pBar->val_3);
		}
		else if (pFooBar != nullptr)
		{
			DBG("FooBar " << pFooBar->val_1 << " " << pFooBar->val_2 << " " << pFooBar->val_3);
		}
	}
}