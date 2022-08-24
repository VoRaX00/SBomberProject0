
#include <conio.h>
#include <windows.h>

#include "MyTools.h"
#include "SBomber.h"
#include "Bomb.h"
#include "Ground.h"
#include "Tank.h"
#include "House.h"
#include <time.h>
#include <memory>
using namespace std;
using namespace MyTools;

//class IFactory
//{
//public:
//    virtual ~IFactory() {}
//    DynamicObject* createBomb(const double x, const double y) const
//    {
//        Bomb* pBomb = createBombInstance();
//        pBomb->SetDirection(0.3, 1);
//        pBomb->SetSpeed(2);
//        pBomb->SetPos(x, y);
//        pBomb->SetWidth(SMALL_CRATER_SIZE);
//        return pBomb;
//    }
//private:
//    virtual Bomb* createBombInstance() const = 0;
//};
//
//class RegularFactory : public IFactory
//{
//    Bomb* createBombInstance() const override
//    {
//        return new Bomb;
//    }
//};

void CommandDropBomb::setParams(Plane* plane, uint16_t *countBomb, int16_t *score)
{
    m_plane = plane;
    m_countBomb = countBomb;
    m_score = score;
}

void CommandDropBomb::Execute()
{
    if (m_countBomb > 0)
    {
        WriteToLog(string(__FUNCTION__) + " was invoked");
        double x = m_plane->GetX() + 4;
        double y = m_plane->GetY() + 2;

        srand(time(0));
        if (rand() % 2 == 1)
        {
            /*Bomb* pBomb = new Bomb;
            BombDecorator m_Bomb(pBomb);*/

            DynamicObject* m_Bomb = new Bomb;
            m_Bomb->SetDirection(0.3, 1);
            m_Bomb->SetSpeed(2);
            m_Bomb->SetPos(x, y);
            m_Bomb->SetWidth(SMALL_CRATER_SIZE);
            DynamicObject* n_bomb = new BombDecorator(m_Bomb);
            m_vecDynamic.push_back(n_bomb);
            m_countBomb--;
            m_score -= Bomb::BombCost;

            /*auto pFactory = new RegularFactory;
            m_vecDynamic.push_back(new BombDecorator(pFactory->createBomb(x, y)));
            delete pFactory;

            m_countBomb--;
            m_score -= Bomb::BombCost;*/
        }
        else
        {
            Bomb* m_Bomb = new Bomb;
            m_Bomb->SetDirection(0.3, 1);
            m_Bomb->SetSpeed(2);
            m_Bomb->SetPos(x, y);
            m_Bomb->SetWidth(SMALL_CRATER_SIZE);

            m_vecDynamic.push_back(m_Bomb);
            m_countBomb--;
            m_score -= Bomb::BombCost;
        }
    }
}

void SBomber::CommandExecuter(Command* pCommand)
{
    pCommand->Execute();
}

SBomber::SBomber()
    : exitFlag(false),
    startTime(0),
    finishTime(0),
    deltaTime(0),
    passedTime(0),
    fps(0),
    bombsNumber(10),
    score(0)
{
    WriteToLog(string(__FUNCTION__) + " was invoked");

    Plane* p = new Plane;
    p->SetDirection(1, 0.1);
    p->SetSpeed(4);
    p->SetPos(5, 10);
    vecDynamicObj.push_back(p);

    LevelGUI* pGUI = new LevelGUI;
    pGUI->SetParam(passedTime, fps, bombsNumber, score);
    const uint16_t maxX = GetMaxX();
    const uint16_t maxY = GetMaxY(); 
    const uint16_t offset = 3;
    const uint16_t width = maxX - 7;
    pGUI->SetPos(offset, offset);
    pGUI->SetWidth(width);
    pGUI->SetHeight(maxY - 4);
    pGUI->SetFinishX(offset + width - 4);
    vecStaticObj.push_back(pGUI);

    Ground* pGr = new Ground;
    const uint16_t groundY = maxY - 5;
    pGr->SetPos(offset + 1, groundY);
    pGr->SetWidth(width - 2);
    vecStaticObj.push_back(pGr);

    Tank* pTank = new Tank;
    pTank->SetWidth(13);
    pTank->SetPos(30, groundY - 1);
    vecStaticObj.push_back(pTank);

    pTank = new Tank;
    pTank->SetWidth(13);
    pTank->SetPos(50, groundY - 1);
    vecStaticObj.push_back(pTank);

    House * pHouse = new House;
    pHouse->SetWidth(13);
    pHouse->SetPos(80, groundY - 1);
    vecStaticObj.push_back(pHouse);

    /*
    Bomb* pBomb = new Bomb;
    pBomb->SetDirection(0.3, 1);
    pBomb->SetSpeed(2);
    pBomb->SetPos(51, 5);
    pBomb->SetSize(SMALL_CRATER_SIZE);
    vecDynamicObj.push_back(pBomb);
    */
}

SBomber::~SBomber()
{
    for (size_t i = 0; i < vecDynamicObj.size(); i++)
    {
        if (vecDynamicObj[i] != nullptr)
        {
            delete vecDynamicObj[i];
        }
    }

    for (size_t i = 0; i < vecStaticObj.size(); i++)
    {
        if (vecStaticObj[i] != nullptr)
        {
            delete vecStaticObj[i];
        }
    }
}

void SBomber::MoveObjects()
{
    WriteToLog(string(__FUNCTION__) + " was invoked");

    for (size_t i = 0; i < vecDynamicObj.size(); i++)
    {
        if (vecDynamicObj[i] != nullptr)
        {
            vecDynamicObj[i]->Move(deltaTime);
        }
    }
};

void SBomber::CheckObjects()
{
    WriteToLog(string(__FUNCTION__) + " was invoked");

    CheckPlaneAndLevelGUI();
    CheckBombsAndGround();
};

void SBomber::CheckPlaneAndLevelGUI()
{
    if (FindPlane()->GetX() > FindLevelGUI()->GetFinishX())
    {
        exitFlag = true;
    }
}

void SBomber::CheckBombsAndGround() 
{
    vector<DynamicObject*> vecBombs = FindAllBombs();
    Ground* pGround = FindGround();
    const double y = pGround->GetY();
    for (size_t i = 0; i < vecBombs.size(); i++)
    {
        if (vecBombs[i]->GetY() >= y) // ����������� ����� � ������
        {
            pGround->AddCrater(vecBombs[i]->GetX());
            CheckDestoyableObjects(vecBombs[i]);

            CommandDel<DynamicObject>delCom(SBomber::vecDynamicObj);
            delCom.setObj(vecBombs[i]);
            delCom.Execute();
        }
    }

}

void SBomber::CheckDestoyableObjects(DynamicObject* pBomb)
{
    vector<DestroyableGroundObject*> vecDestoyableObjects = FindDestoyableGroundObjects();
    const double size = pBomb->GetWidth();
    const double size_2 = size / 2;
    for (size_t i = 0; i < vecDestoyableObjects.size(); i++)
    {
        const double x1 = pBomb->GetX() - size_2;
        const double x2 = x1 + size;
        if (vecDestoyableObjects[i]->isInside(x1, x2))
        {
            score += vecDestoyableObjects[i]->GetScore();
            CommandDel<GameObject>comDel(SBomber::vecStaticObj);
            comDel.setObj(vecDestoyableObjects[i]);
            CommandExecuter(&comDel);
        }
    }
}

//void SBomber::DeleteDynamicObj(DynamicObject* pObj)
//{
//    auto it = vecDynamicObj.begin();
//    for (; it != vecDynamicObj.end(); it++)
//    {
//        if (*it == pObj)
//        {
//            vecDynamicObj.erase(it);
//            break;
//        }
//    }
//}

//void SBomber::DeleteStaticObj(GameObject* pObj)
//{
//    auto it = vecStaticObj.begin();
//    for (; it != vecStaticObj.end(); it++)
//    {
//        if (*it == pObj)
//        {
//            vecStaticObj.erase(it);
//            break;
//        }
//    }
//}

vector<DestroyableGroundObject*> SBomber::FindDestoyableGroundObjects() const
{
    vector<DestroyableGroundObject*> vec;
    Tank* pTank;
    House* pHouse;
    for (size_t i = 0; i < vecStaticObj.size(); i++)
    {
        pTank = dynamic_cast<Tank*>(vecStaticObj[i]);
        if (pTank != nullptr)
        {
            vec.push_back(pTank);
            continue;
        }

        pHouse = dynamic_cast<House*>(vecStaticObj[i]);
        if (pHouse != nullptr)
        {
            vec.push_back(pHouse);
            continue;
        }
    }

    return vec;
}

Ground* SBomber::FindGround() const
{
    Ground* pGround;

    for (size_t i = 0; i < vecStaticObj.size(); i++)
    {
        pGround = dynamic_cast<Ground *>(vecStaticObj[i]);
        if (pGround != nullptr)
        {
            return pGround;
        }
    }

    return nullptr;
}

vector<DynamicObject*> SBomber::FindAllBombs() const
{
    vector<DynamicObject*> vecBombs;

    for (size_t i = 0; i < vecDynamicObj.size(); i++)
    {
        Bomb* pBomb = dynamic_cast<Bomb*>(vecDynamicObj[i]);
        if (pBomb != nullptr)
        {
            vecBombs.push_back(pBomb);
        }
    }

    for (size_t i = 0; i < vecDynamicObj.size(); i++)
    {
        BombDecorator* pBomb = dynamic_cast<BombDecorator*>(vecDynamicObj[i]);
        if (pBomb != nullptr)
        {
            vecBombs.push_back(pBomb->m_bomb);
        }
    }

    return vecBombs;
}

Plane* SBomber::FindPlane() const
{
    for (size_t i = 0; i < vecDynamicObj.size(); i++)
    {
        Plane* p = dynamic_cast<Plane*>(vecDynamicObj[i]);
        if (p != nullptr)
        {
            return p;
        }
    }

    return nullptr;
}

LevelGUI* SBomber::FindLevelGUI() const
{
    for (size_t i = 0; i < vecStaticObj.size(); i++)
    {
        LevelGUI* p = dynamic_cast<LevelGUI*>(vecStaticObj[i]);
        if (p != nullptr)
        {
            return p;
        }
    }

    return nullptr;
}

void SBomber::ProcessKBHit()
{
    int c = _getch();

    if (c == 224)
    {
        c = _getch();
    }

    WriteToLog(string(__FUNCTION__) + " was invoked. key = ", c);

    switch (c) {

    case 27: // esc
        exitFlag = true;
        break;

    case 72: // up
        FindPlane()->ChangePlaneY(-0.25);
        break;

    case 80: // down
        FindPlane()->ChangePlaneY(0.25);
        break;

    case 'b':
    {
        CommandDropBomb dropCom(SBomber::vecDynamicObj);
        dropCom.setParams(FindPlane(), &bombsNumber, &score);
        CommandExecuter(&dropCom);
        break;
    }

    case 'B':
    {
        CommandDropBomb dropCom(SBomber::vecDynamicObj);
        dropCom.setParams(FindPlane(), &bombsNumber, &score);
        CommandExecuter(&dropCom);
        break;
    }

    default:
        break;
    }
}

void SBomber::DrawFrame()
{
    WriteToLog(string(__FUNCTION__) + " was invoked");

    for (size_t i = 0; i < vecDynamicObj.size(); i++)
    {
        if (vecDynamicObj[i] != nullptr)
        {
            vecDynamicObj[i]->Draw();
        }
    }

    for (size_t i = 0; i < vecStaticObj.size(); i++)
    {
        if (vecStaticObj[i] != nullptr)
        {
            vecStaticObj[i]->Draw();
        }
    }

    GotoXY(0, 0);
    fps++;

    FindLevelGUI()->SetParam(passedTime, fps, bombsNumber, score);
}

void SBomber::TimeStart()
{
    WriteToLog(string(__FUNCTION__) + " was invoked");
    startTime = GetTickCount64();
}

void SBomber::TimeFinish()
{
    finishTime = GetTickCount64();
    deltaTime = uint16_t(finishTime - startTime);
    passedTime += deltaTime;

    WriteToLog(string(__FUNCTION__) + " deltaTime = ", (int)deltaTime);
}

//void SBomber::DropBomb()
//{
//    if (bombsNumber > 0)
//    {
//        WriteToLog(string(__FUNCTION__) + " was invoked");
//
//        Plane* pPlane = FindPlane();
//        double x = pPlane->GetX() + 4;
//        double y = pPlane->GetY() + 2;
//
//        Bomb* pBomb = new Bomb;
//        pBomb->SetDirection(0.3, 1);
//        pBomb->SetSpeed(2);
//        pBomb->SetPos(x, y);
//        pBomb->SetWidth(SMALL_CRATER_SIZE);
//
//        vecDynamicObj.push_back(pBomb);
//        bombsNumber--;
//        score -= Bomb::BombCost;
//    }
//}
