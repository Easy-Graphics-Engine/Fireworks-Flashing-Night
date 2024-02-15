#include <cmath>
#define SHOW_CONSOLE
#include "fireworks.h"

const double Fireworks::particleSpeed = 3.0;

Fireworks::Fireworks()
{
    init();
}

void Fireworks::init()
{
    delayTime = rand() % 300 + 20;
    riseTime = rand() % 80 + 160;
    bloomTime = 160;

    risePos.x = rand() % 450 + 300.0f;
    risePos.y = GROUND;

    riseSpeed.y = myrand(1.0f) - 3.0f;  //上升速度，根据坐标系需要是负的
    riseSpeed.x = myrand(0.4f) - 0.2f;  //可稍微倾斜

    //随机颜色
    color = HSVtoRGB(myrand(360.0f), 1.0f, 1.0f);

    //给每一个粒子设置初始速度
    for (int i = 0; i < NUM_PARTICLE - 1; i += 2)
    {
        //为了球状散开，设初始速度大小相等
        //初始随机速度水平角度和垂直角度，因为看到是平面的，所以求x, y分速度
        double levelAngle = randomf() * 360;
        double verticalAngle = randomf() * 360;

        //速度投影到xOy平面
        double xySpeed = particleSpeed * cos(verticalAngle);

        //求x, y分速度
        p[i].speed.x = xySpeed * cos(levelAngle);
        p[i].speed.y = xySpeed * sin(levelAngle);

        //动量守恒，每对速度反向
        if (i + 1 < NUM_PARTICLE) {
            p[i + 1].speed.x = -p[i].speed.x;
            p[i + 1].speed.y = -p[i].speed.y;
        }
    }
}

void Fireworks::draw(PIMAGE pimg)
{
    //未开始
    if (delayTime > 0)
        return;
    //烟花上升阶段
    else if (riseTime > 0) {
        setfillcolor(color, pimg);
        //画四个点，这样大一些
        bar(risePos.x, risePos.y, risePos.x + 2, risePos.y + 2, pimg);
    }
    //烟花绽放阶段
    else {
        setfillcolor(color, pimg);
        for (int i = 0; i < NUM_PARTICLE; i++) {
            bar(p[i].pos.x, p[i].pos.y, p[i].pos.x + 2, p[i].pos.y + 2, pimg);
        }
    }
}

//更新位置等相关属性
void Fireworks::update()
{
    if (delayTime-- > 0)
        return;
    //处于上升阶段，只更新烟花位置
    else if (riseTime > 0) {
        risePos.x += riseSpeed.x;
        risePos.y += riseSpeed.y;

        //重力作用
        riseSpeed.y += 0.005;

        //上升完毕，到达爆炸阶段
        if (--riseTime <= 0) {
            //设粒子初始位置为烟花当前位置
            for (int i = 0; i < NUM_PARTICLE; i++) {
                p[i].pos.x = risePos.x;
                p[i].pos.y = risePos.y;
            }
        }
    }
    //烟花绽放阶段
    else if (bloomTime-- > 0) {
        //粒子散开，更新粒子位置
        for (int i = 0; i < NUM_PARTICLE; i++) {
            p[i].pos.x += p[i].speed.x;
            p[i].pos.y += p[i].speed.y;

            //重力作用
            p[i].speed.y += 0.005;

            //速度减慢
            p[i].speed.x *= 0.982;
            p[i].speed.y *= 0.982;
        }
    }
    else {
        //烟花重新开始
        init();
    }
}
