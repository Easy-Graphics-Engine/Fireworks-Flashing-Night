#pragma once

#ifndef FIREWORKS_H_
#define FIREWORKS_H_

#include <graphics.h>

//随机数, 0.0~1.0
#define myrand(m) ((float)rand() * m / 36565)

struct Speed {
    double x, y;
};

struct Pos {
    double x, y;
};

struct Particle
{
    Pos pos;
    Speed speed;
};


#define GROUND 580  //地面位置y坐标

class Fireworks//烟花类
{
private:

    static const int NUM_PARTICLE = 200;
    static const double particleSpeed;
    Particle p[NUM_PARTICLE];

    color_t color;

    int delayTime;      //延迟时间
    int riseTime;       //上升时间
    int bloomTime;      //爆炸时间

    Pos risePos;        //上升阶段位置
    Speed riseSpeed;    //上升速度

public:
    //初始化
    Fireworks();

    void init();

    //更新位置等相关属性
    void update();

    //根据属性值绘画
    void draw(PIMAGE pimg = NULL);
};

#endif // ! FIREWORKS_H_

