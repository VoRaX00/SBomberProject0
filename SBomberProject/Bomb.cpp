
#include <iostream>

#include "Bomb.h"
#include "MyTools.h"
#include "Visitor.h"

using namespace std;
using namespace MyTools;

void Bomb::Draw() const
{
    MyTools::SetColor(CC_LightMagenta);
    GotoXY(x, y);
    cout << "*";
}

void __fastcall Bomb::Accept(Visitor& v)
{
    v.log(*this);
}

void Bomb::AddObserver(DestroyableGroundObject* o)
{
    vecObs.push_back(o); 
}

void Bomb::RemoveObserver(DestroyableGroundObject* obj)
{
    auto it = find(vecObs.begin(), vecObs.end(), obj);
    vecObs.erase(it);
}
