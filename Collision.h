#pragma once

#include "Common/d3dApp.h"
using namespace DirectX;

class Collision {
public:

    enum EPos {tfl = 0, tfr = 1, tbl = 2, tbr = 3, bfl = 4, bfr = 5, bbl = 6, bbr = 7, size = 8};    //Postion

    //Holds data for a collision cubes position
    class ColCube
    {
    public:
        
        XMFLOAT3 list[8];

        ColCube() {
            for (int i = 0; i < (int)EPos::size; i++) {
                list[i] = XMFLOAT3(0, 0, 0);
            }

        }

        ColCube(XMFLOAT3 ntfl, XMFLOAT3 ntfr, XMFLOAT3 ntbl, XMFLOAT3 ntbr, XMFLOAT3 nbfl, XMFLOAT3 nbfr, XMFLOAT3 nbbl, XMFLOAT3 nbbr) {
            list[EPos::tfl] = ntfl;
            list[EPos::tfr] = ntfr;
            list[EPos::tbl] = ntbl;
            list[EPos::tbr] = ntbr;
            list[EPos::bfl] = nbfl;
            list[EPos::bfr] = nbfr;
            list[EPos::bbl] = nbbl;
            list[EPos::bbr] = nbbr;
        }

        void Translate(XMFLOAT3 move);
    };

    //Holds data on a collision point data (point and size)
    struct ColPoint {
        XMFLOAT3 ftl; //Front Top Left		- 7th in vertex array
        XMFLOAT3 size; //Width, Height, Depth

        ColPoint() {
            ftl = XMFLOAT3(0, 0, 0);
            size = XMFLOAT3(0, 0, 0);
        }

        ColPoint(const XMFLOAT3 nFTL, const XMFLOAT3 nSize) {
            ftl = nFTL;
            size = nSize;
        }
    };

    //Holds data about which points in a collision cube have been hit
    class ColPoints {
    public:
        bool list[8];

        ColPoints() {
            for (int i = 0; i < (int)EPos::size; i++) {
                list[i] = false;
            }
        }

        ColPoints(bool ntfl, bool ntfr, bool ntbl, bool ntbr, bool nbfl, bool nbfr, bool nbbl, bool nbbr) {
            list[EPos::tfl] = ntfl;
            list[EPos::tfr] = ntfr;
            list[EPos::tbl] = ntbl;
            list[EPos::tbr] = ntbr;
            list[EPos::bfl] = nbfl;
            list[EPos::bfr] = nbfr;
            list[EPos::bbl] = nbbl;
            list[EPos::bbr] = nbbr;
        }

        ColPoints operator +(ColPoints& b) {
            ColPoints ret(list[EPos::tfl] || b.list[EPos::tfl],
                list[EPos::tfr] || b.list[EPos::tfr],
                list[EPos::tbl] || b.list[EPos::tbl],
                list[EPos::tbr] || b.list[EPos::tbr],
                list[EPos::bfl] || b.list[EPos::bfl],
                list[EPos::bfr] || b.list[EPos::bfr],
                list[EPos::bbl] || b.list[EPos::bbl],
                list[EPos::bbr] || b.list[EPos::bbr]);
            return ret;
        }

        ColPoints& operator +=(ColPoints& b) {
            list[EPos::tfl] |= b.list[EPos::tfl];
            list[EPos::tfr] |= b.list[EPos::tfr];
            list[EPos::tbl] |= b.list[EPos::tbl];
            list[EPos::tbr] |= b.list[EPos::tbr];
            list[EPos::bfl] |= b.list[EPos::bfl];
            list[EPos::bfr] |= b.list[EPos::bfr];
            list[EPos::bbl] |= b.list[EPos::bbl];
            list[EPos::bbr] |= b.list[EPos::bbr];
            return *this;
        }

        void Reset();

        bool AnyBottom();
        bool AnyTop();
        bool AnyLeft();
        bool AnyRight();
        bool AnyFront();
        bool AnyBack();


    };

    static bool Within(const ColCube cube, const XMFLOAT3 point);
    static bool CheckCollisions(const ColCube Entity, const ColCube Block);
    static ColPoints CheckCollisionPoints(const ColCube Entity, const ColCube Block);

};