#pragma once

#include "Common/d3dApp.h"
using namespace DirectX;

class Collision {
public:

    enum class Directions { FRONT, BACK, TOP, BOTTOM, LEFT, RIGHt};

    //Holds data for a collision cubes position
    struct ColCube
    {

        XMFLOAT3 tfl;//top front left
        XMFLOAT3 tfr;//top front right
        XMFLOAT3 tbl;//top back left
        XMFLOAT3 tbr;//top back right

        XMFLOAT3 bfl;//bottom front left
        XMFLOAT3 bfr;//bottom front right
        XMFLOAT3 bbl;//bottom back left
        XMFLOAT3 bbr;//bottom back right

        ColCube() {
            tfl = XMFLOAT3(0, 0, 0);
            tfr = XMFLOAT3(0, 0, 0);
            tbl = XMFLOAT3(0, 0, 0);
            tbr = XMFLOAT3(0, 0, 0);
            bfl = XMFLOAT3(0, 0, 0);
            bfr = XMFLOAT3(0, 0, 0);
            bbl = XMFLOAT3(0, 0, 0);
            bbr = XMFLOAT3(0, 0, 0);
        }

        ColCube(XMFLOAT3 ntfl, XMFLOAT3 ntfr, XMFLOAT3 ntbl, XMFLOAT3 ntbr, XMFLOAT3 nbfl, XMFLOAT3 nbfr, XMFLOAT3 nbbl, XMFLOAT3 nbbr) {
            tfl = ntfl;
            tfr = ntfr;
            tbl = ntbl;
            tbr = ntbr;
            bfl = nbfl;
            bfr = nbfr;
            bbl = nbbl;
            bbr = nbbr;
        }
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
        bool tfl;//top front left
        bool tfr;//top front right
        bool tbl;//top back left
        bool tbr;//top back right

        bool bfl;//bottom front left
        bool bfr;//bottom front right
        bool bbl;//bottom back left
        bool bbr;//bottom back right

        ColPoints() {
            tfl = false;
            tfr = false;
            tbl = false;
            tbr = false;
            bfl = false;
            bfr = false;
            bbl = false;
            bbr = false;
        }

        ColPoints(bool ntfl, bool ntfr, bool ntbl, bool ntbr, bool nbfl, bool nbfr, bool nbbl, bool nbbr) {
            tfl = ntfl;
            tfr = ntfr;
            tbl = ntbl;
            tbr = ntbr;
            bfl = nbfl;
            bfr = nbfr;
            bbl = nbbl;
            bbr = nbbr;
        }

        ColPoints operator +(ColPoints& b) {
            ColPoints ret(tfl || b.tfl,
            tfr || b.tfr,
            tbl || b.tbl,
            tbr || b.tbr,
            bfl || b.bfl,
            bfr || b.bfr,
            bbl || b.bbl,
            bbr || b.bbr);
            return ret;
        }

        ColPoints& operator +=(ColPoints& b) {
            tfl |= b.tfl;
            tfr |= b.tfr;
            tbl |= b.tbl;
            tbr |= b.tbr;
            bfl |= b.bfl;
            bfr |= b.bfr;
            bbl |= b.bbl;
            bbr |= b.bbr;
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