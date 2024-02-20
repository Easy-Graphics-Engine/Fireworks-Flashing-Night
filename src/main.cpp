#include <graphics.h>

#include <math.h>
#include <stdlib.h>
#include <assert.h>

/* -------------------------------------------------------------------------- */
// 速度
typedef struct Speed
{
    double x, y;
} Speed;

// 位置
typedef struct Position
{
    double x, y;
} Position;

// 粒子
typedef struct Particle
{
    Position position;      // 粒子位置
    Speed    speed;         // 粒子速度
} Particle;

// 烟花燃放过程的几个阶段
typedef enum FireworksStage
{
    FireworksStage_WAIT  = 0,   // 等待阶段
    FireworksStage_RISE  = 1,   // 上升阶段
    FireworksStage_BLOOM = 2,   // 爆照阶段
} FireworksStage;
#define FIREWORKS_STAGE_COUNT   3

// 烟花
typedef struct Fireworks
{
    Particle* particle;     // 指向粒子数组
    int       particleNum;  // 粒子数量 (上限为 PARTICLE_MAX_NUM)

    Speed     speed;        // 烟花速度
    Position  position;     // 烟花位置
    color_t   color;        // 烟花颜色

    int       endTimes[FIREWORKS_STAGE_COUNT];  // 各阶段的结束时间
    int       time;         // 当前时间
    FireworksStage stage;   // 当前阶段
    bool      finished;     // 燃放结束标志位
} Fireworks;

/* -------------------------------------------------------------------------- */
// 创建一个 Fireworks 对象并进行初始化
Fireworks* createFireworks();

// 销毁 Fireworks 对象
void destroyFireworks(Fireworks* fireworks);

// 更新 Fireworks，返回值：{false: 结束; true: }结束时返回 false, 否则返回 true
void updateFireworks(Fireworks* fireworks);

// 初始化烟花
void initFireworks(Fireworks* fireworks);

// 判断烟花是否已燃放结束
bool isFireworksFinished(const Fireworks* fireworks);

// 设置烟花位置
void setFireworksPosition(Fireworks* fireworks, double x, double y);

// 设置烟花速度
void setFireworksSpeed(Fireworks* fireworks, double x, double y);

// 设置烟花颜色
void setFireworksColor(Fireworks* fireworks, color_t color);

// 绘制烟花
void drawFireworks(const Fireworks* fireworks);

/* -------------------------------------------------------------------------- */
// 更新烟花速度
static void updateFireworksSpeed(Fireworks* fireworks, double ax, double ay);

// 更新烟花位置
static void updateFireworksPosition(Fireworks* fireworks);

// 设置烟花各个阶段的时间值
static void setFireworksStageTime(Fireworks* fireworks, int waitTime, int riseTime, int bloomTime);

// 切换到下一阶段(已处于最后阶段时不再切换)
static void switchToNextStage(Fireworks* fireworks);

// 初始化粒子(位置、速度)
static void initParticles(Fireworks* fireworks, Position initialPosition);

// 更新粒子(位置、速度)
static void updateParticles(Fireworks* fireworks);

// 阶段检查，根据时间计数切换阶段，结束时设置标志位（可以 isFireworksFinished() 检查)
static void checkFireworksStage(Fireworks* fireworks);

// 重置烟花状态 (可重新燃放)
static void resetFireworksState(Fireworks* fireworks);

/* -------------------------------------------------------------------------- */
static const double accelerationY = 0.007;  // y 方向加速度

/* -------------------------------------------------------------------------- */
// 给烟花一个随机的速度、颜色和粒子数量
void randomFireworks(Fireworks* fireworks);

// 从图片文件中加载图像
PIMAGE loadImageFromFile(const char* fileName);

#define NUM_FIREWORKS     12    // 烟花数量
#define GROUND            580   // 地面位置y坐标
#define PARTICLE_MIN_NUM  160   // 每个烟花包含的最小粒子数目
#define PARTICLE_MAX_NUM  240   // 每个烟花包含的最大粒子数目

int main()
{
    initgraph(768, 768, INIT_RENDERMANUAL | INIT_NOFORCEEXIT);

    // 随机数初始化
    randomize();

    Fireworks* fireworksArray[NUM_FIREWORKS];

    // 创建烟花并进行初始化
    for (int i = 0; i < NUM_FIREWORKS; i++) {
        Fireworks* fireworks = createFireworks();
        randomFireworks(fireworks);
        setFireworksPosition(fireworks, 300 + random(450), GROUND);
        fireworksArray[i] = fireworks;
    }

    const char* backgroundImagePath = "resources/background.jpg";
    const char* backgroundMusicPath = "resources/background_music.mp3";

    // 背景图片
    PIMAGE backgroundImage = loadImageFromFile(backgroundImagePath);

    // 绘制背景图片后刷新，减少空白期
    putimage(0, 0, backgroundImage);
    delay_ms(0);

    //背景音乐
    int musicBeginTimeMs = 1000;
    MUSIC bgMusic;
    bgMusic.OpenFile(backgroundMusicPath);
    bgMusic.SetVolume(1.0f);
    if (bgMusic.IsOpen()) {
        bgMusic.Play(musicBeginTimeMs);
    }

    //图像缓存, 因为要加背景图，直接加模糊滤镜会把背景图模糊掉
    //所以另设一个图像缓存来绘制烟花并加模糊滤镜，再绘制到窗口
    PIMAGE cachePimg = newimage(800, 800);

    //计时用，主要用来定时检查音乐播放
    int timeCount = 0;

    for (; is_run(); delay_fps(60))
    {
        // 隔 1 秒检查一次，如果播放完了，重新播放
        if ((++timeCount % 60 == 0) && (bgMusic.GetPlayStatus() == MUSIC_MODE_STOP)) {
            timeCount = 0;
            bgMusic.Play(musicBeginTimeMs);
        }

        //更新位置
        for (int i = 0; i < NUM_FIREWORKS; i++) {
            Fireworks* fireworks = fireworksArray[i];
            updateFireworks(fireworks);
            if(isFireworksFinished(fireworks)) {
                randomFireworks(fireworks);
                setFireworksPosition(fireworks, 300 + random(450), GROUND);
                resetFireworksState(fireworks);
            }
        }

        // 绘制烟花到图像缓存中，并进行模糊处理
        settarget(cachePimg);

        for (int i = 0; i < NUM_FIREWORKS; i++) {
            drawFireworks(fireworksArray[i]);
        }

        imagefilter_blurring(cachePimg, 0x0a, 0xff);

        // 将背景图片和图像缓存绘制到窗口
        settarget(NULL);                        // 绘图目标切换回窗口
        cleardevice();                          // 清屏
        putimage(0, 0, backgroundImage);        // 绘制背景
        putimage(0, 0, cachePimg, SRCPAINT);    // 缓存绘制到窗口，模式为（最终颜色 = 窗口像素颜色 Or 图像像素颜色)
    }

    // 释放动态分配的内存
    for (int i = 0; i < NUM_FIREWORKS; i++) {
        destroyFireworks(fireworksArray[i]);
        fireworksArray[i] = NULL;
    }

    // 销毁图像，释放内存
    delimage(backgroundImage);
    delimage(cachePimg);

    // 关闭音乐
    bgMusic.Close();

    closegraph();

    return 0;
}

/* -------------------------------------------------------------------------- */
PIMAGE loadImageFromFile(const char* fileName)
{
    PIMAGE image = newimage();
    getimage(image, fileName);
    return image;
}

void randomFireworks(Fireworks* fireworks)
{
    int waitTime  = 120 + random(500);
    int riseTime  = 160 + random(40);
    int bloomTime = 150 + random(20);

    setFireworksStageTime(fireworks, waitTime, riseTime, bloomTime);
    setFireworksColor(fireworks, HSVtoRGB((float)randomf() * 360.0, 1.0f, 1.0f));

    double xSpeed = -0.25 + randomf() * 0.5; // 水平分速度 x, 范围: [-0.2, 0.2), 可稍微倾斜
    double ySpeed = -3.0 + randomf() * 0.8; // 垂直分速度 y, 范围: [-3.0, -2.2), 向上必须为负值
    setFireworksSpeed(fireworks, xSpeed, ySpeed);
}

Fireworks* createFireworks()
{
    Fireworks* fireworks = (Fireworks*)malloc(sizeof(Fireworks));
    assert(fireworks != NULL);
    fireworks->particle = (Particle*)malloc(sizeof(Particle) * PARTICLE_MAX_NUM);
    fireworks->particleNum = 0;

    initFireworks(fireworks);

    return fireworks;
}

void destroyFireworks(Fireworks* fireworks)
{
    free(fireworks->particle);
    free(fireworks);
}

void updateFireworks(Fireworks* fireworks)
{
    if (isFireworksFinished(fireworks))
        return;

    fireworks->time++;

    // 根据当前所处阶段进行相应的处理
    switch(fireworks->stage)
    {
    case FireworksStage_WAIT:   // 等待阶段
        // 不做任何处理
        break;
    case FireworksStage_RISE:   // 上升阶段
        // 更新烟花位置
        updateFireworksPosition(fireworks);

        // 根据设置的加速度更新速度
        updateFireworksSpeed(fireworks, 0.0, accelerationY);
        break;
    case FireworksStage_BLOOM:  // 爆炸阶段
        // 更新粒子位置
        updateParticles(fireworks);
        break;
    }

    // 根据燃放时间检查当前所处阶段
    checkFireworksStage(fireworks);
}

void checkFireworksStage(Fireworks* fireworks)
{
    if (isFireworksFinished(fireworks))
        return;

    // 当时间超过当前阶段所设置的时间时，跳转至下一阶段
    if (fireworks->time > fireworks->endTimes[fireworks->stage]) {
        FireworksStage currentStage = fireworks->stage;

        // 当前阶段结束时做的处理
        switch (currentStage)
        {
            case FireworksStage_WAIT:                                                   break;
            case FireworksStage_RISE:  initParticles(fireworks, fireworks->position);   break;
            case FireworksStage_BLOOM: fireworks->finished = true;                      break;
        }

        if (!isFireworksFinished(fireworks))
            switchToNextStage(fireworks);
    }
}

void initFireworks(Fireworks* fireworks)
{
    setFireworksStageTime(fireworks, 0, 0, 0);
    setFireworksPosition(fireworks, 0.0, 0.0);
    setFireworksSpeed(fireworks, 0.0, 0.0);
    setFireworksColor(fireworks, EGEARGB(255, 255, 255, 255));

    resetFireworksState(fireworks);
}

void updateFireworksSpeed(Fireworks* fireworks, double ax, double ay)
{
    fireworks->speed.x += ax;
    fireworks->speed.y += ay;
}

void setFireworksSpeed(Fireworks* fireworks, double x, double y)
{
    fireworks->speed.x = x;
    fireworks->speed.y = y;
}

void setFireworksPosition(Fireworks* fireworks, double x, double y)
{
    fireworks->position.x = x;
    fireworks->position.y = y;
}

void setFireworksColor(Fireworks* fireworks, color_t color)
{
    fireworks->color = color;
}

bool isFireworksFinished(const Fireworks* fireworks)
{
    return fireworks->finished;
}

void drawFireworks(const Fireworks* fireworks)
{
    if (isFireworksFinished(fireworks))
        return;

    switch (fireworks->stage)
    {
    case FireworksStage_WAIT:
        // 等待阶段，不绘制
        break;

    case FireworksStage_RISE:
        setfillcolor(fireworks->color);
        // 绘制 2x2 的点
        bar(fireworks->position.x, fireworks->position.y,
            fireworks->position.x + 2, fireworks->position.y + 2);
        break;

    case FireworksStage_BLOOM:
        setfillcolor(fireworks->color);
        for (int i = 0; i < fireworks->particleNum; i++) {
            bar(fireworks->particle[i].position.x, fireworks->particle[i].position.y,
                fireworks->particle[i].position.x + 2, fireworks->particle[i].position.y + 2);
        }
        break;
    }
}

/* -------------------------------------------------------------------------- */
static void initParticles(Fireworks* fireworks, Position initialPosition)
{
    Particle* const particle = fireworks->particle;
    fireworks->particleNum = PARTICLE_MIN_NUM + random(PARTICLE_MAX_NUM - PARTICLE_MIN_NUM + 1);

    for (int i = 0; i < fireworks->particleNum; i += 2)
    {
        // 为了球状散开，设初始速度大小相等
        // 初始随机速度水平角度和垂直角度，因为看到是平面的，所以求 x, y 分速度
        double levelAngle    = randomf() * 2 * PI;
        double verticalAngle = randomf() * 2 * PI;

        double speed = 2.5 + randomf() * 0.5;           // 随机发射速度

        double xySpeed = speed * cos(verticalAngle);    // 速度投影到 xOy 平面
        double xSpeed  = xySpeed * cos(levelAngle);     // 求水平分速度 x
        double ySpeed  = xySpeed * sin(levelAngle);     // 求垂直分速度 y

        // 动量守恒，(i) 和 (i+1) 这对粒子速度反向
        particle[i].position = initialPosition;
        particle[i].speed.x = xSpeed;
        particle[i].speed.y = ySpeed;

        if (i + 1 < fireworks->particleNum)
        {
            particle[i+1].position = initialPosition;
            particle[i + 1].speed.x = -particle[i].speed.x;
            particle[i + 1].speed.y = -particle[i].speed.y;
        }
    }
}

static void updateParticles(Fireworks* fireworks)
{
    for (int i = 0; i < fireworks->particleNum; i++) {
        fireworks->particle[i].position.x += fireworks->particle[i].speed.x;
        fireworks->particle[i].position.y += fireworks->particle[i].speed.y;

        //重力作用
        fireworks->particle[i].speed.y += accelerationY;

        //速度衰减
        fireworks->particle[i].speed.x *= 0.982;
        fireworks->particle[i].speed.y *= 0.982;
    }
}

static void updateFireworksPosition(Fireworks* fireworks)
{
    fireworks->position.x += fireworks->speed.x;
    fireworks->position.y += fireworks->speed.y;
}

static void setFireworksStageTime(Fireworks* fireworks, int waitTime, int riseTime, int bloomTime)
{
    fireworks->endTimes[FireworksStage_WAIT]  = waitTime;
    fireworks->endTimes[FireworksStage_RISE]  = riseTime;
    fireworks->endTimes[FireworksStage_BLOOM] = bloomTime;

    // 累加求各阶段结束时间
    for (int i = 1; i < FIREWORKS_STAGE_COUNT; i++)
        fireworks->endTimes[i] += fireworks->endTimes[i-1];

    fireworks->time = 0;
}

static void switchToNextStage(Fireworks* fireworks)
{
    int stageIndex = fireworks->stage;

    if ((stageIndex + 1) < FIREWORKS_STAGE_COUNT)
        fireworks->stage = (FireworksStage)(stageIndex + 1);
}

static void resetFireworksState(Fireworks* fireworks)
{
    fireworks->particleNum = 0;
    fireworks->time  = 0;
    fireworks->stage = FireworksStage_WAIT;
    fireworks->finished = false;
}


